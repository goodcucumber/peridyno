#include "VolumeGenerator.h"

#include "Module/FastSweepingMethod.h"
#include "Module/FastSweepingMethodGPU.h"

namespace dyno
{
	IMPLEMENT_TCLASS(VolumeGenerator, TDataType)

	template<typename TDataType>
	VolumeGenerator<TDataType>::VolumeGenerator()
		: Volume<TDataType>()
	{
		auto fsm = std::make_shared<FastSweepingMethodGPU<TDataType>>();
		this->inTriangleSet()->connect(fsm->inTriangleSet());
		fsm->outLevelSet()->connect(this->outLevelSet());

		this->resetPipeline()->pushModule(fsm);

		this->varSpacing()->attach(std::make_shared<FCallBackFunc>(
			[=]() {
				fsm->varSpacing()->setValue(this->varSpacing()->getValue());
			}));

		this->varPadding()->attach(std::make_shared<FCallBackFunc>(
			[=]() {
				fsm->varPadding()->setValue(this->varPadding()->getValue());
			}));
	}

	template<typename TDataType>
	VolumeGenerator<TDataType>::~VolumeGenerator()
	{
	}

	template<typename TDataType>
	void VolumeGenerator<TDataType>::resetStates()
	{
		Volume<TDataType>::resetStates();
	}

	DEFINE_CLASS(VolumeGenerator);
}