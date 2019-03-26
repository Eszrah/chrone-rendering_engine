#pragma once

#include <vulkan\vulkan.hpp>

#include "NativeType.h"
#include "renderer/ResourceHandle.h"

namespace chrone::rendering
{


struct CmdBufferCreateInfo
{
	vk::BufferCreateInfo	createInfo;
	vk::MemoryPropertyFlags	memoryProperties;
};


struct CmdImageCreateInfo
{
	enum struct CreationType
	{
		Image,
		View
	};

	struct ImageCreation
	{
		vk::ImageCreateInfo		createInfo;
		vk::MemoryPropertyFlags	memoryProperties;
	};

	struct ViewProperties
	{
		vk::ImageViewType			viewType{};
		vk::Format					format{};
		vk::ComponentMapping		components{};
		vk::ImageSubresourceRange	subresourceRange{};
	};

	struct ViewCreation
	{
		HImageView	srcImageView{};
	};

	CreationType	creationType = CreationType::Image;
	ViewProperties	viewProperties{};

	union
	{
		ImageCreation	imageCreation;
		ViewCreation	viewCreation;
	};
};


struct CmdRenderPassCreateInfo
{
	vk::RenderPassCreateInfo	createInfo{};
};


struct ShaderStageCreateInfo
{
	vk::ShaderStageFlagBits	stage{};
	Uint32					byteSize{};
	Uint32 const*			code{};
	std::string				entryPointName{};
};


struct CmdGraphicsPipelineCreateInfo
{
	Uint32					stageCount{};
	ShaderStageCreateInfo	stages[6]{};

	Uint32						descriptorSetLayoutCount{};
	HDescriptorSetLayout const*	pDescriptorSetLayouts{};

	vk::PipelineVertexInputStateCreateInfo const*      vertexInputState;
	vk::PipelineInputAssemblyStateCreateInfo const*    inputAssemblyState;
	vk::PipelineTessellationStateCreateInfo const*     tessellationState;
	vk::PipelineViewportStateCreateInfo const*         viewportState;
	vk::PipelineRasterizationStateCreateInfo const*    rasterizationState;
	vk::PipelineMultisampleStateCreateInfo const*      multisampleState;
	vk::PipelineDepthStencilStateCreateInfo const*     depthStencilState;
	vk::PipelineColorBlendStateCreateInfo const*       colorBlendState;
	vk::PipelineDynamicStateCreateInfo const*          dynamicState;
	HRenderPass	renderPass{};
	Uint32		subpass{};
};


struct CmdFrameBufferCreateInfo
{
	HFramebuffer		framebuffer{};
	HRenderPass			renderPass{};
	Uint32				attachmentCount{};
	const HImageView*	attachments{};
	Uint32				width{};
	Uint32				height{};
	Uint32				layers{};
};


struct CmdComputePipelineCreateInfo
{
	ShaderStageCreateInfo	stage{};
};


struct CmdFenceCreateInfo 
{
	vk::FenceCreateInfo	createInfo{};
};


struct CmdSemaphoreCreateInfo 
{
	vk::SemaphoreCreateInfo	createInfo{};
};


struct CmdDrawIndexed
{
	Uint32	indexCount{ 0u };
	Uint32	instanceCount{ 0u };
	Uint32	firstIndex{ 0u };
	Int32	vertexOffset{ 0u };
	Uint32	firstInstance{ 0u };
};


struct CmdDraw
{
	Uint32	vertexCount{ 0u };
	Uint32	instanceCount{ 0u };
	Uint32	firstIndex{ 0u };
	Uint32	firstInstance{ 0u };
};


struct CmdBeginRenderPass
{
	HFramebuffer	framebuffer{};
	HRenderPass	renderPass{};
	vk::Rect2D	renderArea{};
	uint32_t	clearValueCount{ 0u };
	const vk::ClearValue*	clearValues{ nullptr };
};


struct CmdBindGraphicsPipeline
{
	HGraphicsPipeline	pipeline{};
};


struct CmdBindComputePipeline
{
	HComputePipeline	pipeline{};
};


struct CmdBindVertexBuffer
{
	Uint32 firstBinding{ 0u };
	Uint32 bindingCount{ 1u };
	const HBuffer* pBuffers;
	const vk::DeviceSize* pOffsets;
};


struct CmdBindIndexBuffer
{
	HBuffer	indexBuffer{};
	vk::DeviceSize offset;
	vk::IndexType indexType;
};


struct CmdDispatch 
{
	Uint32	groupCountX{ 0u };
	Uint32	groupCountY{ 0u };
	Uint32	groupCountZ{ 0u };
};


struct CmdMapBuffer
{
	HBuffer	buffer{};
	vk::DeviceSize	offset{};
	vk::DeviceSize	size{};
	vk::MemoryMapFlags	flags{};
	void*	pData{};
};


struct CmdCopyBuffer
{
	HBuffer	srcBuffer;
	HBuffer	dstBuffer;
	Uint32	regionCount;
	const vk::BufferCopy*	pRegions;
};

}