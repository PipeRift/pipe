// Copyright 2015-2024 Piperift - All rights reserved

#include "PipeReflect.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Memory/HeapArena.h"
#include "Pipe/Memory/MultiLinearArena.h"
#include "PipeECS.h"


namespace p
{
	void Read(Reader& r, TypeId& val)
	{
		u64 idValue = TypeId::None().GetId();
		r.Serialize(idValue);
		val = TypeId{idValue};
	}

	void Write(Writer& w, TypeId val)
	{
		w.Serialize(val.GetId());
	}


	struct TypeRegistry
	{
		MultiLinearArena arena;

		TArray<TypeId> ids{arena};

		TArray<TypeId> parentIds{arena};
		TArray<sizet> sizes{arena};
		TArray<StringView> names{arena};
		TArray<u64> flags{arena};
		TArray<TArray<TypeProperty>> ownProperties{arena};
		TArray<TArray<const TypeProperty*>> allProperties{arena};
		TArray<const TypeOps*> operations{arena};

		bool initialized = false;
	};

	struct TypeEditContext
	{
		// Index being currently edited
		i32 index = NO_INDEX;
		// Nested types being registered. A type can be registered from inside another
		TArray<TypeId> typeStack;
	};

	TypeRegistry& GetRegistry()
	{
		static TypeRegistry registry;
		return registry;
	}
	TArray<void (*)()>& GetRefectInitCallbacks()
	{
		static TArray<void (*)()> onInitCallbacks{GetHeapArena()};
		return onInitCallbacks;
	}

	static TypeEditContext currentEdit;

	bool TypeProperty::HasFlag(PropertyFlags flag) const
	{
		return HasAnyFlags(flag);
	}
	bool TypeProperty::HasAllFlags(PropertyFlags inFlags) const
	{
		return (flags & inFlags) == inFlags;
	}
	bool TypeProperty::HasAnyFlags(PropertyFlags inFlags) const
	{
		return (flags & inFlags) > 0;
	}

	void EnumTypeOps::SetValueFromIndex(void* data, i32 index) const
	{
		const void* valuePtr = GetValuePtrByIndex(index);
		CopyMem(data, valuePtr, valueSize);
	}
	void EnumTypeOps::SetValue(void* data, Tag valueName) const
	{
		i32 index = names.FindIndex(valueName);
		if (index != NO_INDEX)
		{
			SetValueFromIndex(data, index);
		}
	}
	const void* EnumTypeOps::GetValuePtrByIndex(i32 index) const
	{
		return (void*)(values.Data() + (index * valueSize));
	}
	Tag EnumTypeOps::GetNameByIndex(i32 index) const
	{
		return names.IsValidIndex(index) ? names[index] : Tag::None();
	}

	i32 EnumTypeOps::Size() const
	{
		return names.Size();
	}

	void* ContainerTypeOps::GetData(void* container) const
	{
		return getData(container);
	}
	i32 ContainerTypeOps::GetSize(void* container) const
	{
		return getSize(container);
	}
	void* ContainerTypeOps::GetItem(void* container, i32 index) const
	{
		return getItem(container, index);
	}
	void ContainerTypeOps::AddItem(void* container, void* item) const
	{
		addItem(container, item);
	}
	void ContainerTypeOps::RemoveItem(void* container, i32 index) const
	{
		removeItem(container, index);
	}
	void ContainerTypeOps::Clear(void* container) const
	{
		clear(container);
	}


	namespace details
	{
		i32 GetTypeIndex(const TypeRegistry& registry, TypeId id)
		{
			return registry.ids.FindSorted(id);
		}

		bool HasTypeFlags(const TypeRegistry& registry, i32 index, TypeFlags flags)
		{
			return index != NO_INDEX && (registry.flags[index] & flags) == flags;
		}
		bool HasAnyTypeFlags(const TypeRegistry& registry, i32 index, TypeFlags flags)
		{
			return index != NO_INDEX && (registry.flags[index] & flags) > 0;
		}
	}    // namespace details


