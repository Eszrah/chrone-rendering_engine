#pragma once

#include <vulkan\vulkan.hpp>

#include "NativeType.h"
#include "ResourceHandle.h"

namespace chrone::rendering
{

using HImage = HandleUI32<struct HImageType>;

struct Buffer 
{
	vk::Buffer	buffer{};
	vk::DeviceMemory memory{};
	Uint32	size{};
};


struct Image 
{
	vk::Image	image{};
	vk::DeviceMemory memory{};

};


struct ImageView 
{
	vk::ImageView	view{};
};


struct GraphicsPipeline
{

};


struct ComputePipeline 
{
};


struct RenderPass 
{
	vk::RenderPass	renderPass;
};


struct Framebuffer 
{

};


struct Fence 
{

};


struct Semaphore 
{

};
/*

struct Image
{
	vk::ImageType imageType{};
	vk::Format format{};
	vk::Extent3D extent{};
	Uint32 mipLevels{};
	Uint32 arrayLayers{};

	vk::Image	image{};
	vk::DeviceMemory memory{};
};


struct ImageViewProperties
{
	vk::ImageViewType	viewType{};
	vk::Format	format{};
	vk::ComponentMapping	components{};
	vk::ImageSubresourceRange	subresourceRange{};
};

struct ImageView
{
	ImageViewProperties	properties{};
	HImage	hImage{};
	vk::ImageView	view{};
};


struct GraphicsPipeline
{
	vk::Pipeline	pipeline{};
	vk::PipelineLayout	layout{};
};


struct RenderPass
{
	vk::RenderPass	renderpass{};
};


struct Framebuffer
{
	vk::Framebuffer	framebuffer{};
};


struct DescriptorSetLayout
{
	std::vector<vk::DescriptorPoolSize>	typesInfo{};
	vk::DescriptorSetLayout	layout{};
};


struct DescriptorAllocator
{
	struct Pool
	{
		Uint32	freeSetsCount{};
		vk::DescriptorPool	descriptorPool{};
	};

	using PoolVector = std::vector<Pool>;

	Uint32	maxSetsCount{};
	HDescriptorSetLayout	hDescriptorSetLayout{};
	PoolVector	pools{};
	//PoolVector	freePools{};
};


struct DescriptorSet
{
	HDescriptorAllocator	hDescriptorAllocator{};
	Uint32	poolIndex{};
	vk::DescriptorSet	descriptorSet{};
};*/
}