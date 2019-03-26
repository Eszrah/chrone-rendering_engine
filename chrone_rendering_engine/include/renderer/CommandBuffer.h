#pragma once

#include "NativeType.h"
#include "chrone-memory/include/LinearMemoryMapper.h"

namespace chrone::rendering
{

using CommandBufferMemoryMapper = chrone::memory::DynamicLinearMemoryMapper;

struct CommandHeader
{
	using CommandFunctor = void(*)(void*);
	CommandFunctor	functor{ nullptr };
};


struct CommandBuffer
{
	Uint8*	data{ nullptr };
	CommandBufferMemoryMapper	memoryMapper;
};


struct ResourceCommandBuffer
{
	Uint8*	data{ nullptr };
	CommandBufferMemoryMapper	memoryMapper;
};

}