	bool InitializeReflect()
	{
		auto& registry = GetRegistry();
		if (!P_EnsureMsg(!registry.initialized, "Reflection is already initialized"))
		{
			return false;
		}

		for (auto callback : GetRefectInitCallbacks())
		{
			if (callback)
			{
				callback();
			}
		}
		registry.initialized = true;
		return true;
	}

	void OnReflectInit(void (*callback)())
	{
		if (callback)
		{
			GetRefectInitCallbacks().Add(callback);
		}
	}

	TView<TypeId> GetRegisteredTypeIds()
	{
		return GetRegistry().ids;
	}

	bool IsTypeRegistered(TypeId id)
	{
		return details::GetTypeIndex(GetRegistry(), id) != NO_INDEX;
	}

	TypeId GetTypeParent(TypeId id)
	{
		auto& registry  = GetRegistry();
		const i32 index = details::GetTypeIndex(registry, id);
		return index != NO_INDEX ? registry.parentIds[index] : TypeId{};
	}

	bool IsTypeParentOf(TypeId parentId, TypeId childId)
	{
		TypeId currentId = childId;
		do
		{
			currentId = GetTypeParent(currentId);
			if (parentId == currentId)
				return true;
		} while (currentId.IsValid());
		return false;
	}

	sizet GetTypeSize(TypeId id)
	{
		const auto& registry = GetRegistry();
		const i32 index      = details::GetTypeIndex(registry, id);
		return index != NO_INDEX ? registry.sizes[index] : 0;
	}

	StringView GetTypeName(TypeId id)
	{
		const auto& registry = GetRegistry();
		const i32 index      = details::GetTypeIndex(registry, id);
		return index != NO_INDEX ? registry.names[index] : StringView{};
	}

	TypeFlags GetTypeFlags(TypeId id)
	{
		const auto& registry = GetRegistry();
		const i32 index      = details::GetTypeIndex(registry, id);
		return index != NO_INDEX ? registry.flags[index] : TF_None;
	}

	bool HasTypeFlags(TypeId id, TypeFlags flags)
	{
		auto& registry = GetRegistry();
		return details::HasTypeFlags(registry, details::GetTypeIndex(registry, id), flags);
	}
	bool HasAnyTypeFlags(TypeId id, TypeFlags flags)
	{
		auto& registry = GetRegistry();
		return details::HasAnyTypeFlags(registry, details::GetTypeIndex(registry, id), flags);
	}

	TView<const TypeProperty> GetOwnTypeProperties(TypeId id)
	{
		auto& registry  = GetRegistry();
		const i32 index = details::GetTypeIndex(registry, id);
		return index != NO_INDEX ? registry.ownProperties[index] : TView<TypeProperty>{};
	}
	TView<const TypeProperty*> GetTypeProperties(TypeId id)
	{
		auto& registry  = GetRegistry();
		const i32 index = details::GetTypeIndex(registry, id);
		return index != NO_INDEX ? registry.allProperties[index] : TView<const TypeProperty*>{};
	}

	const TypeOps* GetTypeOps(TypeId id)
	{
		auto& registry  = GetRegistry();
		const i32 index = details::GetTypeIndex(registry, id);
		return index != NO_INDEX ? registry.operations[index] : nullptr;
	}

	const ObjectTypeOps* GetTypeObjectOps(TypeId id)
	{
		auto& registry  = GetRegistry();
		const i32 index = details::GetTypeIndex(registry, id);
		return (index != NO_INDEX && (registry.flags[index] & TF_Object) == TF_Object)
		         ? static_cast<const ObjectTypeOps*>(registry.operations[index])
		         : nullptr;
	}

	const ContainerTypeOps* GetTypeContainerOps(TypeId id)
	{
		auto& registry  = GetRegistry();
		const i32 index = details::GetTypeIndex(registry, id);
		return (index != NO_INDEX && (registry.flags[index] & TF_Container) == TF_Container)
		         ? static_cast<const ContainerTypeOps*>(registry.operations[index])
		         : nullptr;
	}


#define P_CheckEditingType                      \
	P_CheckMsg(currentEdit.index != NO_INDEX,   \
	    "Not currently editing a type! Forgot " \
	    "BeginTypeId()?")

