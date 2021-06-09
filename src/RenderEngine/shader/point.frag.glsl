#version 440

in vec3 vPosition;
in vec3 vColor;

uniform float uPointSize;

layout (std140, binding=0) uniform TransformUniformBlock
{
	mat4 model;
	mat4 view;
	mat4 proj;

	int width;
	int height;
} transform;

layout(std140, binding = 2) uniform MaterialUniformBlock
{
	vec4  albedo;
	float metallic;
	float roughness;

	int   colorMode;
	float colorMin;
	float colorMax;

	int   shadowMode;
};

layout(location = 0) out vec4 fragColor;

subroutine void RenderPass(void);
layout(location = 0) subroutine uniform RenderPass renderPass;

vec3 normal;
vec3 position;

void main(void) 
{
    // make sphere...
    vec2 uv = gl_PointCoord * 2.0 - vec2(1.0);
    float d = dot(uv, uv);
    if (d > 1.0)
    {
        discard;
    }
	normal = vec3(uv.x, -uv.y, sqrt(1.f-d));
	position = vPosition + normal * uPointSize;

	// update depth
	vec4 clipPos = transform.proj * vec4(position, 1);
	float ndcZ = clipPos.z / clipPos.w;

	gl_FragDepth = (gl_DepthRange.diff * ndcZ + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
	renderPass();
}

vec3 pbr();
layout(index = 0) subroutine(RenderPass) void ColorPass(void)
{
	fragColor.rgb = pbr();
	fragColor.a = 1.0;
}

layout(index = 1) subroutine(RenderPass) void DepthPass(void)
{
}


layout(std140, binding = 1) uniform LightUniformBlock
{
	vec4 ambient;
// main directional light
vec4 intensity;
vec4 direction;
mat4 transform;
} light;

/***************** ShadowMap *********************/
layout(binding = 5) uniform sampler2D shadowDepth;
//layout(binding = 6) uniform sampler2D shadowColor;

vec3 GetShadowFactor(vec3 pos)
{
	vec4 posLightSpace = light.transform * vec4(pos, 1);
	vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowDepth, projCoords.xy).r;
	float currentDepth = min(1.0, projCoords.z);

	//float bias = max(0.05 * (1.0 - dot(normal, normalize(light.direction.xyz))), 0.005); 
	float bias = 0.005;

	// simple PCF
	vec3 shadow = vec3(0);
	vec2 texelSize = 1.0 / textureSize(shadowDepth, 0);
	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowDepth, projCoords.xy + vec2(x, y) * texelSize).r;
			float visible = currentDepth - bias > pcfDepth ? 0.0 : 1.0;
			//shadow += texture(shadowColor, projCoords.xy + vec2(x, y) * texelSize).rgb * visible;
			// for transparent object, we only consider shadow from opacity objects...
			shadow += vec3(visible);
		}
	}
	return clamp(shadow / 9.0, 0, 1);
}

// refer to https://learnopengl.com
const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / max(denom, 0.001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 pbr()
{
	vec3 N = normalize(normal);
	vec3 V = normalize(-position);

	float dotNV = dot(N, V);
	if (dotNV < 0.0)	N = -N;

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo.rgb, metallic);

	// reflectance equation
	vec3 Lo = vec3(0.0);
	//for(int i = 0; i < 4; ++i) 
	{
		// calculate per-light radiance
		//vec3 L = normalize(lightPositions[i] - WorldPos);
		vec3 L = normalize(light.direction.xyz);
		vec3 H = normalize(V + L);
		//float distance = length(lightPositions[i] - WorldPos);
		//float attenuation = 1.0 / (distance * distance);
		//vec3 radiance = lightColors[i] * attenuation;
		vec3 radiance = light.intensity.rgb * light.intensity.a;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

		vec3 nominator = NDF * G * F;
		float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		vec3 specular = nominator / max(denominator, 0.001); // prevent divide by zero for NdotV=0.0 or NdotL=0.0

		// kS is equal to Fresnel
		vec3 kS = F;
		// for energy conservation, the diffuse and specular light can't
		// be above 1.0 (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.0 - kS.
		vec3 kD = vec3(1.0) - kS;
		// multiply kD by the inverse metalness such that only non-metals 
		// have diffuse lighting, or a linear blend if partly metal (pure metals
		// have no diffuse light).
		kD *= 1.0 - metallic;

		// scale light by NdotL
		float NdotL = max(dot(N, L), 0.0);

		// add to outgoing radiance Lo
		//Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

		Lo += GetShadowFactor(position) * (kD * albedo.rgb / PI + specular) * radiance * NdotL;
	}

	vec3 ambient = light.ambient.rgb * light.ambient.a * albedo.rgb;

	return ambient + Lo;
}
