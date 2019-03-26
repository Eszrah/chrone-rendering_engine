#pragma once

#include <vulkan\vulkan.hpp>

namespace chrone::rendering
{

struct FormatHelper 
{
	static vk::Format 
	FindSupportedFormat(vk::PhysicalDevice physicalDevice,
		const std::vector<vk::Format>& candidates, 
		vk::ImageTiling tiling, 
		vk::FormatFeatureFlags features)
	{
		for (vk::Format format : candidates)
		{
			//checking supported features
			vk::FormatProperties const properties{ 
				physicalDevice.getFormatProperties(format) };

			if (tiling == vk::ImageTiling::eLinear && 
				(properties.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == vk::ImageTiling::eOptimal && 
				(properties.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		return vk::Format::eUndefined;
	}


	static vk::Format 
		FindDepthFormat(vk::PhysicalDevice physicalDevice)
	{
		return FindSupportedFormat(physicalDevice,
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, 
			vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}


	static bool 
	HasStencilComponent(vk::Format format)
	{
		return format == vk::Format::eD32SfloatS8Uint || 
			format == vk::Format::eD24UnormS8Uint;
	}
};


struct ShaderModuleHelper 
{
	static bool	
	CreateShaderModule(
		vk::Device device, 
		Uint32 size, char const* code, 
		vk::ShaderModule& shaderModule)
	{
		vk::ShaderModuleCreateInfo	createInfo {
			{}, size, reinterpret_cast<const uint32_t*>(code) };

		if (device.createShaderModule(&createInfo, nullptr, &shaderModule) 
			!= vk::Result::eSuccess)
		{
			return false;
		}

		return true;
	}
};

struct ImageHelper 
{
	static vk::ImageCreateInfo
		CreateImage2DSampledConcurrentCreateInfo(vk::Extent2D extent,
			Uint32 mipCount,
			vk::Format format,
			vk::SharingMode sharingMode = vk::SharingMode::eConcurrent,
			Uint32 queueFamilyIndexCount = 0,
			const Uint32* pQueueFamilyIndices = nullptr)
	{
		return  vk::ImageCreateInfo{ {}, vk::ImageType::e2D,
			format, vk::Extent3D{ extent.width, extent.height, 1 }, mipCount, 
			1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | 
				vk::ImageUsageFlagBits::eSampled,  
			sharingMode, queueFamilyIndexCount, pQueueFamilyIndices, 
			vk::ImageLayout::eUndefined };

	}

};

struct ImageViewHelper 
{
	static
		vk::ImageViewCreateInfo
		CreateDefaultImageViewCreateInfo(
			Image const& image, vk::ImageAspectFlags aspectFlags,
			vk::ImageViewType viewType = vk::ImageViewType::e2D)
	{
		vk::ImageSubresourceRange const range{ aspectFlags, 0, image.mipLevels, 
			0, image.arrayLayers };

		return vk::ImageViewCreateInfo{ {}, image.image, viewType, 
			image.format, vk::ComponentMapping{}, range };
	}
};

struct MemoryPropertyHelper
{
	inline static auto const	eHostCoherentVisible{
		vk::MemoryPropertyFlagBits::eHostCoherent | 
		vk::MemoryPropertyFlagBits::eHostVisible };
};

struct BufferUsageHelper 
{
	inline static auto const	eTransferDstVertexBuffer{
	vk::BufferUsageFlagBits::eVertexBuffer |
	vk::BufferUsageFlagBits::eTransferDst };
};

struct BufferHelper 
{
	static vk::BufferCreateInfo
		CreateExclusiveBufferCreateInfo(
			vk::DeviceSize const size, 
			vk::BufferUsageFlags const usage)
	{
		return { {}, size, usage };
	}

	static vk::BufferCreateInfo
		CreateTransferDstExclusiveBufferCreateInfo(
			vk::DeviceSize const size)
	{
		return { {}, size, vk::BufferUsageFlagBits::eTransferDst };
	}


};

struct MemoryHelper
{
	static Uint32 
		FindMemoryType(
			vk::PhysicalDevice physicalDevice, 
			Uint32 typeFilter, 
			vk::MemoryPropertyFlags properties)
	{
		vk::PhysicalDeviceMemoryProperties const	memProperties{ 
			physicalDevice.getMemoryProperties() };

		for (Uint32 i = 0; i < memProperties.memoryTypeCount; i++) 
		{
			if ((typeFilter & (1 << i)) && 
				(memProperties.memoryTypes[i].propertyFlags & properties) == 
				properties) 
			{
				return i;
			}
		}

		return std::numeric_limits<Uint32>::max();
	}
};

struct ViewportHelper 
{
	inline static vk::Viewport const
		CreateViewportCreateInfo(Decimal32 width, Decimal32 height,
			Decimal32 xOffset = 0.f, Decimal32 yOffset = 0.f,
			Decimal32 minDepth = 0.f, Decimal32 maxDepth = 0.f)
	{
		return { xOffset, yOffset, width, height, minDepth, maxDepth };
	}

};

struct InputAssemblyStateHelper 
{
	inline static vk::PipelineInputAssemblyStateCreateInfo const	
		defaultInputState { 
		{}, vk::PrimitiveTopology::eTriangleList, false };
};

struct RasterizerHelper 
{
	inline static vk::PipelineRasterizationStateCreateInfo const	
		defaultRasterizer {
		{}, false, false, vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
		false, 0.0f, 0.0f, 0.0f, 1.0f };
};

struct PipelineMultiSampleHelper
{
	inline static vk::PipelineMultisampleStateCreateInfo const 
		defaultPipelineMS{
	{}, vk::SampleCountFlagBits::e1, false, 1.0f };
};


struct PipelineColorBlendAttachmentHelper
{
	inline static vk::ColorComponentFlags const	eRGBAMask{
	vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
	vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };

	inline static vk::PipelineColorBlendAttachmentState const	
		defaultColorBlend
	{ 0, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
		vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
		vk::BlendOp::eAdd, eRGBAMask };
};

struct PipelineDepthStencilStateHelper 
{
	inline static vk::PipelineDepthStencilStateCreateInfo const
		defaultDepthStencilState{ {},
		true, true, vk::CompareOp::eLess, false, false, {}, {}, 0.0f, 1.0f };
};


struct DescriptorSetHelper
{
	static constexpr Uint32 DescriptorTypeCount{ 
		VkDescriptorType::VK_DESCRIPTOR_TYPE_RANGE_SIZE };

};

}