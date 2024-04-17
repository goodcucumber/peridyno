import os
import sys

import PyPeridyno as dyno

print(sys.path)

scn = dyno.SceneGraph()

scn.set_upper_bound(dyno.Vector3f([1.5, 1, 1.5]))
scn.set_lower_bound(dyno.Vector3f([-0.5, 0, -0.5]))

cube = dyno.CubeModel3f()
cube.var_location().set_value(dyno.Vector3f([0.6, 0.5, 0.5]))
cube.var_length().set_value(dyno.Vector3f([0.2, 0.2, 0.2]))
cube.graphics_pipeline().disable()

sampler = dyno.CubeSampler3f()
sampler.var_sampling_distance().set_value(0.005)
sampler.graphics_pipeline().disable()

cube.out_cube().connect(sampler.in_cube())

initialParticles = dyno.MakeParticleSystem3f()

sampler.state_point_set().promote_ouput().connect(initialParticles.in_points())

fluid = dyno.ParticleFluid3f()
fluid.var_reshuffle_particles().set_value(True)
initialParticles.connect(fluid.import_initial_states())

boundary = dyno.StaticBoundary3f()
boundary.load_cube(dyno.Vector3f([-0.5, 0, -0.5]), dyno.Vector3f([1.5, 2, 1.5]), 0.02, True)

script_dir = os.path.dirname(os.path.abspath(__file__))
relative_path = "../../../data/bowl/bowl.sdf"
file_path = os.path.join(script_dir, relative_path)
if os.path.isfile(file_path):
    boundary.load_sdf(file_path, False)
else:
    print(f"File not found: {file_path}")

fluid.connect(boundary.import_particle_systems())

calculateNorm = dyno.CalculateNorm3f()
fluid.state_velocity().connect(calculateNorm.in_vec())
fluid.graphics_pipeline().push_module(calculateNorm)

colorMapper = dyno.ColorMapping3f()
colorMapper.var_max().set_value(5.0)
calculateNorm.out_norm().connect(colorMapper.in_scalar())
fluid.graphics_pipeline().push_module(colorMapper)

ptRender = dyno.GLPointVisualModule()
ptRender.set_color(dyno.Color(1, 0, 0))
ptRender.set_color_map_mode(ptRender.ColorMapMode.PER_VERTEX_SHADER)

fluid.state_point_set().connect(ptRender.in_point_set())
colorMapper.out_color().connect(ptRender.in_color())

fluid.graphics_pipeline().push_module(ptRender)

colorBar = dyno.ImColorbar3f()
colorBar.var_max().set_value(5.0)
colorBar.var_field_name().set_value("Velocity")
calculateNorm.out_norm().connect(colorBar.in_scalar())

fluid.graphics_pipeline().push_module(colorBar)

scn.add_node(cube)
scn.add_node(sampler)
scn.add_node(initialParticles)
scn.add_node(fluid)
scn.add_node(boundary)

app = dyno.GLfwApp()
app.set_scenegraph(scn)
app.initialize(800, 600, True)
app.main_loop()
