// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Function.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Reflect/BaseClass.h"
#include "Pipe/Reflect/DataType.h"


namespace p
{
	class ClassType : public DataType
	{
		template<typename T, typename Parent, TypeFlags flags, typename TType>
		friend struct TClassTypeBuilder;

		using NewFunc = TFunction<BaseClass*(Arena& arena)>;

		NewFunc onNew;
		mutable TOwnPtr<BaseClass> defaultValue;

	public:
		static constexpr TypeCategory typeCategory = TypeCategory::Class;


	public:
		ClassType() : DataType(typeCategory) {}
		~ClassType() {}

		PIPE_API BaseClass* New(Arena& arena) const;

		PIPE_API ClassType* GetParent() const
		{
			return static_cast<ClassType*>(parent);
		}

		PIPE_API const TArray<ClassType*>& GetChildren() const
		{
			// Classes only have Class children. It is safe to reinterpret_cast.
			return reinterpret_cast<const TArray<ClassType*>&>(DataType::GetChildren());
		}

		PIPE_API void GetChildrenDeep(TArray<ClassType*>& outChildren) const
		{
			// Classes only have Class children. It is safe to reinterpret_cast.
			DataType::GetChildrenDeep(reinterpret_cast<TArray<DataType*>&>(outChildren));
		}

		PIPE_API ClassType* FindChild(const Name& className) const
		{
			// Classes only have Class children. It is safe to static_cast.
			return static_cast<ClassType*>(DataType::FindChild(className));
		}

		PIPE_API bool IsA(ClassType* other) const
		{
			return this == other;
		}

		PIPE_API TPtr<BaseClass> GetDefaultPtr() const
		{
			if (!defaultValue)
			{
				Arena& arena = GetCurrentArena();
				auto deleter = [](Arena* arena, void* ptr) {
					// Delete(arena, static_cast<BaseClass*>(ptr));
				};
				defaultValue = TOwnPtr<BaseClass>(arena, New(arena), {});
			}
			return defaultValue;
		}

		template<typename T>
		T& GetDefault() const requires(IsClass<T>())
		{
			Check(T::GetType() == this);
			CheckMsg(!HasFlag(Class_Abstract), "Tried to get default from an abstract class");
			return *GetDefaultPtr();
		}
	};
}    // namespace p
