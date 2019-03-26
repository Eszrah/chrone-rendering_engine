#include "vulkan_backend\Device.h"

#include <set>
#include <string>

#include "vulkan_backend/VulkanHelper.h"


namespace chrone::rendering
{

bool Device::CreateInstance(InstanceCreationInfo const& createInfo)
{
	if (!_CreateInstance(createInfo)) { return false; }

	return true;
}

bool Device::Initialize(InitInfo const& initInfo)
{
	if (!instance) { return false; }

	if (!_SetupDebugCallback(initInfo)) { return false; }
	if (!_CreateSurface(initInfo)) { return false; }
	if (!_ChoosePhysicalDevice()) { return false; }
	if (!_CreateLogicalDevice()) { return false; }
	if (!_CreateSwapChain(initInfo)) { return false; }
	if (!_CreateCommandPools()) { return false; }

	if (!_TransitionSwapChain()) { return false; }

	return true;
}

vk::CommandBuffer Device::BeginSingleTimeCommands(vk::CommandPool pool)
{
	vk::CommandBufferAllocateInfo allocInfo = { pool, vk::CommandBufferLevel::ePrimary, 1 };

	vk::CommandBuffer commandBuffer;
	logicalDevice.allocateCommandBuffers(&allocInfo, &commandBuffer);

	vk::CommandBufferBeginInfo beginInfo = { vk::CommandBufferUsageFlagBits::eOneTimeSubmit };

	commandBuffer.begin(&beginInfo);

	return commandBuffer;
}

void Device::EndSingleTimeCommands(vk::Queue queue, vk::CommandPool pool, vk::CommandBuffer commandBuffer)
{
	commandBuffer.end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	queue.submit(1, &submitInfo, {});
	queue.waitIdle();

	logicalDevice.freeCommandBuffers(pool, 1, &commandBuffer);
}

vk::Result Device::AcquireNextImageKHR(vk::Semaphore semaphore, vk::Fence fence, Uint32 * pImageIndex)
{
	return logicalDevice.acquireNextImageKHR(swapchain.swapChain, std::numeric_limits<uint64_t>::max(), semaphore, fence, pImageIndex);
}

bool Device::_CreateInstance(InstanceCreationInfo const& createInfo)
{
	vk::ApplicationInfo	const	apInfo { "My Vulkan App", 
		VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), 
		VK_API_VERSION_1_0 };


	std::vector<char const*>	extensions(createInfo.requiredExtensions,
		createInfo.requiredExtensions + createInfo.extensionsCount);

	Uint32	validationLayerCount{ 0u };
	char const* const*	validationLayers{ nullptr };

	if (createInfo.enableDebugLayers)
	{ 
		_GetDebugExtensions(extensions); 
		validationLayerCount = static_cast<Uint32>(_validationLayers.size());
		validationLayers = _validationLayers.data();
	}


	vk::InstanceCreateInfo	const instanceInfo{ {}, &apInfo,
		validationLayerCount, validationLayers,
		static_cast<Uint32>(extensions.size()), extensions.data() };

	instance = vk::createInstance(instanceInfo);
	_enableDebugLayers = createInfo.enableDebugLayers;

	return (instance != vk::Instance{});
}


bool Device::_SetupDebugCallback(InitInfo const& initInfo)
{
	if (!_enableDebugLayers) return true;

	vk::DebugUtilsMessengerCreateInfoEXT const	createInfo
	{
		vk::DebugUtilsMessengerCreateFlagsEXT(),
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		//vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | it floods a lot
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
		initInfo.debugCallback, initInfo.callbackUserData };

	//loading all extensions for this instance
	_dldi = vk::DispatchLoaderDynamic{ instance };

	if (!_dldi.vkCreateDebugUtilsMessengerEXT)
	{
		return false;
	}


	if (instance.createDebugUtilsMessengerEXT(&createInfo, nullptr, 
		&_dbgMessenger, _dldi) != vk::Result::eSuccess)
	{
		return false;
	}

	return true;
}


