#pragma once

#include <vulkan\vulkan.hpp>

#include "BackendType.h"
#include "NativeType.h"



namespace chrone::rendering
{


class Device
{
public:

	struct InstanceCreationInfo
	{
		bool	enableDebugLayers{ true };
		Uint32	extensionsCount{ 0u };
		char const**	requiredExtensions{ nullptr };
	};

	struct InitInfo
	{
		PFN_vkDebugUtilsMessengerCallbackEXT	debugCallback{ nullptr };
		void*	callbackUserData{ nullptr };

		Uint32	windowBaseWidth{};
		Uint32	windowBaseHeight{};
		VkSurfaceKHR internalSurface{};
	};

	bool	CreateInstance(InstanceCreationInfo const& createInfo);
	bool	Initialize(InitInfo const& initInfo);

	vk::CommandBuffer BeginSingleTimeCommands(vk::CommandPool pool);
	void EndSingleTimeCommands(vk::Queue queue, vk::CommandPool pool, vk::CommandBuffer commandBuffer);


	vk::Result AcquireNextImageKHR(vk::Semaphore semaphore, vk::Fence fence, Uint32* pImageIndex);

	bool	_CreateInstance(InstanceCreationInfo const& createInfo);
	bool	_SetupDebugCallback(InitInfo const& initInfo);
	bool	_CreateSurface(InitInfo const& initInfo);
	
	bool	_ChoosePhysicalDevice();
	bool	_IsDeviceSuitable(vk::PhysicalDevice device, QueueFamilyIndices& queueIndices);
	QueueFamilyIndices	_FindDeviceQueueFamilies(vk::PhysicalDevice device);
	static bool	_CheckDeviceExtensionSupport(vk::PhysicalDevice device, std::vector<const char*> const& checkedExtensions);

	bool	_CreateLogicalDevice();

	bool	_CreateSwapChain(InitInfo const& initInfo);
	static SwapChainSupportDetails	_QuerySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);
	static vk::SurfaceFormatKHR _ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
	static vk::PresentModeKHR _ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes);
	static vk::Extent2D _ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, Uint32 width, Uint32 height);


	bool	_CreateCommandPools();

	static void	_GetDebugExtensions(std::vector<char const*>& extensions);

	bool _TransitionSwapChain();

	Uint32	_width{};
	Uint32	_height{};
	vk::Instance	instance{};
	vk::PhysicalDevice	physicalDevice{};
	vk::Device	logicalDevice{};
	vk::DispatchLoaderDynamic _dldi{};
	vk::DebugUtilsMessengerEXT	_dbgMessenger{};

	bool	_enableDebugLayers{ true };
	const std::vector<const char*>	_deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const std::vector<const char*>	_validationLayers{ "VK_LAYER_LUNARG_standard_validation" };

	vk::SurfaceKHR	surface{};

	Swapchain	swapchain{};
	QueueFamilyIndices	_queueFamilyIndices{};

	//queues
	vk::Queue	graphicsQueue{};
	vk::Queue	presentQueue{};
	vk::Queue	transferQueue{};

	//command pools
	vk::CommandPool graphicsCommandPool{};
	vk::CommandPool transferCommandPool{};
};


}