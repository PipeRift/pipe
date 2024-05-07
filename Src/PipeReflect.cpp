// Copyright 2015-2024 Piperift - All rights reserved

#include "PipeReflect.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Platform.h"
#include "PipeECS.h"


namespace p
{
	struct TypeRegistry2
	{
		MultiLinearArena arena;

		TArray<TypeId> ids{arena};

		TArray<TypeId> parentIds{arena};
		TArray<StringView> names{arena};
		TArray<u64> flags{arena};
		TArray<TArray<TypeProperty>> properties{arena};
		TArray<const TypeOperations*> operations{arena};
	};

	struct TypeEditContext
	{
		// Index being currently edited
		i32 index = NO_INDEX;
		// Nested types being registered. A type can be registered from inside another
		TArray<TypeId> typeStack;
	};


	static TypeRegistry2 registry;
	static TypeEditContext currentEdit;


	i32 GetTypeIndex(TypeId id)
	{
		return registry.ids.FindSorted(id);
	}

	TypeId GetParentId(TypeId id)
	{
		const i32 index = GetTypeIndex(id);
		return index != NO_INDEX ? registry.parentIds[index] : TypeId{};
	}

	bool IsParentOf(TypeId parentId, TypeId childId)
	{
		TypeId currentId = childId;
		do
		{
			currentId = GetParentId(currentId);
			if (parentId == currentId)
				return true;
		} while (currentId.IsValid());
		return false;
	}

	StringView GetTypeName(TypeId id)
	{
		const i32 index = GetTypeIndex(id);
		return index != NO_INDEX ? registry.names[index] : StringView{};
	}


	namespace details
	{
#define P_CheckEditingType                      \
	P_CheckMsg(currentEdit.index != NO_INDEX,   \
	    "Not currently editing a type! Forgot " \
	    "BeginTypeId()?")


		bool BeginTypeId(TypeId id)
		{
			bool added      = false;
			const i32 index = registry.ids.AddUniqueSorted(id, {}, &added);
			if (!added)    // Not already registered?
			{
				return false;
			}
			currentEdit.index = index;
			currentEdit.typeStack.Add(id);

			registry.parentIds.Insert(index);
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
			                      : GetTypeIndex(currentEdit.typeStack.Last());
		}

		void SetTypeParent(TypeId parentId)
		{
			P_CheckEditingType;
			registry.parentIds[currentEdit.index] = parentId;
		}

		void SetTypeName(StringView name)
		{
			P_CheckEditingType;
			registry.names[currentEdit.index] = name;
		}

		void SetTypeFlags(u64 flags)
		{
			P_CheckEditingType;
			registry.flags[currentEdit.index] = flags;
		}

		void AddTypeProperty(const TypeProperty& property)
		{
			P_CheckEditingType;
			auto& properties = registry.properties[currentEdit.index];
			properties.Add(property);
		}

		void SetTypeOperations(const TypeOperations* operations)
		{
			P_CheckEditingType;
			registry.operations[currentEdit.index] = operations;
		}
	}    // namespace details
}    // namespace p