bool Device::_CreateSurface(InitInfo const& initInfo)
{
	VkSurfaceKHR internalSurface{ initInfo.internalSurface };

	if (!internalSurface) { return false; }

	surface = vk::SurfaceKHR{ internalSurface };
	return true;
}

bool Device::_ChoosePhysicalDevice()
{
	//gathering all devices
	auto const	physicalDevices{ instance.enumeratePhysicalDevices() };

	//picking the first which is suitable
	for (const auto& device : physicalDevices)
	{
		if (_IsDeviceSuitable(device, _queueFamilyIndices))
		{
			physicalDevice = device;
			break;
		}
	}

	return (physicalDevice != vk::PhysicalDevice{});
}


bool Device::_IsDeviceSuitable(vk::PhysicalDevice device, QueueFamilyIndices& queueIndices)
{
	//Here we could have get the physical device's feature and properties;
	//vk::PhysicalDeviceProperties prop = device.getProperties();

	//we will just pick the first one we get
	queueIndices = _FindDeviceQueueFamilies(device);
	bool const	extensionSupported{ 
		_CheckDeviceExtensionSupport(device, _deviceExtensions) };

	bool swapChainAdequate{ false };

	//This is important to not check this if the extension is not supported
	if (extensionSupported)
	{
		auto const	swapChainSupportDetails{ _QuerySwapChainSupport(device, surface) };
		swapChainAdequate = !swapChainSupportDetails.presentModes.empty() &&
			!swapChainSupportDetails.formats.empty();
	}


	auto const	deviceFeatures{ device.getFeatures() };

	return queueIndices.isComplete() && extensionSupported &&
		swapChainAdequate && deviceFeatures.samplerAnisotropy;
}


QueueFamilyIndices Device::_FindDeviceQueueFamilies(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices{};
	VkBool32 presentSupport = false;

	auto const	queueFamilies{ device.getQueueFamilyProperties() };

	for (auto index{ 0u }; index < queueFamilies.size(); ++index)
	{
		vk::QueueFamilyProperties const& currentFamily{ queueFamilies[index] };

		//checking if this family has at least one queue which support graphics and compute
		if (currentFamily.queueCount > 0 &&
			currentFamily.queueFlags & vk::QueueFlagBits::eGraphics &&
			currentFamily.queueFlags & vk::QueueFlagBits::eCompute)
		{
			indices.graphicsFamily = index;
		}

		presentSupport = device.getSurfaceSupportKHR(index, surface);

		//checking if the family can present to a window
		if (currentFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = index;
		}

		if (currentFamily.queueCount > 0 &&
			currentFamily.queueFlags & vk::QueueFlagBits::eTransfer &&
			!(currentFamily.queueFlags & vk::QueueFlagBits::eGraphics))
		{
			//we want another queue
			indices.transferFamily = index;
		}

		if (indices.isComplete())
		{
			break;
		}
	}

	return indices;
}

bool Device::_CheckDeviceExtensionSupport(vk::PhysicalDevice device, 
	std::vector<const char*> const& checkedExtensions)
{
	//Checking for swap chain support

	auto const	availableExtensions{ device.enumerateDeviceExtensionProperties() };

	std::set<std::string>	requiredExtensions{ checkedExtensions.cbegin(), checkedExtensions.cend() };

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}


bool Device::_CreateLogicalDevice()
{
	std::vector<vk::DeviceQueueCreateInfo> queuesCreateInfo;
	std::set<int> const	uniqueQueueFamilies{ _queueFamilyIndices.graphicsFamily,
		_queueFamilyIndices.presentFamily, _queueFamilyIndices.transferFamily };

	float const	queuePriority{ 1.0f };


	for (int queueFamily : uniqueQueueFamilies)
	{
		vk::DeviceQueueCreateInfo const	queueCreateInfo{ {},
			(Uint32)queueFamily, 1, &queuePriority };

		queuesCreateInfo.push_back(queueCreateInfo);
	}

	vk::PhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	//creating the logical device
	vk::DeviceCreateInfo const	deviceCreateInfo{
		{}, (Uint32)queuesCreateInfo.size(), queuesCreateInfo.data(),
		static_cast<Uint32>(_validationLayers.size()), _validationLayers.data(),
		static_cast<Uint32>(_deviceExtensions.size()), _deviceExtensions.data(), 
		&deviceFeatures };

	if (physicalDevice.createDevice(&deviceCreateInfo, nullptr, &logicalDevice) != vk::Result::eSuccess)
	{
		return false;
	}


	graphicsQueue = logicalDevice.getQueue(_queueFamilyIndices.graphicsFamily, 0);
	presentQueue = logicalDevice.getQueue(_queueFamilyIndices.presentFamily, 0);
	transferQueue = logicalDevice.getQueue(_queueFamilyIndices.transferFamily, 0);
	return true;
}


