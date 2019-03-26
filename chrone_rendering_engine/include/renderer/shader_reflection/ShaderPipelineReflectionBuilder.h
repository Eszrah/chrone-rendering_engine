#pragma once

#include <vulkan/vulkan.hpp>

namespace spirv_cross
{
struct Resource;
}

namespace shaderReflection
{

struct SetElementInfo
{
	std::string	name{};
	uint32_t	bindingIndex{};
	vk::ShaderStageFlags	boundStages{};
};

struct UniformBufferReflection
{
	SetElementInfo	info{};
	//uint32_t	size{};
};

struct SampledImageReflection
{
	SetElementInfo	info{};
};

struct DescriptorSetReflection
{
	uint32_t	setIndex{};
	std::vector<UniformBufferReflection>	uniformBuffers{};
	std::vector<SampledImageReflection>	sampledImages{};

};

struct LocatedElement 
{
	std::string	name{};
	uint32_t	location{};
	uint32_t	size{};
};

struct StageLocations
{
	std::vector<LocatedElement>	inputs{};
};

struct ShaderPipelineReflection
{
	StageLocations	stageInputs{};
	std::vector<DescriptorSetReflection>	sets{};
};

struct ShaderStageCode
{
	vk::ShaderStageFlagBits	stage{};
	size_t byteSize{};
	uint32_t const*	byteCode{};
};

class ShaderPipelineReflectionBuilder
{
public:

	struct BuildInfo
	{
		uint32_t	stageCount{};
		ShaderStageCode const*	stages{};
	};

	bool	BuildReflection(BuildInfo const& buildInfo, ShaderPipelineReflection& reflection);

private:

};

}
