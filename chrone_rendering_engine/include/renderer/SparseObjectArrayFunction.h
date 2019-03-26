#pragma once

namespace chrone::rendering
{
/*
template <class ResourceType, class HandleType>
inline ResourceType*	ObjectHolder<ResourceType, HandleType>::AcquireResource(HandleType Handle)
{
	ResourceType*	Resource{ GetResource(Handle) };

	if (Resource)
	{
		new(Resource) ResourceType();
	}

	return Resource;
}

template <class ResourceType, class HandleType>
inline ResourceType*	ObjectHolder<ResourceType, HandleType>::GetResource(const HandleType Handle)
{
	const KeyType	index = Handle.GetValue() & IndexBitmask;
	const KeyType	generation = Handle.GetValue() & GenerationBitmask;


	if (index < Resources.size() &&
		generation == Generations[static_cast<IntegralVector::size_type>(index)])
	{
		return &Resources[static_cast<IntegralVector::size_type>(index)];
	}

	return nullptr;
}

template <class ResourceType, class HandleType>
inline const ResourceType*	ObjectHolder<ResourceType, HandleType>::GetResource(const HandleType Handle) const
{
	const KeyType	index = Handle.GetValue() & IndexBitmask;
	const KeyType	generation = Handle.GetValue() & GenerationBitmask;


	if (index < Resources.size() &&
		generation == Generations[static_cast<IntegralVector::size_type>(index)])
	{
		return &Resources[static_cast<IntegralVector::size_type>(index)];
	}

	return nullptr;
}

template <class ResourceType, class HandleType>
inline void	ObjectHolder<ResourceType, HandleType>::ReleaseResource(HandleType& Handle)
{
	const KeyType	index = Handle.GetValue() & IndexBitmask;
	const KeyType	generation = Handle.GetValue() & GenerationBitmask;

	if (index < Resources.size() &&
		generation == Generations[static_cast<IntegralVector::size_type>(index)])
	{
		freeIndices.push_back(index);
		Generations[index] = (Generations[index] + GenerationInc) & GenerationBitmask;
		Resources[index].~ResourceType();
	}

	Handle = InvalidHandle;
}

template <class ResourceType, class HandleType>
inline bool ObjectHolder<ResourceType, HandleType>::FastValidityCheck(const HandleType Handle) const
{
	return Handle != InvalidHandle;
}

template <class ResourceType, class HandleType>
inline void ObjectHolder<ResourceType, HandleType>::Reset()
{
	Resources.clear();
	Generations.clear();
	freeIndices.clear();
}

template<class ResourceType, class HandleType>
inline HandleType ObjectHolder<ResourceType, HandleType>::GenerateHandle()
{
	HandleType	handle{};
	KeyType	index = KeyType();

	if (!freeIndices.empty())
	{
		index = freeIndices.back();
		freeIndices.pop_back();
	}
	else
	{
		index = static_cast<KeyType>(Resources.size());

		if ((index + 1) >= IndexBitmask)
		{
			return InvalidHandle;
		}

		Resources.resize(Resources.size() + 1u);
		Generations.push_back(static_cast<KeyType>(0));
	}


	handle = static_cast<KeyType>((0xFF000000 & Generations[index]) | (IndexBitmask & index));

	return handle;
}*/
}