bool Device::_CreateSwapChain(InitInfo const& initInfo)
{
	SwapChainSupportDetails const	swapChainSupport{ _QuerySwapChainSupport(physicalDevice, surface) };
	swapchain.format = _ChooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR const	presentMode{ _ChooseSwapPresentMode(swapChainSupport.presentModes) };
	swapchain.extent = _ChooseSwapExtent(swapChainSupport.capabilities, initInfo.windowBaseWidth, initInfo.windowBaseHeight);

	//selecting how many images we wamt in the swapchain
	Uint32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

	//A value of 0 for maxImageCount means that there is no limit besides memory requirements, which is why we need to check for that.
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
		imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	Uint32 const	imageArrayLayers = 1;
	vk::ImageUsageFlags const	usageBM{ vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst };

	//Creating the swap chain
	vk::SwapchainCreateInfoKHR	createInfo {
		{}, surface, imageCount, swapchain.format.format, 
		swapchain.format.colorSpace, swapchain.extent, 
		imageArrayLayers, usageBM };

	Uint32 const queueFamilyIndices[] = { 
		(Uint32)_queueFamilyIndices.graphicsFamily, 
		(Uint32)_queueFamilyIndices.presentFamily };

	if (queueFamilyIndices[0] != queueFamilyIndices[1])
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = true;
	createInfo.oldSwapchain = vk::SwapchainKHR{};

	if (logicalDevice.createSwapchainKHR(
		&createInfo, nullptr, &swapchain.swapChain) != vk::Result::eSuccess)
	{
		//throw std::runtime_error("failed to create swap chain!");
		return false;
	}

	//-> the size could be different from minImageCount
	//Note that when we created the swap chain, we passed the number of desired images to a field called minImageCount. The implementation is allowed to create more images, which is why we need to explicitly query the amount again.
	swapchain.images = logicalDevice.getSwapchainImagesKHR(swapchain.swapChain);
	_width = initInfo.windowBaseWidth;
	_height = initInfo.windowBaseHeight;

	auto const&	swapchainImages{ swapchain.images };
	auto&	swapChainImageViews{ swapchain.imageViews };
	swapChainImageViews.resize(swapchainImages.size());

	vk::ImageSubresourceRange const range{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

	for (auto i = 0; i < swapChainImageViews.size(); i++)
	{
		//swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainFormat.format, vk::ImageAspectFlagBits::eColor, 1);
		vk::ImageSubresourceRange const range{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
		vk::ImageViewCreateInfo const imageViewCI{ {}, swapchainImages[i], vk::ImageViewType::e2D, swapchain.format.format, vk::ComponentMapping{}, range };

		if (logicalDevice.createImageView(&imageViewCI, nullptr, &swapChainImageViews[i].view) != vk::Result::eSuccess)
		{
			return false;
		}
	
		swapChainImageViews[i].properties.format = imageViewCI.format;
	}

	return true;
}

SwapChainSupportDetails Device::_QuerySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{
	/*Just checking if a swap chain is available is not sufficient, because it may not actually be compatible with our window surface. Creating a swap chain also involves a lot more settings than instance and device creation, so we need to query for some more details before we are able to proceed.There are basically three kinds of properties we need to check:•Basic surface capabilities (min/max number of images in swap chain,min/max width and height of images)•Surface formats (pixel format, color space)•Available presentation modes*/

	SwapChainSupportDetails details;

	details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
	details.formats = device.getSurfaceFormatsKHR(surface);
	details.presentModes = device.getSurfacePresentModesKHR(surface);

	return details;
}

vk::SurfaceFormatKHR Device::_ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	/*
	•Surface format (color depth)
	•Presentation mode (conditions for swapping images to the screen)
	•Swap extent (resolution of images in swap chain)
	*/

	//•Surface format(color depth)
	if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
		return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
	}

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

vk::PresentModeKHR Device::_ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes)
{
	//•Presentation mode(conditions for swapping images to the screen)

	//VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away, which may result in tearing.

	//	VK_PRESENT_MODE_FIFO_KHR : The swap chain is a queue where the display takes an image from the front of the queue when the display is refreshed and the program inserts rendered images at the back of the queue.If the queue is full then the program has to wait.This is most similar to vertical sync as found in modern games.The moment that the display is refreshed is known as "vertical blank".

	//	VK_PRESENT_MODE_FIFO_RELAXED_KHR : This mode only differs from the previous one if the application is late and the queue was empty at the last vertical blank.Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives.This may result in visible tearing.

	//	VK_PRESENT_MODE_MAILBOX_KHR : This is another variation of the second mode.Instead of blocking the application when the queue is full, the images that are already queued are simply replaced with the newer ones.This mode can be used to implement triple buffering, which allows you to avoid tearing with significantly less latency issues than standard vertical sync that uses double buffering.

	//only vk::PresentModeKHR::eFifo mode is guaranteed to be available

	vk::PresentModeKHR	bestMode{ vk::PresentModeKHR::eFifo };

	//it allows triple buffering without tearing and with a low latency
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == vk::PresentModeKHR::eMailbox)
		{
			return availablePresentMode;
		}
		else if (availablePresentMode == vk::PresentModeKHR::eImmediate)
		{
			//Unfortunately some drivers currently don't properly support VK_PRESENT_MODE_FIFO_KHR, so we should prefer VK_PRESENT_MODE_IMMEDIATE_KHR if VK_PRESENT_MODE_MAILBOX_KHR is not available:
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

vk::Extent2D Device::_ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR & capabilities, Uint32 width, Uint32 height)
{
	//•Swap extent (resolution of images in swap chain)

	if (capabilities.currentExtent.width != std::numeric_limits<float>::max())
	{
		return capabilities.currentExtent;
	}

	else
	{
		VkExtent2D actualExtent = { width, height };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}


bool Device::_CreateCommandPools()
{
	vk::CommandPoolCreateInfo const	graphicsPoolCreateInfo{ vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		static_cast<Uint32>(_queueFamilyIndices.graphicsFamily) };

	vk::CommandPoolCreateInfo const	transferPoolCreateInfo{ {},
		static_cast<Uint32>(_queueFamilyIndices.transferFamily) };

	if (logicalDevice.createCommandPool(&graphicsPoolCreateInfo, 
		nullptr, &graphicsCommandPool) != vk::Result::eSuccess)
	{
		return false;
	}

	if (logicalDevice.createCommandPool(&transferPoolCreateInfo, 
		nullptr, &transferCommandPool) != vk::Result::eSuccess)
	{
		return false;
	}

	return true;
}

void Device::_GetDebugExtensions(std::vector<char const*>& extensions)
{
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
}

bool Device::_TransitionSwapChain()
{
	vk::ImageSubresourceRange const range{ vk::ImageAspectFlagBits::eColor, 0,
	1, 0, 1 };

	const auto& swapChainImages = swapchain.images;

	for (auto currentImage : swapChainImages)
	{
		//general layout transition
		vk::ImageMemoryBarrier const	imageMemoryBarrier{
			(vk::AccessFlagBits)0,
			vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
			vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR,
			VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
			currentImage, range };

		vk::CommandPool	transferPool{ transferCommandPool };

		vk::CommandBuffer transferCommandBuffer{ BeginSingleTimeCommands(transferPool) };
		transferCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits{}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		EndSingleTimeCommands(transferQueue, transferPool, transferCommandBuffer);

	}


	return true;
}

}