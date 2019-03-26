#include "ShaderPipelineReflectionBuilder.h"

#include "spirv_glsl.hpp"
#include <algorithm>

namespace shaderReflection
{

bool 
ShaderPipelineReflectionBuilder::BuildReflection(
	BuildInfo const& buildInfo, 
	ShaderPipelineReflection& reflection)
{
	reflection = {};

	if (buildInfo.stageCount == 0u || buildInfo.stages == nullptr)
	{
		return false;
	}

	auto&	sets{ reflection.sets };	

	for (auto stageIndex{ 0u }; stageIndex < buildInfo.stageCount; ++stageIndex)
	{
		ShaderStageCode const&	currentStage{ buildInfo.stages[stageIndex] };

		spirv_cross::CompilerGLSL const glsl{ currentStage.byteCode,
			currentStage.byteSize };

		spirv_cross::ShaderResources resources = glsl.get_shader_resources();

		if (currentStage.stage == vk::ShaderStageFlagBits::eVertex)
		{
			auto const	resourceStageInputs{ resources.stage_inputs };
			auto const	inputElementsCount{ resourceStageInputs.size() };
			auto& stageInput{ reflection.stageInputs };

			stageInput.inputs.resize(inputElementsCount);

			for (auto index{ 0u }; index < resourceStageInputs.size(); ++index)
			{
				auto const&	currentResourceInput{ resourceStageInputs[index] };
				auto&	currentInput{ stageInput.inputs[index] };

				currentInput.name = currentResourceInput.name;
				currentInput.location = glsl.get_decoration(
					currentResourceInput.id, spv::DecorationLocation);
			}
		}


		for (auto const& resource : resources.uniform_buffers)
		{
			//Retrieving the descriptor set with the resource set and the binding indices
			uint32_t const setIndex{ glsl.get_decoration(resource.id, 
				spv::DecorationDescriptorSet) };

			uint32_t const bindingIndex{ glsl.get_decoration(resource.id, 
				spv::DecorationBinding) };
			
			auto	foundSetIt{ 
				std::find_if(sets.begin(), sets.end(), 
				[searchedIndex = setIndex] (auto const& set) 
				{ return set.setIndex == searchedIndex; }) };
			
			if (foundSetIt == sets.end())
			{
				sets.resize(sets.size() + 1);
				foundSetIt = --sets.end();
			}

			foundSetIt->setIndex = setIndex;

			auto&	ubos{ foundSetIt->uniformBuffers };
			auto	foundUBIt{ std::find_if(ubos.begin(), ubos.end(), 
				[searchedIndex = bindingIndex] (auto const& ubo) 
				{ return ubo.info.bindingIndex == searchedIndex; }) };

			if (foundUBIt == ubos.end())
			{
				ubos.resize(ubos.size() + 1);
				foundUBIt = --ubos.end();
			}
			
			foundUBIt->info.name = resource.name;
			foundUBIt->info.bindingIndex = bindingIndex;
			foundUBIt->info.boundStages |= currentStage.stage;
		}

		for (auto const& resource : resources.sampled_images)
		{
			//Retrieving the descriptor set with the resource set and the binding indices
			uint32_t const setIndex{ glsl.get_decoration(resource.id, 
				spv::DecorationDescriptorSet) };
			
			uint32_t const bindingIndex{ glsl.get_decoration(resource.id, 
				spv::DecorationBinding) };

			auto	foundSetIt{ std::find_if(sets.begin(), sets.end(), 
				[searchedIndex = setIndex] (auto const& set) 
			{ return set.setIndex == searchedIndex; }) };

			if (foundSetIt == sets.end())
			{
				sets.resize(sets.size() + 1);
				foundSetIt = --sets.end();
			}

			foundSetIt->setIndex = setIndex;

			auto&	sampledImages{ foundSetIt->sampledImages };
			auto	foundSampledImageIt{ 
				std::find_if(sampledImages.begin(), sampledImages.end(), 
				[searchedIndex = bindingIndex] (auto const& sampledImage) 
				{ return sampledImage.info.bindingIndex == searchedIndex; }) };

			if (foundSampledImageIt == sampledImages.end())
			{
				sampledImages.resize(sampledImages.size() + 1);
				foundSampledImageIt = --sampledImages.end();
			}

			foundSampledImageIt->info.name = resource.name;
			foundSampledImageIt->info.bindingIndex = bindingIndex;
			foundSampledImageIt->info.boundStages |= currentStage.stage;
		}
	}

	return true;
}

}