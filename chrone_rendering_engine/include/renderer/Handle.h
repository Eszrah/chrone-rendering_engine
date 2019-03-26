#pragma once

#include "NativeType.h"

namespace chrone::rendering
{

template<class HandleType>
struct HandleUI32
{
	HandleUI32() = default;
	HandleUI32(HandleUI32 const&) = default;
	HandleUI32(HandleUI32&&) = default;
	~HandleUI32() = default;

	HandleUI32(Uint32 value):
		value{ value }
	{}

	HandleUI32&	operator=(HandleUI32 const&) = default;
	HandleUI32&	operator=(HandleUI32&&) = default;

	Uint32 GetValue() const { return value; }

	Uint32	value{ 0u };
};

}