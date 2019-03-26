#pragma once

#include "vulkan_backend/BackendType.h"
#include "vulkan_backend/Device.h"
#include "renderer/CommandBuffer.h"
#include "renderer/RendererResourceManager.h"

namespace chrone::rendering
{

class Renderer
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
	bool	Shutdown();
	vk::Instance	GetInstance() { return _device.instance; }
	/*
	Provide a way to retrieve queue family indices:
	design flaw but don't care for the moment
	*/

	/*
	Resource allocation:
	- buffer, buffer views
	- image, image views
	- graphics pipeline
	- Shader stage
	- renderpass
	*/
	

	bool	Submit(CommandBufferSubmition const& submition);

	//shitty thing
	bool	CreateResourceCmdBuffer(ResourceCommandBuffer_Deprecated& cmdBuffer) { cmdBuffer.rendererResourceManager = &_resourceManager; return true; }
	bool	CreateResourceCmdBuffer(RenderCommandBuffer& cmdBuffer) { cmdBuffer.rendererResourceManager = &_resourceManager; return true; }

	vk::SurfaceFormatKHR	GetSwapChainImageFormat() const { return _device.swapchain.format; }
	std::vector<Uint32>	GetQueueIndices() const { return {(Uint32)_device._queueFamilyIndices.graphicsFamily, (Uint32)_device._queueFamilyIndices.transferFamily}; }

	bool	AcquireImage();
	bool	CopyFrambeufferToSwapChain(HImageView hImageView);
	bool	SwapImage();


private:

	vk::CommandBuffer	graphicsCommandBuffer{};
	vk::CommandBuffer	transferCommandBuffer{};

	Uint32	imageIndex{ 0 };
	vk::Semaphore	frameSemaphore{};
	vk::Fence	frameFence{};
	RendererResourceManager	_resourceManager{};
	CommandBufferInterface	commandsInterface{};
	Device	_device{ };
};

}

