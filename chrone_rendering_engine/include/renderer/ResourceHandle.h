#pragma once

#include <limits>

#include "NativeType.h"
#include "renderer/Handle.h"

namespace chrone::rendering
{

using HBuffer = HandleUI32<struct HBufferType>;
using HImageView = HandleUI32<struct HImageViewType>;
using HGraphicsPipeline = HandleUI32<struct HGraphicsPipelineType>;
using HComputePipeline = HandleUI32<struct HComputePipelineType>;
using HRenderPass = HandleUI32<struct HRenderPassType>;
using HFramebuffer = HandleUI32<struct HFramebufferType>;
using HDescriptorSetLayout = HandleUI32<struct HDescriptorSetLayoutType>;
using HDescriptorAllocator = HandleUI32<struct HDescriptorAllocatorType>;
using HDescriptorSet = HandleUI32<struct HDescriptorSetType>;

using HFence = HandleUI32<struct HFenceType>;
using HSemaphore = HandleUI32<struct HSemaphoreType>;


using HResourceCommandBuffer = HandleUI32<struct HResourceCommandBufferType>;
using HCommandBuffer = HandleUI32<struct HCommandBufferType>;

}