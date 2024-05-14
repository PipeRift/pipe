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
		TArray<TArray<TypeProperty>> properties{arena};
		TArray<const TypeOps*> operations{arena};

		bool initialized = false;
		TArray<void (*)()> onInitCallbacks{GetHeapArena()};
	};

	struct TypeEditContext
	{
		// Index being currently edited
		i32 index = NO_INDEX;
		// Nested types being registered. A type can be registered from inside another
		TArray<TypeId> typeStack;
	};

	TypeRegistry& AssureRegistry()
	{
		static TypeRegistry registry;
		return registry;
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
		i32 GetTypeIndex(TypeId id)
		{
			return AssureRegistry().ids.FindSorted(id);
		}
	}    // namespace details


	bool InitializeReflect()
	{
		auto& registry = AssureRegistry();
		if (!P_EnsureMsg(!registry.initialized, "Reflection is already initialized"))
		{
			return false;
		}

		for (auto callback : registry.onInitCallbacks)
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
			AssureRegistry().onInitCallbacks.Add(callback);
		}
	}

	TView<TypeId> GetRegisteredTypeIds()
	{
		return AssureRegistry().ids;
	}

	bool IsTypeRegistered(TypeId id)
	{
		return details::GetTypeIndex(id) != NO_INDEX;
	}

	TypeId GetTypeParent(TypeId id)
	{
		const i32 index = details::GetTypeIndex(id);
		return index != NO_INDEX ? AssureRegistry().parentIds[index] : TypeId{};
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
		const i32 index = details::GetTypeIndex(id);
		return index != NO_INDEX ? AssureRegistry().sizes[index] : 0;
	}

	StringView GetTypeName(TypeId id)
	{
		const i32 index = details::GetTypeIndex(id);
		return index != NO_INDEX ? AssureRegistry().names[index] : StringView{};
	}

	TypeFlags GetTypeFlags(TypeId id)
	{
		const i32 index = details::GetTypeIndex(id);
		return index != NO_INDEX ? AssureRegistry().flags[index] : TF_None;
	}

	bool HasTypeFlags(TypeId id, TypeFlags flags)
	{
		const i32 index = details::GetTypeIndex(id);
		return index != NO_INDEX && (AssureRegistry().flags[index] & flags) == flags;
	}
	bool HasAnyTypeFlags(TypeId id, TypeFlags flags)
	{
		const i32 index = details::GetTypeIndex(id);
		return index != NO_INDEX && (AssureRegistry().flags[index] & flags) > 0;
	}

	TView<TypeProperty> GetTypeProperties(TypeId id)
	{
		const i32 index = details::GetTypeIndex(id);
		return index != NO_INDEX ? AssureRegistry().properties[index] : TView<TypeProperty>{};
	}

	const TypeOps* GetTypeOps(TypeId id)
	{
		const i32 index = details::GetTypeIndex(id);
		return index != NO_INDEX ? AssureRegistry().operations[index] : nullptr;
	}

	const ObjectTypeOps* GetTypeObjectOps(TypeId id)
	{
		auto& registry  = AssureRegistry();
		const i32 index = details::GetTypeIndex(id);
		return (index != NO_INDEX && (registry.flags[index] & TF_Object) == TF_Object)
		         ? static_cast<const ObjectTypeOps*>(registry.operations[index])
		         : nullptr;
	}

	const ContainerTypeOps* GetTypeContainerOps(TypeId id)
	{
		auto& registry  = AssureRegistry();
		const i32 index = details::GetTypeIndex(id);
		return (index != NO_INDEX && (registry.flags[index] & TF_Object) == TF_Object)
		         ? static_cast<const ContainerTypeOps*>(registry.operations[index])
		         : nullptr;
	}


#define P_CheckEditingType                      \
	P_CheckMsg(currentEdit.index != NO_INDEX,   \
	    "Not currently editing a type! Forgot " \
	    "BeginTypeId()?")

	bool BeginTypeId(TypeId id)
	{
		auto& registry  = AssureRegistry();
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
		registry.properties.Insert(index);
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
		currentEdit.typeStack.RemoveLast();
		// Apply the index (that could have changed)
		currentEdit.index = currentEdit.typeStack.IsEmpty()
		                      ? NO_INDEX
		                      : details::GetTypeIndex(currentEdit.typeStack.Last());
	}

	void SetTypeParent(TypeId parentId)
	{
		P_CheckEditingType;
		AssureRegistry().parentIds[currentEdit.index] = parentId;
	}

	void SetTypeSize(sizet size)
	{
		P_CheckEditingType;
		AssureRegistry().sizes[currentEdit.index] = size;
	}

	void SetTypeName(StringView name)
	{
		P_CheckEditingType;
		AssureRegistry().names[currentEdit.index] = name;
	}

	void SetTypeFlags(TypeFlags flags)
	{
		P_CheckEditingType;
		AssureRegistry().flags[currentEdit.index] = flags;
	}

	void AddTypeFlags(TypeFlags flags)
	{
		P_CheckEditingType;
		AssureRegistry().flags[currentEdit.index] |= flags;
	}

	void RemoveTypeFlags(TypeFlags flags)
	{
		P_CheckEditingType;
		AssureRegistry().flags[currentEdit.index] &= ~flags;
	}

	void AddTypeProperty(const TypeProperty& property)
	{
		P_CheckEditingType;
		auto& properties = AssureRegistry().properties[currentEdit.index];
		properties.Add(property);
	}

	void SetTypeOps(const TypeOps* operations)
	{
		P_CheckEditingType;
		AssureRegistry().operations[currentEdit.index] = operations;
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