/*


//bool Renderer::AcquireImage()
//{
//	//uint32_t imageIndex;
//	_device->_logicalDevice.acquireNextImageKHR(_device->_swapchain.swapChain,
//		std::numeric_limits<uint64_t>::max(),
//		_imageAvailableSemaphores[_currentFrame], {}, &_currentImageIndex);
//
//
//	return true;
//}
//
//bool Renderer::SwapImage()
//{
//	vk::Semaphore const* waitSwapSemaphore{ &_renderFinishedSemaphores[_currentFrame] };
//	vk::SwapchainKHR const	swapChains[] = { _device->_swapchain.swapChain };
//
//	vk::PresentInfoKHR const	presentInfo{
//		1, waitSwapSemaphore, 1, swapChains, &_currentImageIndex, nullptr
//	};
//
//	_device->_transferQueue.waitIdle();
//	_device->_graphicsQueue.waitIdle();
//	_device->_presentQueue.presentKHR(&presentInfo);
//
//	return false;
//}
//
//bool Renderer::CreateDeviceBuffer(Uint32 size, void const* data, vk::BufferUsageFlagBits usage, Buffer& deviceBuffer)
//{
//	vk::DeviceSize const	bufferSize{ size };
//	vk::BufferCreateInfo const	stagingCI{ BufferHelper::
//		CreateTransferDstExclusiveBufferCreateInfo(bufferSize) };
//
//	Buffer	stagingBuffer{};
//
//	if (!_device->CreateBuffer(stagingCI,
//		MemoryPropertyHelper::eHostCoherentVisible, stagingBuffer))
//	{
//		return false;
//	}
//
//	/*Mapping the memory*/
//	void*	mappedData{ _device->_logicalDevice.mapMemory(
//		stagingBuffer.memory, 0, stagingCI.size,{}) };
//
//	memcpy(mappedData, data, bufferSize);
//
//	auto const&	queueFamilyIndices{ _device->_queueFamilyIndices };
//	Uint32 const queueIndices[] = { (Uint32)queueFamilyIndices.graphicsFamily,
//		(Uint32)queueFamilyIndices.transferFamily };
//
//	vk::BufferCreateInfo const	deviceBufferCI{ {}, bufferSize, 
//		vk::BufferUsageFlagBits::eTransferDst | usage,
//		vk::SharingMode::eConcurrent, 2, queueIndices };
//
//	if (!_device->CreateBuffer(deviceBufferCI,
//		vk::MemoryPropertyFlagBits::eDeviceLocal, deviceBuffer))
//	{
//		return false;
//	}
//
//	CopyBuffer(stagingBuffer, deviceBuffer, stagingBuffer.size);
//	_device->DestroyBuffer(stagingBuffer);
//
//	return true;
//}
//
//bool Renderer::CopyBuffer(Buffer& dstBuffer, Buffer const& srcBuffer, vk::DeviceSize size)
//{
//	vk::CommandBuffer commandBuffer{ 
//		_BeginSingleTimeCommands(_device->_transferCommandPool) };
//
//	vk::BufferCopy const	copy{ 0, 0, size };
//	commandBuffer.copyBuffer(srcBuffer.buffer, dstBuffer.buffer, 1, &copy);
//
//	_EndSingleTimeCommands(_device->_transferQueue,
//		_device->_transferCommandPool, commandBuffer);
//
//	return true;
//}
//
//bool Renderer::CreateIndexedMesh(
//	const Uint32 indicesCount, const Uint32* indices, 
//	const Uint32 verticesCount, const Decimal32* vertices, IndexedMesh& mesh)
//{
//
//	vk::DeviceSize const	bufferSize = sizeof(vertices[0]) * verticesCount;
//
//	IndexedVertexBatch&	indexedVertices{ mesh.indexedVertices };
//
//	indexedVertices.indexType = vk::IndexType::eUint32;
//
//	//Index buffer
//	if (!CreateDeviceBuffer(sizeof(indices[0]) * indicesCount, indices,
//		vk::BufferUsageFlagBits::eIndexBuffer, indexedVertices.indexBuffer))
//	{
//		return false;
//	}
//
//	//Vertex buffer
//	if (!CreateDeviceBuffer(sizeof(vertices[0]) * verticesCount, vertices, 
//		vk::BufferUsageFlagBits::eVertexBuffer, indexedVertices.vertexBuffer))
//	{
//		_device->DestroyBuffer(indexedVertices.indexBuffer);
//		return false;
//	}
//
//	//UBO
//	vk::BufferCreateInfo const	uboCI{ {}, sizeof(mesh.matrix), 
//		vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive };
//	
//	if (!_device->CreateBuffer(uboCI, 
//		MemoryPropertyHelper::eHostCoherentVisible, mesh.uboMatrix))
//	{
//		return false;
//	}
//
//	mesh.indexedVertices.indexCount = indicesCount;
//
//	return true;
//}
//
//bool Renderer::CreateDefaultGraphicsPipeline(
//	PipelineShaderStages const& modules, vk::PipelineLayout pipelineLayout,
//	vk::PipelineVertexInputStateCreateInfo const& VIInfo, vk::Pipeline& pipeline)
//{
//	auto const	inputAssembly{ InputAssemblyStateHelper::defaultInputState };
//	
//	auto const	scissorRect{ vk::Rect2D{ {0, 0}, _device->_swapchain.extent } };
//	auto const	viewport{ ViewportHelper::CreateViewportCreateInfo(
//		static_cast<Decimal32>(_device->_width), 
//		static_cast<Decimal32>(_device->_height)) };
//
//	vk::PipelineViewportStateCreateInfo const viewportState{ {},
//	1, &viewport, 1, &scissorRect };
//
//	auto const	rasterizerCreateInfo{ RasterizerHelper::defaultRasterizer };
//	auto const	msCreateInfo{ PipelineMultiSampleHelper::defaultPipelineMS };
//	auto const	colorBlendAttachment{ 
//		PipelineColorBlendAttachmentHelper::defaultColorBlend};
//
//	vk::PipelineColorBlendStateCreateInfo const	colorBlending{
//	{}, false, vk::LogicOp::eCopy, 1, &colorBlendAttachment };
//
//	auto const	depthStencilStateCreateInfo{
//	PipelineDepthStencilStateHelper::defaultDepthStencilState };
//
//	vk::GraphicsPipelineCreateInfo const	pipelineCreateInfo{
//		{}, static_cast<uint32_t>(modules.stages.size()), modules.stages.data(),
//			&VIInfo, &inputAssembly, nullptr, &viewportState, &rasterizerCreateInfo,
//			&msCreateInfo, &depthStencilStateCreateInfo, &colorBlending,
//			nullptr, pipelineLayout, _renderPass, 0, vk::Pipeline{}, -1 };
//
//	if (_device->_logicalDevice.createGraphicsPipelines(
//		{}, 1, &pipelineCreateInfo, nullptr, &pipeline) != vk::Result::eSuccess)
//	{
//		return false;
//	}
//
//	return true;
//}
//
//bool Renderer::CreateShaderModule(
//	Uint32 size, char const* code,
//	vk::ShaderModule& shaderModule)
//{
//	vk::ShaderModuleCreateInfo	createInfo{
//		{}, size, reinterpret_cast<const uint32_t*>(code) };
//
//	if (_device->_logicalDevice.createShaderModule(&createInfo, nullptr, &shaderModule)
//		!= vk::Result::eSuccess)
//	{
//		return false;
//	}
//
//	return true;
//}
//
//bool Renderer::DestroyShaderModule(vk::ShaderModule & shaderModule)
//{
//	_device->_logicalDevice.destroyShaderModule(shaderModule);
//	return true;
//}
//
//bool Renderer::_CreateRenderPass()
//{
//	vk::SubpassDependency const	dependency{ VK_SUBPASS_EXTERNAL, 0u,
//		vk::PipelineStageFlagBits::eColorAttachmentOutput, 
//		vk::PipelineStageFlagBits::eColorAttachmentOutput,
//		(vk::AccessFlagBits)0, vk::AccessFlagBits::eColorAttachmentRead | 
//		vk::AccessFlagBits::eColorAttachmentWrite };
//
//	auto const	swapchainFormat{ _device->_swapchain.format };
//
//	depthImage.format = FormatHelper::FindDepthFormat(_device->_physicalDevice);
//
//	vk::AttachmentDescription const	colorAttachment{
//		{}, swapchainFormat.format, vk::SampleCountFlagBits::e1,
//		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
//		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
//		vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
//	};
//
//	vk::AttachmentDescription const	depthAttachment{
//		{}, depthImage.format,
//		vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, 
//		vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, 
//		vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, 
//		vk::ImageLayout::eDepthStencilAttachmentOptimal };
//
//	/*
//	The attachment parameter specifies which attachment to reference by its index in the attachment descriptions array.
//	Our array consists of a single VkAttachmentDescription, so its index is 0.
//	*/
//	vk::AttachmentReference const colorAttachmentRef{ 0, 
//		vk::ImageLayout::eColorAttachmentOptimal };
//	
//	vk::AttachmentReference const depthAttachmentRef{ 1, 
//		vk::ImageLayout::eDepthStencilAttachmentOptimal };
//
//	vk::SubpassDescription const subpass{
//		{}, vk::PipelineBindPoint::eGraphics,
//		0, nullptr, 1, &colorAttachmentRef, nullptr, &depthAttachmentRef };
//
//	std::array<vk::AttachmentDescription, 2> attachments{ 
//		colorAttachment, depthAttachment };
//
//	vk::RenderPassCreateInfo const	renderPassInfo{
//		{}, static_cast<uint32_t>(attachments.size()), attachments.data(), 
//		1, &subpass, 1, &dependency };
//
//	if (_device->_logicalDevice.createRenderPass(
//		&renderPassInfo, nullptr, &_renderPass) != vk::Result::eSuccess)
//	{
//		return false;
//	}
//
//	return true;
//}
//
////bool Renderer::_CreateGraphicsPipeline()
////{
////	auto const vertShaderCode = readFile("./vulkan_tutorial/shaders/vert.spv");
////	auto const fragShaderCode = readFile("./vulkan_tutorial/shaders/frag.spv");
////
////	vk::ShaderModule vertShaderModule{};
////	vk::ShaderModule fragShaderModule{};
////
////	ShaderModuleHelper::CreateShaderModule(_device->_logicalDevice, static_cast<Uint32>(vertShaderCode.size()), vertShaderCode.data(), vertShaderModule);
////	ShaderModuleHelper::CreateShaderModule(_device->_logicalDevice, static_cast<Uint32>(fragShaderCode.size()), fragShaderCode.data(), fragShaderModule);
////
////	vk::PipelineShaderStageCreateInfo vertShaderStageInfo
////	{
////		{}, vk::ShaderStageFlagBits::eVertex,
////		vertShaderModule, "main"
////	};
////
////	vk::PipelineShaderStageCreateInfo fragShaderStageInfo
////	{
////		{}, vk::ShaderStageFlagBits::eFragment,
////		fragShaderModule, "main"
////	};
////
////	vk::PipelineShaderStageCreateInfo	const shaderStages[]{ 
////		vertShaderStageInfo , fragShaderStageInfo };
////
////	/*
////	Vertex input
////
////	Bindings: spacing between data and whether the data is per-vertex or per-instance (see instancing)
////	Attribute descriptions: type of the attributes passed to the vertex shader, which binding to load them from and at which offset
////	*/
////
////	auto bindingDescription = vk::VertexInputBindingDescription{ 0, 
////		sizeof(float) * 5, vk::VertexInputRate::eVertex };
////
////	std::array<vk::VertexInputAttributeDescription, 2> const	
////		attributeDescriptions{
////	vk::VertexInputAttributeDescription{ 0, 0, vk::Format::eR32G32B32Sfloat, 0 },
////	vk::VertexInputAttributeDescription{ 0, 0, vk::Format::eR32G32B32Sfloat, 
////		sizeof(float) * 3}
////	};
////
////	vk::PipelineVertexInputStateCreateInfo const vertexInputInfo{ {},
////		1, &bindingDescription, 
////		static_cast<uint32_t>(attributeDescriptions.size()), 
////		attributeDescriptions.data() };
////
////	/*
////	Input assembly
////
////	The VkPipelineInputAssemblyStateCreateInfo struct describes two things: what kind of geometry will be drawn from the vertices and if primitive restart should be enabled. The former is specified in the topology member and can have values like:
////	*/
////
////	vk::PipelineInputAssemblyStateCreateInfo const inputAssembly{
////		{}, vk::PrimitiveTopology::eTriangleList, false };
////
////	/*
////	Viewports and scissors
////
////	*/
////	auto const	swapChainExtent{ _device->_swapchain.extent};
////
////	vk::Viewport const	viewport{
////		0.0f, 0.0f,
////		static_cast<float>(swapChainExtent.width), 
////		static_cast<float>(swapChainExtent.height), 0.0f, 1.0f };
////
////	vk::Rect2D const scissor{ {0, 0}, swapChainExtent };
////
////	vk::PipelineViewportStateCreateInfo const viewportState{ {},
////		1, &viewport, 1, &scissor };
////
////
////	/*
////	Rasterizer
////	*/
////
////	vk::PipelineRasterizationStateCreateInfo rasterizer{
////		{}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
////		vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f };
////
////
////	/*
////	Multisampling
////	*/
////
////	vk::PipelineMultisampleStateCreateInfo const multisampling{
////		{}, vk::SampleCountFlagBits::e1, false, 1.0f };
////
////
////	/*
////	Color blending
////
////	There are two types of structs to configure color blending.
////	The first struct, VkPipelineColorBlendAttachmentState contains the configuration per attached framebuffer
////	and the second struct, VkPipelineColorBlendStateCreateInfo contains the global color blending settings. In our case we only have one framebuffer:
////	*/
////
////	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
////		//other fields are well set by default
////	};
////
////	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | 
////		vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | 
////		vk::ColorComponentFlagBits::eA;
////
////	vk::PipelineColorBlendStateCreateInfo colorBlending{
////		{}, false, vk::LogicOp::eCopy, 1, &colorBlendAttachment
////	};
////
////
////	/*Pipeline layout*/
////
////	vk::PipelineLayoutCreateInfo const pipelineLayoutInfo{
////		{}, 1, &_descriptorSetLayout };
////
////	if (_device->_logicalDevice.createPipelineLayout(
////		&pipelineLayoutInfo, nullptr, &_pipelineLayout) != vk::Result::eSuccess)
////	{
////		throw std::runtime_error("failed to create pipeline layout!");
////	}
////
////	vk::PipelineDepthStencilStateCreateInfo const	depthStencilCreateInfo{ {},
////		true, true, vk::CompareOp::eLess, false, false, {}, {}, 0.0f, 1.0f
////	};
////
////	vk::GraphicsPipelineCreateInfo pipelineInfo
////	{
////		{},
////		2, shaderStages,
////		&vertexInputInfo, &inputAssembly, nullptr, &viewportState, &rasterizer,
////		&multisampling, &depthStencilCreateInfo, &colorBlending, nullptr, 
////		_pipelineLayout, _renderPass, 0, vk::Pipeline{}, -1 };
////
////	if (_device->_logicalDevice.createGraphicsPipelines(
////		{}, 1, &pipelineInfo, nullptr, &_graphicsPipeline) 
////		!= vk::Result::eSuccess)
////	{
////		return false;
////	}
////
////	_device->_logicalDevice.destroyShaderModule(vertShaderModule);
////	_device->_logicalDevice.destroyShaderModule(fragShaderModule);
////
////	return true;
////}
//
//bool Renderer::_CreateCommandBuffers()
//{
//	vk::CommandBufferAllocateInfo const	allocInfo{
//	_device->_graphicsCommandPool, vk::CommandBufferLevel::ePrimary, 1 };
//
//	if (_device->_logicalDevice.allocateCommandBuffers(
//		&allocInfo, &_commandBuffer) != vk::Result::eSuccess)
//	{
//		return false;
//	}
//
//	return true;
//}
//
//bool Renderer::_CreateDepthResources()
//{
//	QueueFamilyIndices const&	_queueFamilyIndices{ _device->_queueFamilyIndices };
//	
//	Uint32 const queueIndices[] { 
//		(Uint32)_queueFamilyIndices.graphicsFamily,
//		(Uint32)_queueFamilyIndices.transferFamily };
//	
//	Uint32 const	mipCount{ 1u };
//	vk::Extent2D const&	swapchainExtent{ _device->_swapchain.extent };
//
//	vk::ImageCreateInfo	imageCreateInfo{ 
//		ImageHelper::CreateImage2DSampledConcurrentCreateInfo(
//			swapchainExtent, mipCount, depthImage.format, 
//			vk::SharingMode::eConcurrent, 2, queueIndices) };
//
//	imageCreateInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
//
//	_device->CreateImage(imageCreateInfo, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage);
//	_TransitionImageLayout(depthImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, mipCount, _device->_graphicsQueue, _device->_graphicsCommandPool);
//
//	auto const	viewCreateInfo{ 
//		ImageViewHelper::CreateDefaultImageViewCreateInfo(
//			depthImage, vk::ImageAspectFlagBits::eDepth) };
//
//	if (!_device->CreateImageView(viewCreateInfo, depthImageView))
//	{
//		return false;
//	}
//
//	return true;
//}
//
//bool Renderer::_CreateFramebuffers()
//{
//	auto& const	swapChainImageViews{ _device->_swapchain.imageViews };
//	auto& const	swapChainExtent{ _device->_swapchain.extent };
//	_framebuffers.resize(swapChainImageViews.size());
//
//	for (auto index{ 0u }; index < swapChainImageViews.size(); ++index)
//	{
//		std::array<vk::ImageView, 2> attachments = {
//			swapChainImageViews[index].view, depthImageView.view
//		};
//		vk::FramebufferCreateInfo const createInfo{ {},
//			_renderPass, static_cast<uint32_t>(attachments.size()), 
//			attachments.data(), swapChainExtent.width, swapChainExtent.height, 1u};
//
//		if (_device->_logicalDevice.createFramebuffer(&createInfo, 
//			nullptr, &_framebuffers[index]) != vk::Result::eSuccess)
//		{
//			return false;
//			//throw std::runtime_error("failed to create framebuffer!");
//		}
//	}
//
//	return true;
//}
//
//bool Renderer::_CreateDescriptorSetLayout()
//{
//	//0 because this is the binding in the shader, 1 because this is not an array
//	vk::DescriptorSetLayoutBinding const	uboLayoutBinding{
//		0, vk::DescriptorType::eUniformBuffer, 1,
//		vk::ShaderStageFlagBits::eVertex, nullptr };
//
//	vk::DescriptorSetLayoutBinding const	samplerLayoutBinding{
//		1, vk::DescriptorType::eCombinedImageSampler, 1,
//		vk::ShaderStageFlagBits::eFragment, nullptr };
//
//	std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
//		uboLayoutBinding, samplerLayoutBinding };
//
//	vk::DescriptorSetLayoutCreateInfo const	setLayoutCreateInfo{
//	vk::DescriptorSetLayoutCreateFlagBits{},
//	static_cast<uint32_t>(bindings.size()), bindings.data() };
//
//	if (_device->_logicalDevice.createDescriptorSetLayout(
//		&setLayoutCreateInfo, nullptr, &_descriptorSetLayout) !=
//		vk::Result::eSuccess)
//	{
//		return false;
//		//throw std::runtime_error("failed to create descriptor set layout!");
//	}
//
//	return true;
//}
//
//bool Renderer::_CreateDescriptorPool()
//{
//	std::array<vk::DescriptorPoolSize, 2> poolSizes = {};
//
//	auto const&	swapChainImages{ _device->_swapchain.images };
//
//	poolSizes[0] = vk::DescriptorPoolSize{
//		vk::DescriptorType::eUniformBuffer, 
//		static_cast<uint32_t>(swapChainImages.size()) };
//
//	poolSizes[1] = vk::DescriptorPoolSize{
//		vk::DescriptorType::eCombinedImageSampler, 
//		static_cast<uint32_t>(swapChainImages.size()) };
//
//	vk::DescriptorPoolCreateInfo const	poolCreateInfo{
//		vk::DescriptorPoolCreateFlagBits{}, 
//		static_cast<uint32_t>(swapChainImages.size()), 
//		static_cast<uint32_t>(poolSizes.size()), poolSizes.data() };
//
//	if (_device->_logicalDevice.createDescriptorPool(
//		&poolCreateInfo, nullptr, &_descriptorPool) != vk::Result::eSuccess)
//	{
//		return false;
//		//throw std::runtime_error("failed to create descriptor pool!");
//	}
//
//
//	return true;
//}
//
//bool Renderer::_CreateDescriptorSets()
//{
//	//auto const&	swapChainImages{ _device->_swapchain.images };
//
//	//std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), 
//	//	_descriptorSetLayout);
//
//	//vk::DescriptorSetAllocateInfo const	allocateInfo{
//	//_descriptorPool, (uint32_t)layouts.size(), layouts.data() };
//
//	//_descriptorSets.resize(swapChainImages.size());
//	//if (_device->_logicalDevice.allocateDescriptorSets(
//	//	&allocateInfo, &_descriptorSets[0]) != vk::Result::eSuccess)
//	//{
//	//	throw std::runtime_error("failed to allocate descriptor sets!");
//	//}
//
//	////Configuring the descriptor sets
//	//std::array<vk::WriteDescriptorSet, 2> descriptorWrites = {};
//
//	//for (size_t i = 0; i < swapChainImages.size(); i++)
//	//{
//	//	vk::DescriptorBufferInfo const	bufferInfo{ uniformBuffers[i], 0, VK_WHOLE_SIZE };
//	//	vk::DescriptorImageInfo const	imageInfo{ textureSampler, textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal };
//
//	//	descriptorWrites[0] = vk::WriteDescriptorSet{ 
//	//		_descriptorSets[i], 0, 0, 1, vk::DescriptorType::eUniformBuffer, 
//	//		nullptr, &bufferInfo, nullptr };
//
//	//	descriptorWrites[1] = vk::WriteDescriptorSet{ 
//	//		_descriptorSets[i], 1, 0, 1, 
//	//		vk::DescriptorType::eCombinedImageSampler, 
//	//		&imageInfo, nullptr, nullptr };
//
//
//
//	//	_device->_logicalDevice.updateDescriptorSets(
//	//		static_cast<uint32_t>(descriptorWrites.size()), 
//	//		descriptorWrites.data(), 0, nullptr);
//	//}
//
//	return true;
//}
//
//bool Renderer::_CreateSyncObjects()
//{
//	vk::SemaphoreCreateInfo const	createInfo{};
//	_imageAvailableSemaphores.resize(_maxFrame);
//	_renderFinishedSemaphores.resize(_maxFrame);
//	_inFlightFences.resize(_maxFrame);
//
//	vk::FenceCreateInfo const fenceCreateInfo{
//		vk::FenceCreateFlagBits::eSignaled
//	};
//
//	auto&	logicalDevice{ _device->_logicalDevice };
//
//	for (size_t i = 0; i < _maxFrame; i++)
//	{
//		if (logicalDevice.createSemaphore(
//			&createInfo, nullptr, &_imageAvailableSemaphores[i]) != vk::Result::eSuccess ||
//			
//			logicalDevice.createSemaphore(
//				&createInfo, nullptr, &_renderFinishedSemaphores[i]) != vk::Result::eSuccess ||
//			
//			logicalDevice.createFence(
//				&fenceCreateInfo, nullptr, &_inFlightFences[i]) != vk::Result::eSuccess)
//		{
//			return false;
//		}
//	}
//
//	return true;
//}
//
//vk::CommandBuffer Renderer::_BeginSingleTimeCommands(vk::CommandPool pool)
//{
//	vk::CommandBufferAllocateInfo allocInfo = { pool, vk::CommandBufferLevel::ePrimary, 1 };
//
//	vk::CommandBuffer commandBuffer;
//	_device->_logicalDevice.allocateCommandBuffers(&allocInfo, &commandBuffer);
//
//	vk::CommandBufferBeginInfo beginInfo = { vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
//
//	commandBuffer.begin(&beginInfo);
//
//	return commandBuffer;
//}
//
//void Renderer::_EndSingleTimeCommands(vk::Queue queue, vk::CommandPool pool, vk::CommandBuffer commandBuffer)
//{
//	commandBuffer.end();
//
//	vk::SubmitInfo submitInfo = {};
//	submitInfo.commandBufferCount = 1;
//	submitInfo.pCommandBuffers = &commandBuffer;
//
//	queue.submit(1, &submitInfo, {});
//	queue.waitIdle();
//
//	_device->_logicalDevice.freeCommandBuffers(pool, 1, &commandBuffer);
//}
//
//
//void Renderer::_TransitionImageLayout(Image const& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipCount, vk::Queue queue, vk::CommandPool commandPool)
//{
//	vk::CommandBuffer commandBuffer = _BeginSingleTimeCommands(commandPool);
//
//	vk::ImageSubresourceRange range{ vk::ImageAspectFlagBits::eColor, 0, mipCount, 0, 1 };
//
//	vk::AccessFlagBits	srcAccessMask{};
//	vk::AccessFlagBits	dstAccessMask{};
//
//	vk::PipelineStageFlagBits	sourceStage{};
//	vk::PipelineStageFlagBits	destinationStage{};
//
//	if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
//	{
//		range.aspectMask = vk::ImageAspectFlagBits::eDepth;
//
//		if (FormatHelper::HasStencilComponent(image.format))
//		{
//			range.aspectMask |= vk::ImageAspectFlagBits::eStencil;
//		}
//	}
//	else
//	{
//		range.aspectMask = vk::ImageAspectFlagBits::eColor;
//	}
//
//	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
//	{
//		srcAccessMask = (vk::AccessFlagBits)0;
//		dstAccessMask = vk::AccessFlagBits::eTransferWrite;
//
//		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
//		destinationStage = vk::PipelineStageFlagBits::eTransfer;
//	}
//	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
//	{
//		srcAccessMask = vk::AccessFlagBits::eTransferWrite;
//		dstAccessMask = vk::AccessFlagBits::eShaderRead;
//
//		sourceStage = vk::PipelineStageFlagBits::eTransfer;
//		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
//	}
//	else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
//	{
//		srcAccessMask = (vk::AccessFlagBits)0;
//		dstAccessMask = vk::AccessFlagBits(VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
//
//		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
//		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
//	}
//	else
//	{
//		throw std::invalid_argument("unsupported layout transition!");
//	}
//
//	//we don;t use the barrier to transfer queue family ownership, so we don't care about queue family indices
//	vk::ImageMemoryBarrier const	barrier{
//		srcAccessMask, dstAccessMask,
//		oldLayout, newLayout,
//		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
//		image.image, range };
//
//	commandBuffer.pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlagBits{}, 0, nullptr, 0, nullptr, 1, &barrier);
//
//	_EndSingleTimeCommands(queue, commandPool, commandBuffer);
//}


	//bool	AcquireImage();
	//bool	SwapImage();

	//bool	CreateDeviceBuffer(Uint32 size, void const* data, vk::BufferUsageFlagBits usage, Buffer& deviceBuffer);
	//bool	CopyBuffer(Buffer& dstBuffer, Buffer const& srcBuffer, vk::DeviceSize size);
	//bool	CreateIndexedMesh(const Uint32 indicesCount, const Uint32* indices, const Uint32 verticesCount, const Decimal32* vertices, IndexedMesh& mesh);

	//bool CreateDefaultGraphicsPipeline(PipelineShaderStages const& modules, vk::PipelineLayout pipelineLayout, vk::PipelineVertexInputStateCreateInfo const& VIInfo, vk::Pipeline & pipeline);

	//bool CreateShaderModule(Uint32 size, char const * code, vk::ShaderModule & shaderModule);
	//bool DestroyShaderModule(vk::ShaderModule & shaderModule);

	//bool	_CreateRenderPass();
	////bool	_CreateGraphicsPipeline();
	//bool	_CreateCommandBuffers();
	//bool	_CreateDepthResources();
	//bool	_CreateFramebuffers();

	//bool	_CreateDescriptorSetLayout();
	//bool	_CreateDescriptorPool();
	//bool	_CreateDescriptorSets();
	//bool	_CreateSyncObjects();

	//vk::CommandBuffer _BeginSingleTimeCommands(vk::CommandPool pool);

	//void _EndSingleTimeCommands(vk::Queue queue, vk::CommandPool pool, vk::CommandBuffer commandBuffer);

	//void _TransitionImageLayout(Image const & image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipCount, vk::Queue queue, vk::CommandPool commandPool);

	//size_t _currentFrame = 0;
	//size_t _maxFrame = 1;
	//Uint32	_currentImageIndex{};
	//std::vector<vk::Semaphore> _imageAvailableSemaphores;
	//std::vector<vk::Semaphore> _renderFinishedSemaphores;
	//std::vector<vk::Fence> _inFlightFences;

	//vk::DescriptorPool	_descriptorPool;
	//std::vector<vk::DescriptorSet> _descriptorSets;
	//vk::DescriptorSetLayout _descriptorSetLayout;

	//Image	depthImage{};
	//ImageView	depthImageView{};

	//std::vector<vk::Framebuffer>	_framebuffers{};

	////vk::Pipeline	_graphicsPipeline{};
	////vk::PipelineLayout	_pipelineLayout{};
	//vk::CommandBuffer	_commandBuffer{};
	//vk::RenderPass	_renderPass{};

