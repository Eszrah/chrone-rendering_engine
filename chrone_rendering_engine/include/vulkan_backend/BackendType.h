#pragma once

#include <vector>
#include <vulkan\vulkan.hpp>

#include "NativeType.h"
#include "renderer/RendererResource.h"

namespace chrone::rendering
{


struct QueueFamilyIndices
{
	Int graphicsFamily = -1;
	Int presentFamily = -1;
	Int transferFamily = -1;

	inline bool isComplete() const { return graphicsFamily >= 0 && 
		presentFamily >= 0 && transferFamily >= 0; }
};


struct SwapChainSupportDetails
{
	vk::SurfaceCapabilitiesKHR	capabilities{};
	std::vector<vk::SurfaceFormatKHR>	formats{};
	std::vector<vk::PresentModeKHR>	presentModes{};
};


struct Swapchain
{
	vk::SwapchainKHR	swapChain{};
	vk::SurfaceFormatKHR	format{};
	vk::Extent2D	extent{};
	std::vector<vk::Image>	images{};
	std::vector<ImageView>	imageViews{};
};

}