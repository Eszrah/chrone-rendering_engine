#pragma once

#include "NativeType.h"

namespace chrone::rendering
{

struct CommandBuffer;

struct ResourceCommandBuffer;
struct CmdBufferCreateInfo;

struct CommandBufferFunc
{

	static void
		CmdCreateBuffer(ResourceCommandBuffer& cmdBuffer,
			CmdBufferCreateInfo const& cmd);

};

}