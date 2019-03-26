#include "renderer/CommandBufferFunc.h"

#include "renderer/CommandBuffer.h"
#include "renderer/RenderCommand.h"
#include "renderer/ResourceHandle.h"

#include "chrone-memory/include/LinearMemoryMapperFunc.h"

using namespace chrone::memory;

namespace chrone::rendering
{

void 
CommandBufferFunc::CmdCreateBuffer(
	ResourceCommandBuffer& cmdBuffer,
	CmdBufferCreateInfo const& cmd)
{
	CommandBufferMemoryMapper&	memoryMapper{ cmdBuffer.memoryMapper };
	const Uint32	queueFamilyIndexCountSize{ 
		cmd.createInfo.queueFamilyIndexCount * sizeof(uint32_t) };
	const Uint32	iAllocationSize{ sizeof(CmdBufferCreateInfo) + 
	queueFamilyIndexCountSize + sizeof(HBuffer)};

	Char*	memory{ 
		LinearMemoryMapperFunc::MapMemory(memoryMapper, iAllocationSize) };

	CommandHeader*	header{ reinterpret_cast<CommandHeader*>(memory) };
	memory += sizeof(CommandHeader);
	
	HBuffer*	hBuffer{ reinterpret_cast<HBuffer*>(memory) };
	memory += sizeof(HBuffer);
	
	CmdBufferCreateInfo*	cmdMem{ 
		reinterpret_cast<CmdBufferCreateInfo*>(memory) };

	memcpy(cmdMem, &cmd, sizeof(CmdBufferCreateInfo));
	memcpy(&cmdMem->createInfo.queueFamilyIndexCount, 
		&cmd.createInfo.queueFamilyIndexCount, queueFamilyIndexCountSize);
}

}