#pragma once

#include "RenderCommand.h"
#include "DeviceResource.h"
#include "vulkan_backend/VulkanHelper.h"

namespace chrone::rendering
{

inline 
vk::Result	
CreateBuffer(
	vk::BufferCreateInfo const&	createInfo,
	vk::MemoryPropertyFlags const&	memoryProperties,
	vk::PhysicalDevice const& physicalDevice, 
	vk::Device const& device, 
	Buffer& buffer)
{
	vk::Result	result = 
		device.createBuffer(&createInfo, nullptr, &buffer.buffer);

	if (result != vk::Result::eSuccess) { return result; }

	vk::MemoryRequirements const	memRequirement{
		device.getBufferMemoryRequirements(buffer.buffer) };

	vk::MemoryAllocateInfo const	allocateInfo{ memRequirement.size,
		MemoryHelper::FindMemoryType(physicalDevice,
			memRequirement.memoryTypeBits, memoryProperties) };

	result = device.allocateMemory(&allocateInfo, nullptr, &buffer.memory);

	if (result != vk::Result::eSuccess) { return result; }

	device.bindBufferMemory(buffer.buffer, buffer.memory, 0);
	return result;
}


inline
vk::Result
CreateImage(
	vk::ImageCreateInfo	const&	createInfo,
	vk::MemoryPropertyFlags const&	memoryProperties,
	vk::PhysicalDevice const& physicalDevice,
	vk::Device const& device,
	Image& image)
{
	vk::Result	result = device.createImage(
		&createInfo, nullptr, &image.image);

	if (result != vk::Result::eSuccess) { return result; }

	vk::MemoryRequirements const	memRequirements{
	device.getImageMemoryRequirements(image.image) };

	vk::MemoryAllocateInfo const	allocateInfo{ memRequirements.size,
		MemoryHelper::FindMemoryType(physicalDevice,
			memRequirements.memoryTypeBits,
			memoryProperties) };

	result = device.allocateMemory(&allocateInfo, nullptr, &image.memory);

	if (result != vk::Result::eSuccess) { return result; }

	device.bindImageMemory(image.image, image.memory, { 0 });
}


}