	bool BeginTypeId(TypeId id)
	{
		auto& registry  = GetRegistry();
		bool added      = false;
		const i32 index = registry.ids.AddUniqueSorted(id, {}, &added);
		if (!added)    // Not already registered?
		{
			return false;
		}
		currentEdit.index = index;
		currentEdit.typeStack.Add(id);

		registry.parentIds.Insert(index);
		registry.sizes.Insert(index);
		registry.names.Insert(index);
		registry.flags.Insert(index);
		registry.ownProperties.Insert(index);
		registry.allProperties.Insert(index);
		registry.operations.Insert(index);
		return true;
	}

	void EndTypeId()
	{
		if (!P_EnsureMsg(currentEdit.index != NO_INDEX,
		        "Trying to finish editing a type but no type is being edited. Forgot to call "
		        "BeginTypeId()?"))
		{
			return;
		}

		auto& reg = GetRegistry();

		{    // Cache inherited properties
			auto& allProperties = reg.allProperties[currentEdit.index];
			allProperties.Clear(false);

			// Assign properties from parent
			const i32 parentIdx = details::GetTypeIndex(reg, reg.parentIds[currentEdit.index]);
			if (parentIdx != NO_INDEX)
			{
				allProperties.Append(reg.allProperties[parentIdx]);
			}
			// Assign own properties
			const auto& ownProperties = reg.ownProperties[currentEdit.index];
			allProperties.ReserveMore(ownProperties.Size());
			for (auto& ownProp : ownProperties)
			{
				allProperties.Add(&ownProp);
			}
		}

		currentEdit.typeStack.RemoveLast();
		// Apply the index (that could have changed)
		currentEdit.index = currentEdit.typeStack.IsEmpty()
		                      ? NO_INDEX
		                      : details::GetTypeIndex(reg, currentEdit.typeStack.Last());
	}

	void SetTypeParent(TypeId parentId)
	{
		P_CheckEditingType;
		auto& reg                        = GetRegistry();
		reg.parentIds[currentEdit.index] = parentId;
	}

	void SetTypeSize(sizet size)
	{
		P_CheckEditingType;
		GetRegistry().sizes[currentEdit.index] = size;
	}

	void SetTypeName(StringView name)
	{
		P_CheckEditingType;
		GetRegistry().names[currentEdit.index] = name;
	}

	void SetTypeFlags(TypeFlags flags)
	{
		P_CheckEditingType;
		GetRegistry().flags[currentEdit.index] = flags;
	}

	void AddTypeFlags(TypeFlags flags)
	{
		P_CheckEditingType;
		GetRegistry().flags[currentEdit.index] |= flags;
	}

	void RemoveTypeFlags(TypeFlags flags)
	{
		P_CheckEditingType;
		GetRegistry().flags[currentEdit.index] &= ~flags;
	}

	void AddTypeProperty(const TypeProperty& property)
	{
		P_CheckEditingType;
		auto& ownProperties = GetRegistry().ownProperties[currentEdit.index];
		ownProperties.Add(property);
	}

	void SetTypeOps(const TypeOps* operations)
	{
		P_CheckEditingType;
		GetRegistry().operations[currentEdit.index] = operations;
	}


	TypeId BaseObject::GetTypeId() const
	{
		return static_cast<const Object*>(this)->GetTypeId();
	}
	TPtr<Object> BaseObject::AsPtr() const
	{
		return static_cast<const Object*>(this)->AsPtr();
	}


	TPtr<BaseObject> ObjectOwnership::nextOwner{};

	ObjectOwnership::ObjectOwnership() : self{}, owner{Move(nextOwner)} {}
	const TPtr<BaseObject>& ObjectOwnership::AsPtr() const
	{
		return self;
	}
	const TPtr<BaseObject>& ObjectOwnership::GetOwner() const
	{
		return owner;
	}

	void Object::ChangeOwner(const TPtr<BaseObject>& inOwner)
	{
		ownership.owner = inOwner;
	}
}    // namespace p
