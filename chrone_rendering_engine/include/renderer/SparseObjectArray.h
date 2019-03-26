#pragma once

namespace chrone::rendering
{

template <class ResourceType, class HandleType>
struct SparseObjectArray
{
	using ResourceVector = std::vector<ResourceType>;
	using KeyType = Uint32;
	using IntegralVector = std::vector<KeyType>;

	static const KeyType InvalidHandle = 0xFFFFFFFF;
	static const KeyType GenerationBitmask = 0xFF000000;
	static const KeyType GenerationInc = 0x01000000;
	static const KeyType IndexBitmask = 0x00FFFFFF;

	ResourceVector	resources;
	IntegralVector	generations;
	IntegralVector	freeIndices;
};

}