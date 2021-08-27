#include "ImColorbar.h"
// opengl
#include <glad/glad.h>
#include "RenderEngine.h"
#include "RenderTools.h"
#include "Utility.h"

#include <cuda_gl_interop.h>

// framework
#include <Topology/TriangleSet.h>
#include <Node.h>

namespace dyno
{
	IMPLEMENT_CLASS(ImColorbar)

	ImColorbar::ImColorbar()
	{
		this->setName("im_colorbar");
	}
	ImColorbar::ImColorbar(const char* title)
	{
		mTitle = title;
		this->setName("im_colorbar");
	}
	ImColorbar::~ImColorbar()
	{
		// mColorBuffer.clear();
	}

	void ImColorbar::setCoord(ImVec2 coord)
	{
		mCoord = coord;
	}

	ImVec2 ImColorbar::getCoord() const
	{
		return mCoord;
	}

	bool ImColorbar::initializeGL()
	{
		mCol = nullptr;
		mVal = nullptr;
		return true;
	}

	void ImColorbar::updateGL()
	{
		Node* parent = getParent();

		if (parent == NULL)
			return;

		if (!parent->isVisible())
			return;

		if (!this->inScalar()->isEmpty())
		{
			
			auto pScalar = this->inScalar()->getData();
			auto min_v = m_reduce_real.minimum(pScalar.begin(), pScalar.size());
			auto max_v = m_reduce_real.maximum(pScalar.begin(), pScalar.size());
			
			mNum = 6 + 1;
			
			//assert(std::is_same<float, Real>::value);

			float lowLimit = this->varMin()->getData();
			float upLimit = this->varMax()->getData();
			float dv = (max_v - min_v) / mNum;
			if(this->varType()->getData() == ColorTable::Jet)
			{
				col[0] = ImGui::ToJetColor(min_v, lowLimit, upLimit);
				val[0] = min_v;
				for(int i = 1; i < mNum; ++i)
				{
					val[i] = min_v + dv * i;
					col[i] = ImGui::ToJetColor(val[i], lowLimit, upLimit);
				}
			}
			else if(this->varType()->getData() == ColorTable::Heat)
			{
				col[0] = ImGui::ToHeatColor(min_v, lowLimit, upLimit);
				val[0] = min_v;
				for(int i = 1; i < mNum; ++i)
				{
					val[i] = min_v + dv * i;
					col[i] = ImGui::ToHeatColor(val[i], lowLimit, upLimit);
				}				
			}
		}
		else
		{
			mNum = 1;
			col[0] = ImGui::VecToImU(&mBaseColor);
			val[0] = 0.0;
		}
		mCol = col;
		mVal = val;
	}

	void ImColorbar::paintGL(RenderMode mode)
	{
		if(mode == RenderMode::COLOR)
		{
			auto label = "Right sidebar ImColorBar";
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ExColorsVal[ImGui::ImGuiExColVal_WindowTopBg_1]);
			ImGui::Begin(label, NULL, /*ImGuiWindowFlags_NoMove |*/  ImGuiWindowFlags_NoTitleBar | /*ImGuiWindowFlags_NoBackground |*/ ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::PopStyleColor();
			ImGui::Text(mTitle);
			ImGui::ColorBar<ImU32*>("ColorBar", mVal, mCol, mNum);
			ImGui::End();
		}
	}
}
