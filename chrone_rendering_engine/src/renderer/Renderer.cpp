#include "renderer/Renderer.h"

#include "vulkan_backend/VulkanHelper.h"
#include "renderer/MeshType.h"

#include <iostream>
#include <fstream>

namespace chrone::rendering
{

bool Renderer::CreateInstance(InstanceCreationInfo const& createInfo)
{
	Device::InstanceCreationInfo const instanceCI{ 
		createInfo.enableDebugLayers, 
		createInfo.extensionsCount, 
		createInfo.requiredExtensions };

	if (!_device._CreateInstance(instanceCI)) { return false; }

	return true;
}

bool Renderer::Initialize(InitInfo const& initInfo)
{
	Device::InitInfo const instanceCI{
	initInfo.debugCallback,
	initInfo.callbackUserData,
	initInfo.windowBaseWidth,
	initInfo.windowBaseHeight,
	initInfo.internalSurface };

	if (!_device.Initialize(instanceCI)) { return false; }

	const vk::CommandBufferAllocateInfo graphicsAllocInfo = { _device.graphicsCommandPool, vk::CommandBufferLevel::ePrimary, 1 };
	const vk::CommandBufferAllocateInfo transferAllocInfo = { _device.transferCommandPool, vk::CommandBufferLevel::ePrimary, 1 };

	_device.logicalDevice.allocateCommandBuffers(&graphicsAllocInfo, &graphicsCommandBuffer);
	_device.logicalDevice.allocateCommandBuffers(&transferAllocInfo, &transferCommandBuffer);


	_resourceManager._device = &_device;
	commandsInterface.device = &_device;
	commandsInterface.resourceManager = &_resourceManager;
	commandsInterface.graphicsCommandBuffer = &graphicsCommandBuffer;
	commandsInterface.transferCommandBuffer = &transferCommandBuffer;
	
	commandsInterface.graphicsQueue = &_device.graphicsQueue;
	commandsInterface.transferQueue = &_device.transferQueue;

	vk::FenceCreateInfo const fenceCreateInfo{
		vk::FenceCreateFlagBits::eSignaled
	};	

	_device.logicalDevice.createFence(&fenceCreateInfo, nullptr, &frameFence);


	vk::SemaphoreCreateInfo const	semaphoreCreateInfo{};
	_device.logicalDevice.createSemaphore(&semaphoreCreateInfo, nullptr, &frameSemaphore);




	return true;
}

bool Renderer::Shutdown()
{
	return false;
}


bool Renderer::Submit(CommandBufferSubmition const& submition)
{
	//Processing resources commands
	Uint32	resourceCommandBufferCount{ submition.resourceCommandBufferCount };
	ResourceCommandBuffer_Deprecated const* const*	resourceCommandBuffers{ submition.resourceCommandBuffers };

	for (auto cbIndex{ 0u }; cbIndex < resourceCommandBufferCount; ++cbIndex)
	{
		auto const*	cb{ resourceCommandBuffers[cbIndex] };
		auto const&	commands{ cb->commands };

		for (auto const& command : commands)
		{
			CommandFunctorSignature functor = command.functor;
			(*command.functor)(commandsInterface, command.data);
		}
	}

	_device.transferQueue.waitIdle();

	Uint32	renderCommandBufferCount{ submition.renderCommandBufferCount };
	RenderCommandBuffer const* const*	renderCommandBuffers{ submition.renderCommandBuffers };

	for (auto cbIndex{ 0u }; cbIndex < renderCommandBufferCount; ++cbIndex)
	{
		auto const*	cb{ renderCommandBuffers[cbIndex] };
		auto const&	commands{ cb->commands };

		for (auto const& command : commands)
		{
			(*command.functor)(commandsInterface, command.data);
		}
	}

	_device.transferQueue.waitIdle();
	_device.graphicsQueue.waitIdle();

	return true;
}

bool Renderer::AcquireImage()
{
	_device.logicalDevice.resetFences(1, &frameFence);
	_device.AcquireNextImageKHR({}, frameFence, &imageIndex);
	_device.logicalDevice.waitForFences(1, &frameFence, true, std::numeric_limits<uint64_t>::max());

	vk::ImageSubresourceRange const range{ vk::ImageAspectFlagBits::eColor, 0,
		1, 0, 1 };

	//transition the image layout
	vk::Image currentImage{ _device.swapchain.images[imageIndex] };

	vk::ImageMemoryBarrier const	imageMemoryBarrier{
	vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
	vk::AccessFlagBits::eMemoryWrite,
	vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eTransferDstOptimal,
	VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
	currentImage, range };

	vk::CommandPool	transferPool{ _device.transferCommandPool };

	vk::CommandBuffer transferCommandBuffer{ _device.BeginSingleTimeCommands(transferPool) };
	transferCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits{}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	_device.EndSingleTimeCommands(_device.transferQueue, transferPool, transferCommandBuffer);


	return true;
}

bool Renderer::CopyFrambeufferToSwapChain(HImageView hImageView)
{
	ImageView*	imageView = _resourceManager.GetResource(hImageView);
	Image*	image = _resourceManager.GetResource(imageView->hImage);
	vk::Image	swapChainCurrentImage{ _device.swapchain.images[imageIndex] };

	//imageView's image already in general layout
	{
		vk::ImageSubresourceRange const range{ vk::ImageAspectFlagBits::eColor, 0,
	1, 0, 1 };

		//transition the image layout
		vk::ImageMemoryBarrier const	imageMemoryBarrier{
		vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
		vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
		vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		image->image, range };

		vk::CommandPool	transferPool{ _device.transferCommandPool };

		vk::CommandBuffer transferCommandBuffer{ _device.BeginSingleTimeCommands(transferPool) };
		transferCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits{}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		_device.EndSingleTimeCommands(_device.transferQueue, transferPool, transferCommandBuffer);

	}
	
	
	vk::CommandBuffer graphicsCommandBuffer{ 
		_device.BeginSingleTimeCommands(_device.graphicsCommandPool) };

	vk::ImageCopy const	imageCopy{
		vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, vk::Offset3D{},
		vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, vk::Offset3D{}, image->extent };

	graphicsCommandBuffer.copyImage(image->image, vk::ImageLayout::eTransferSrcOptimal, swapChainCurrentImage, vk::ImageLayout::eTransferDstOptimal, 1, &imageCopy);

	_device.EndSingleTimeCommands(_device.graphicsQueue, _device.graphicsCommandPool, graphicsCommandBuffer);


	//imageView's image already in general layout
	{
		vk::ImageSubresourceRange const range{ vk::ImageAspectFlagBits::eColor, 0,
	1, 0, 1 };

		//transition the image layout
		vk::ImageMemoryBarrier const	imageMemoryBarrier{
		vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
		vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
		vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral,
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
		image->image, range };

		vk::CommandPool	transferPool{ _device.transferCommandPool };

		vk::CommandBuffer transferCommandBuffer{ _device.BeginSingleTimeCommands(transferPool) };
		transferCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits{}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		_device.EndSingleTimeCommands(_device.transferQueue, transferPool, transferCommandBuffer);

	}

	return true;
}

bool Renderer::SwapImage()
{
	vk::ImageSubresourceRange const range{ vk::ImageAspectFlagBits::eColor, 0,
	1, 0, 1 };

	//transition the image layout
	vk::Image currentImage{ _device.swapchain.images[imageIndex] };

	vk::ImageMemoryBarrier const	imageMemoryBarrier{
	vk::AccessFlagBits::eMemoryWrite,
	vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
	vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
	VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
	currentImage, range };

	vk::CommandPool	transferPool{ _device.transferCommandPool };

	vk::CommandBuffer transferCommandBuffer{ _device.BeginSingleTimeCommands(transferPool) };
	transferCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits{}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	_device.EndSingleTimeCommands(_device.transferQueue, transferPool, transferCommandBuffer);

	vk::SwapchainKHR const	swapChains[] = { _device.swapchain.swapChain };

	vk::PresentInfoKHR const	presentInfo{
		0, nullptr, 1, swapChains, &imageIndex, nullptr
	};

	_device.presentQueue.presentKHR(&presentInfo);

	return true;
}


}