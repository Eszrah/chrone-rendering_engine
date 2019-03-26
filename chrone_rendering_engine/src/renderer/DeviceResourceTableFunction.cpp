#include "renderer/DeviceResourceTableFunction.h"

#include "renderer/RenderCommand.h"
#include "renderer/DeviceInstance.h"
#include "renderer/DeviceResourceTable.h"
#include "vulkan_backend/VulkanHelper.h"


namespace chrone::rendering
{

vk::Result 
DeviceResourceTableFunc::AllocateBuffer(
	DeviceResourceTable& resourceTable, 
	const DeviceInstance& device, 
	const HBuffer hBuffer, 
	const CmdBufferCreateInfo& cmd)
{
	Buffer	buffer{};

	vk::Result	result = device.logicalDevice.createBuffer(
		&cmd.createInfo, nullptr, &buffer.buffer);

	if (result != vk::Result::eSuccess) { return result; }

	vk::MemoryRequirements const	memRequirement{
		device.logicalDevice.getBufferMemoryRequirements(buffer.buffer) };

	vk::MemoryAllocateInfo const	allocateInfo{ memRequirement.size,
		MemoryHelper::FindMemoryType(device.physicalDevice,
			memRequirement.memoryTypeBits, cmd.memoryProperties) };

	result = device.logicalDevice.allocateMemory(
		&allocateInfo, nullptr, &buffer.memory);

	if (result != vk::Result::eSuccess) { return result; }

	device.logicalDevice.bindBufferMemory(buffer.buffer, buffer.memory, 0);

	buffer.size = allocateInfo.allocationSize;

	return result;
}


vk::Result 
DeviceResourceTableFunc::AllocateImageView(
	DeviceResourceTable & resourceTable, 
	const DeviceInstance & device, 
	const HImageView hImageView, 
	const CmdImageCreateInfo& cmd)
{
	HImage	hImage{};
	Image	image{};

	if (cmd.creationType == CmdImageCreateInfo::CreationType::Image)
	{
		vk::Result	result = device.logicalDevice.createImage(
			&cmd.imageCreation, nullptr, &image.image);

		if (result != vk::Result::eSuccess) { return result; }

		vk::MemoryRequirements const	memRequirements{
		device.logicalDevice.getImageMemoryRequirements(image.image) };

		vk::MemoryAllocateInfo const	allocateInfo{ memRequirements.size,
			MemoryHelper::FindMemoryType(device.physicalDevice,
				memRequirements.memoryTypeBits,
				cmd.imageCreation.memoryProperties) };

		result = device.logicalDevice.allocateMemory(&allocateInfo, nullptr, &image.memory);

		if (result != vk::Result::eSuccess) { return result; }

		device.logicalDevice.bindImageMemory(image.image, image.memory, { 0 });
	}
	else
	{
		//retrieve the image structure
	}


	const CmdImageCreateInfo::ViewProperties&	viewProperties{
	cmd.viewProperties };

	vk::ImageViewCreateInfo const	viewCreateInfo{ {},
	image.image, viewProperties.viewType, viewProperties.format,
	viewProperties.components, viewProperties.subresourceRange };

	ImageView imageView{ };
	vk::Result	result = device.logicalDevice.createImageView(
		&viewCreateInfo, nullptr, &imageView.view);

	if (result != vk::Result::eSuccess) { return result; }

	Uint32*	imageReferenceViewCount{ nullptr };

	++imageReferenceViewCount;

	return result;
}

vk::Result 
DeviceResourceTableFunc::AllocateRenderPass(
	DeviceResourceTable & resourceTable, 
	const DeviceInstance & device, 
	const HRenderPass hRenderPass, 
	const CmdRenderPassCreateInfo & cmd)
{
	RenderPass	renderPass{};

	vk::Result	result = device.logicalDevice.createRenderPass(
		&cmd.createInfo, nullptr, &renderPass.renderPass);

	return result;
}

}