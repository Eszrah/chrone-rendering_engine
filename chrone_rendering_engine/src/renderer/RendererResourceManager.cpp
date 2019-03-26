#include "renderer/RendererResourceManager.h"

#include <algorithm>

#include "renderer/RendererResource.h"
#include "renderer/ResourceCommand.h"
#include "vulkan_backend/Device.h"
#include "vulkan_backend/VulkanHelper.h"


namespace chrone::rendering
{


bool RendererResourceManager::CreateResource(ResourceCommand<CreateBufferCommand> const& createInfo)
{
	Buffer* buffer{ GetResource(createInfo.buffer) };

	if (_device->logicalDevice.createBuffer(
		&createInfo.createInfo, nullptr, &buffer->buffer) != 
		vk::Result::eSuccess)
	{
		return false;
	}

	vk::MemoryRequirements const	memRequirement{ 
		_device->logicalDevice.getBufferMemoryRequirements(buffer->buffer) };

	vk::MemoryAllocateInfo const	allocateInfo{ memRequirement.size,
		MemoryHelper::FindMemoryType( _device->physicalDevice, 
			memRequirement.memoryTypeBits, createInfo.memoryProperties) };

	if (_device->logicalDevice.allocateMemory(&allocateInfo, nullptr, &buffer->memory) != 
		vk::Result::eSuccess)
	{
		return false;
	}

	_device->logicalDevice.bindBufferMemory(buffer->buffer, buffer->memory, 0);
	return true;
}

bool RendererResourceManager::CreateResource(ResourceCommand<CreateImageViewCommand> const& createInfo)
{
	HImage	srcHImage{};
	Image* srcImage{ nullptr };
	auto const&	viewProperties{ createInfo.viewProperties };


	if (createInfo.creationType == 
		ResourceCommand<CreateImageViewCommand>::CreationType::Image)
	{
		auto const&	imageCreation{ createInfo.imageCreation };
		auto const&	imageCreateInfo{ imageCreation.createInfo };
		
		srcHImage = GenerateHandle<HImage>();
		srcImage = GetResource(srcHImage);

		if (_device->logicalDevice.createImage(
			&imageCreateInfo, nullptr, &srcImage->image) !=
			vk::Result::eSuccess)
		{
			return false;
		}

		vk::MemoryRequirements	memRequirements{
		_device->logicalDevice.getImageMemoryRequirements(srcImage->image) };
			
		vk::MemoryAllocateInfo const	allocateInfo{ memRequirements.size,
			MemoryHelper::FindMemoryType(_device->physicalDevice, 
				memRequirements.memoryTypeBits, 
				imageCreation.memoryProperties) };
		
		if (_device->logicalDevice.allocateMemory(
			&allocateInfo, nullptr, &srcImage->memory) !=
			vk::Result::eSuccess)
		{
			//destroy image and release the handle
			return false;
		}
		
		const auto& imageCI{ createInfo.imageCreation.createInfo };
		srcImage->arrayLayers = imageCI.arrayLayers;
		srcImage->extent = imageCI.extent;
		srcImage->format = imageCI.format;
		srcImage->imageType = imageCI.imageType;
		srcImage->mipLevels = imageCI.mipLevels;

		_device->logicalDevice.bindImageMemory(srcImage->image, srcImage->memory, { 0 });
		_imagesDependantViewCount[srcHImage] = 1u;
	}
	else if (createInfo.creationType ==
		ResourceCommand<CreateImageViewCommand>::CreationType::View)
	{
		ImageView const*	srcImageView{ GetResource(
			createInfo.viewCreation.srcImageView)};
		srcHImage = srcImageView->hImage;
		srcImage = GetResource(srcHImage);
		_imagesDependantViewCount[srcHImage] += 1;
	}


	vk::ImageViewCreateInfo const	viewCreateInfo{ {},
	srcImage->image, viewProperties.viewType, viewProperties.format,
	viewProperties.components, viewProperties.subresourceRange };

	ImageView* imageView{ GetResource(createInfo.imageView) };

	if (_device->logicalDevice.createImageView(
		&viewCreateInfo, nullptr, &imageView->view) !=
		vk::Result::eSuccess)
	{
		return false;
	}
	
	imageView->hImage = srcHImage;
	imageView->properties.viewType = viewProperties.viewType;
	imageView->properties.format = viewProperties.format;
	imageView->properties.components = viewProperties.components;
	imageView->properties.subresourceRange = viewProperties.subresourceRange;

	vk::ImageSubresourceRange const range{ vk::ImageAspectFlagBits::eColor, 0, 
		viewProperties.subresourceRange.levelCount, 0, 
		viewProperties.subresourceRange.layerCount };

	//general layout transition
	vk::ImageMemoryBarrier const	imageMemoryBarrier{
		(vk::AccessFlagBits)0, 
		vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral, 
		VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, 
		srcImage->image, range };

	vk::CommandPool	transferPool{ _device->transferCommandPool };

	vk::CommandBuffer transferCommandBuffer{ _device->BeginSingleTimeCommands(transferPool) };
	transferCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits{}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	_device->EndSingleTimeCommands(_device->transferQueue, transferPool, transferCommandBuffer);

	return true;
}


bool RendererResourceManager::CreateResource(ResourceCommand<CreateFramebufferCommand> const& createInfo)
{
	Framebuffer* framebuffer{ GetResource(createInfo.framebuffer) };
	RenderPass* renderPass{ GetResource(createInfo.renderPass) };
	
	std::vector<vk::ImageView>	attachments{};
	attachments.reserve(createInfo.attachmentCount);

	for (Uint32 index{ 0u }; index < createInfo.attachmentCount; ++index)
	{
		attachments.emplace_back(
			GetResource(createInfo.attachments[index])->view);
	}

	vk::FramebufferCreateInfo const	fbCreateInfo{ {}, renderPass->renderpass, 
		createInfo.attachmentCount, attachments.data(), 
		createInfo.width, createInfo.height, createInfo.layers };

	if (_device->logicalDevice.createFramebuffer(
		&fbCreateInfo, nullptr, &framebuffer->framebuffer) !=
		vk::Result::eSuccess)
	{
		return false;
	}

	return true;
}


bool RendererResourceManager::CreateResource(ResourceCommand<CreateRenderPassCommand> const& createInfo)
{
	RenderPass* renderPass{ GetResource(createInfo.renderPass) };

	if (_device->logicalDevice.createRenderPass(
		&createInfo.createInfo, nullptr, &renderPass->renderpass) !=
		vk::Result::eSuccess)
	{
		return false;
	}

	return true;
}


bool RendererResourceManager::CreateResource(ResourceCommand<CreateGraphicsPipelineCommand> const& createInfo)
{
	GraphicsPipeline* graphicsPipeline{ 
		GetResource(createInfo.graphicsPipeline) };
	
	RenderPass const* renderPass{ 
		GetResource(createInfo.renderPass) };

	//Construct the pipeline layout based on provided descriptor set layouts
	Uint32 const	descriptorSetLayoutCount{ 
		createInfo.descriptorSetLayoutCount };

	HDescriptorSetLayout const*	pDescriptorSetLayouts{ 
		createInfo.pDescriptorSetLayouts };

	std::vector<vk::DescriptorSetLayout>	descriptorSetLayouts(
		descriptorSetLayoutCount);

	for (Uint32 index{ 0u }; index < descriptorSetLayoutCount; ++index)
	{
		descriptorSetLayouts[index] = GetResource(
			pDescriptorSetLayouts[index])->layout;
	};

	vk::PipelineLayoutCreateInfo const	pipelineLayoutCreateInfo{
	{}, static_cast<Uint32>(descriptorSetLayouts.size()), 
	descriptorSetLayouts.data(), 0u, nullptr };


	if (_device->logicalDevice.createPipelineLayout(
		&pipelineLayoutCreateInfo, nullptr, &graphicsPipeline->layout) != 
			vk::Result::eSuccess)
	{
		return false;
	}

	std::vector<vk::ShaderModule>	shaderStagesModule(createInfo.stageCount);
	std::vector<vk::PipelineShaderStageCreateInfo>	shaderStagesCreateInfo(
		createInfo.stageCount);

	for (auto index{ 0u }; index < createInfo.stageCount; ++index)
	{
		auto const&	currentStage{ createInfo.pStages[index] };

		//creating the module
		vk::ShaderModuleCreateInfo const	createInfo{ 
			{}, currentStage.byteSize, currentStage.code };

		//I should check if it succeeded
		_device->logicalDevice.createShaderModule(
			&createInfo, nullptr, &shaderStagesModule[index]);
		
		shaderStagesCreateInfo[index] = vk::PipelineShaderStageCreateInfo{ 
			{}, currentStage.stage, shaderStagesModule[index], currentStage.entryPointName.c_str() };
	}



	vk::GraphicsPipelineCreateInfo const	pipelineInfo {
		{}, static_cast<Uint32>(shaderStagesCreateInfo.size()), 
		shaderStagesCreateInfo.data(), &createInfo.pVertexInputState, 
		&createInfo.pInputAssemblyState, &createInfo.pTessellationState, 
		&createInfo.pViewportState, &createInfo.pRasterizationState,
		&createInfo.pMultisampleState, &createInfo.pDepthStencilState, 
		&createInfo.pColorBlendState, createInfo.pDynamicState, 
		graphicsPipeline->layout, renderPass->renderpass, createInfo.subpass,
		vk::Pipeline{}, -1 };


	if (_device->logicalDevice.createGraphicsPipelines(
		{}, 1, &pipelineInfo, nullptr, &graphicsPipeline->pipeline) 
			!= vk::Result::eSuccess)
	{
		return false;
	}

	//destroying the module
	for (auto index{ 0u }; index < createInfo.stageCount; ++index)
	{
		_device->logicalDevice.destroyShaderModule(shaderStagesModule[index]);
	}

	return true;
}

bool RendererResourceManager::CreateResource(ResourceCommand<CreateDescriptorSetLayoutCommand> const& createInfo)
{
	DescriptorSetLayout*	descriptorSetLayout{
		GetResource(createInfo.hDescriptorSetLayout) };

	std::vector<vk::DescriptorPoolSize>&	typesInfo{ 
		descriptorSetLayout->typesInfo };

	vk::DescriptorSetLayoutCreateInfo const& descriptorSetLayoutCI{ 
		createInfo.createInfo };

	vk::DescriptorSetLayoutBinding const*	bindings{ 
		descriptorSetLayoutCI.pBindings };

	typesInfo.reserve(DescriptorSetHelper::DescriptorTypeCount);

	//We want to find how many bindings there are for each type
	for (Uint32 typeIndex{ 0u }; 
		typeIndex < DescriptorSetHelper::DescriptorTypeCount; ++typeIndex)
	{
		Uint32	typeCount{ 0u };

		for (auto bindingIndex{ 0u };
			bindingIndex < descriptorSetLayoutCI.bindingCount; ++bindingIndex)
		{
			if (static_cast<Uint32>(bindings[bindingIndex].descriptorType) ==
				typeIndex)
			{
				++typeCount;
			}
		}

		if (typeCount != 0u)
		{
			typesInfo.emplace_back(static_cast<vk::DescriptorType>(typeIndex),
				typeCount);
		}
	}

	if (_device->logicalDevice.createDescriptorSetLayout(&descriptorSetLayoutCI,
		nullptr, &descriptorSetLayout->layout) != vk::Result::eSuccess)
	{
		return false;
	}

	return true;
}

bool RendererResourceManager::CreateResource(ResourceCommand<CreateDescriptorAllocatorCommand> const& createInfo)
{
	DescriptorAllocator*	descriptorAllocator{ 
		GetResource(createInfo.hDescriptorAllocator) };

	DescriptorSetLayout const*	descriptorSetLayout{
		GetResource(createInfo.hDescriptorSetLayout) };

	vk::DescriptorPoolCreateInfo const	descriptorPoolCreateInfo{
	vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 
	createInfo.poolChunkCount, 
	static_cast<Uint32>(descriptorSetLayout->typesInfo.size()), 
	descriptorSetLayout->typesInfo.data() };

	descriptorAllocator->pools.resize(1u);
	descriptorAllocator->hDescriptorSetLayout = createInfo.hDescriptorSetLayout;

	DescriptorAllocator::Pool&	descriptorAllocatorPool{ 
		descriptorAllocator->pools[0u] };

	descriptorAllocator->maxSetsCount = createInfo.poolChunkCount;
	descriptorAllocatorPool.freeSetsCount = createInfo.poolChunkCount;

	if (_device->logicalDevice.createDescriptorPool(
		&descriptorPoolCreateInfo, nullptr, 
		&descriptorAllocatorPool.descriptorPool) != 
			vk::Result::eSuccess)
	{
		return false;
	}

	return true;
}

bool RendererResourceManager::CreateResource(ResourceCommand<AllocateDescriptorSetCommand> const& createInfo)
{
	DescriptorAllocator*	descriptorAllocator{
		GetResource(createInfo.hDescriptorAllocator) };

	DescriptorAllocator::PoolVector&	poolVector{ 
		descriptorAllocator->pools };

	DescriptorSet*	descriptorSet{
		GetResource(createInfo.hDescriptorSet) };

	auto poolIt{ std::find_if(poolVector.begin(), poolVector.end(),
		[](const DescriptorAllocator::Pool& pool)
		{ return pool.freeSetsCount != 0u; }) };


	DescriptorSetLayout const*	descriptorSetLayout{
		GetResource(
			descriptorAllocator->hDescriptorSetLayout) };

	if (poolIt == poolVector.end())
	{
		//we have to create a new descriptor pool
		poolVector.emplace_back(/*descriptorAllocator->maxSetsCount, {}*/);
		poolIt = --poolVector.end();

		vk::DescriptorPoolCreateInfo const	descriptorPoolCreateInfo{
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			poolIt->freeSetsCount, 
			static_cast<Uint32>(descriptorSetLayout->typesInfo.size()),
			descriptorSetLayout->typesInfo.data() };

		if (_device->logicalDevice.createDescriptorPool(
			&descriptorPoolCreateInfo, nullptr,
			&poolIt->descriptorPool) !=
			vk::Result::eSuccess)
		{
			return false;
		}
	}

	descriptorSet->hDescriptorAllocator = createInfo.hDescriptorAllocator;

	vk::DescriptorSetAllocateInfo const	allocateInfo{
		poolIt->descriptorPool, 1, &descriptorSetLayout->layout };

	if (_device->logicalDevice.allocateDescriptorSets(
		&allocateInfo, &descriptorSet->descriptorSet) != vk::Result::eSuccess)
	{
		return false;
	}

	return true;
}

}