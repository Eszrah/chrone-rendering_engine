#include "renderer/CommandBufferInterface.h"

#include <vulkan\vulkan.hpp>

#include "renderer/CommandBuffer.h"
#include "renderer/RendererResourceManager.h"
#include "vulkan_backend/Device.h"


namespace chrone::rendering
{



void 
CommandBufferInterface::Draw(CommandBufferInterface& interface, void * data)
{
	auto*	realData{ static_cast<RenderCommand<DrawCommand>*>(data) };

	interface.graphicsCommandBuffer->draw(realData->vertexCount, realData->instanceCount, realData->firstVertex, realData->firstInstance);
}


void 
CommandBufferInterface::DrawIndexed(CommandBufferInterface& interface, void * data)
{
	auto*	realData{ static_cast<RenderCommand<DrawIndexedCommand>*>(data) };

	vk::DeviceSize	offset{ 0u };
	//Buffer vertexBuffer{};//interface.resourceManager->GetResource();
	//Buffer indexBuffer{};//interface.resourceManager->GetResource();
	//interface.graphicsCommandBuffer->bindVertexBuffers(0, 1, &vertexBuffer.buffer, &offset);
	//interface.graphicsCommandBuffer->bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint32);
	//interface.graphicsCommandBuffer->drawIndexed(realData->indexCount, realData->instanceCount, realData->firstIndex, realData->vertexOffset, realData->firstInstance);
}


void 
CommandBufferInterface::BindGraphicsPipeline(CommandBufferInterface& interface, void * data)
{
	auto*	realData{ static_cast<RenderCommand<BindGraphicsPipelineCommand>*>(data) };
	GraphicsPipeline* graphicsPipeline{ interface.resourceManager->GetResource(realData->pipeline) };
	interface.graphicsCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline->pipeline);
}

void CommandBufferInterface::BindVertexBuffer(CommandBufferInterface& interface, void * data)
{
	auto*	realData{ static_cast<RenderCommand<BindVertexBufferCommand>*>(data) };

	std::vector<vk::Buffer>	buffers(realData->bindingCount);
	
	for (auto index = 0u; index < realData->bindingCount; ++index)
	{
		buffers[index] = interface.resourceManager->GetResource(realData->pBuffers[index])->buffer;
	}

	interface.graphicsCommandBuffer->bindVertexBuffers(realData->firstBinding, realData->bindingCount, buffers.data(), realData->pOffsets);
}


void
CommandBufferInterface::BeginRenderPass(CommandBufferInterface& interface,
	void* data)
{
	auto*	realData{ static_cast<RenderCommand<BeginRenderPassCommand>*>(data) };
	
	Framebuffer* framebuffer{ interface.resourceManager->GetResource(realData->framebuffer) };//;
	RenderPass* renderPass{ interface.resourceManager->GetResource(realData->renderPass) };//;


	vk::RenderPassBeginInfo const	beginInfo{ renderPass->renderpass, 
		framebuffer->framebuffer, realData->renderArea, 
		realData->clearValueCount, realData->clearValues };
	
	vk::CommandBufferBeginInfo const	recordBeginInfo{
		vk::CommandBufferUsageFlagBits::eSimultaneousUse, nullptr };

	interface.graphicsCommandBuffer->begin(&recordBeginInfo);
	interface.graphicsCommandBuffer->beginRenderPass(beginInfo, vk::SubpassContents::eInline);
}


void 
CommandBufferInterface::EndRenderPass(CommandBufferInterface& interface,
	void* data)
{
	vk::SubmitInfo const	submitInfo{ 0, nullptr, nullptr, 1, interface.graphicsCommandBuffer};

	interface.graphicsCommandBuffer->endRenderPass();
	interface.graphicsCommandBuffer->end();
	interface.graphicsQueue->submit(1, &submitInfo, {});
	interface.graphicsQueue->waitIdle();
}

void CommandBufferInterface::MapBuffer(CommandBufferInterface & instance, void * data)
{
	auto*	realData{ static_cast<RenderCommand<MapBufferCmd>*>(data) };
	Buffer* mappedBuffer{ instance.resourceManager->GetResource(realData->buffer) };

	void*	srcData{ *realData->ppData };
	void*	mappedData{};
	instance.device->logicalDevice.mapMemory(mappedBuffer->memory, realData->offset, realData->size, realData->flags, &mappedData);
	memcpy(mappedData, srcData, realData->size);
	instance.device->logicalDevice.unmapMemory(mappedBuffer->memory);
}

void CommandBufferInterface::CopyBuffer(CommandBufferInterface & instance, void * data)
{
	auto*	realData{ static_cast<RenderCommand<CopyBufferCmd>*>(data) };

	vk::CommandPool	transferPool{ instance.device->transferCommandPool };
	vk::CommandBuffer transferCommandBuffer{ instance.device->BeginSingleTimeCommands(transferPool) };

	Buffer* srcBuffer{ instance.resourceManager->GetResource(realData->srcBuffer) };
	Buffer* dstBuffer{ instance.resourceManager->GetResource(realData->dstBuffer) };


	transferCommandBuffer.copyBuffer(srcBuffer->buffer, dstBuffer->buffer, realData->regionCount, realData->pRegions);
	instance.device->EndSingleTimeCommands(instance.device->transferQueue, transferPool, transferCommandBuffer);
}


void CommandBufferInterface::CreateBuffer(CommandBufferInterface& interface, void * data)
{
	auto*	realData{ static_cast<ResourceCommand<CreateBufferCommand>*>(data) };
	interface.resourceManager->CreateResource(*realData);
}


void CommandBufferInterface::CreateImageView(CommandBufferInterface& interface, void * data)
{
	auto*	realData{ static_cast<ResourceCommand<CreateImageViewCommand>*>(data) };
	interface.resourceManager->CreateResource(*realData);
}

void CommandBufferInterface::CreateRenderPass(CommandBufferInterface& interface, void * data)
{
	auto*	realData{ static_cast<ResourceCommand<CreateRenderPassCommand>*>(data) };
	interface.resourceManager->CreateResource(*realData);
}

void CommandBufferInterface::CreateFramebuffer(CommandBufferInterface& interface, void * data)
{
	auto*	realData{ static_cast<ResourceCommand<CreateFramebufferCommand>*>(data) };
	interface.resourceManager->CreateResource(*realData);
}

void CommandBufferInterface::CreateGraphicsPipeline(CommandBufferInterface& interface, void * data)
{
	auto*	realData{ static_cast<ResourceCommand<CreateGraphicsPipelineCommand>*>(data) };
	interface.resourceManager->CreateResource(*realData);
}

}