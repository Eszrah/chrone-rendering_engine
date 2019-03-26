#pragma once

#include <vulkan\vulkan.hpp>

namespace chrone::rendering
{

struct DeviceInstance 
{
	vk::PhysicalDevice physicalDevice;
	vk::Device logicalDevice;
};

}