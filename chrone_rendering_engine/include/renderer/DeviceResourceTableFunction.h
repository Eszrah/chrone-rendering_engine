#pragma once

#include <vulkan\vulkan.hpp>
#include "ResourceHandle.h"

namespace chrone::rendering
{

struct DeviceResourceTable;
struct DeviceInstance;

struct CmdBufferCreateInfo;
struct CmdImageCreateInfo;
struct CmdRenderPassCreateInfo;

struct DeviceResourceTableFunc
{
	vk::Result	AllocateBuffer(DeviceResourceTable& resourceTable, const DeviceInstance& device, const HBuffer hBuffer, const CmdBufferCreateInfo& cmd);
	//vk::Result	DeallocateBuffer(DeviceResourceTable& resourceTable, const DeviceInstance& device, const HBuffer hBuffer);
	vk::Result	AllocateImageView(DeviceResourceTable& resourceTable, const DeviceInstance& device, const HImageView hImageView, const CmdImageCreateInfo& cmd);
	//vk::Result	DeallocateImageView(DeviceResourceTable& resourceTable, const DeviceInstance& device, const HImageView hImageView);

	vk::Result	AllocateRenderPass(DeviceResourceTable& resourceTable, const DeviceInstance& device, const HRenderPass hRenderPass, const CmdRenderPassCreateInfo& cmd);

};


}