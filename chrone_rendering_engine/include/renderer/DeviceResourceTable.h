#pragma once

#include "DeviceResource.h"
#include "SparseObjectArray.h"
#include "chrone-memory/include/LinearMemoryMapper.h"

namespace chrone::rendering
{

struct DeviceResourceTable 
{
	SparseObjectArray<Buffer, HBuffer>	buffers{};
	
	SparseObjectArray<Image, HImage>	images{};
	std::vector<Uint32>	imageReferenceViewCount{};
	SparseObjectArray<ImageView, HImageView>	imageViews{};
	
	SparseObjectArray<GraphicsPipeline, HGraphicsPipeline>	graphicsPipeline{};
	SparseObjectArray<ComputePipeline, HComputePipeline>	computePipeline{};
	SparseObjectArray<Framebuffer, HFramebuffer>	frameBuffers{};
	SparseObjectArray<RenderPass, HRenderPass>	renderPasses{};
	SparseObjectArray<Fence, HFence>	fences{};
	SparseObjectArray<Semaphore, HSemaphore>	semaphores{};

	chrone::memory::StaticLinearMemoryMapper	memoryMapper;
};

}