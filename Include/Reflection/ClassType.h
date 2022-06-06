// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Core/Function.h"
#include "Reflection/BaseClass.h"
#include "Reflection/DataType.h"


namespace p
{
	class ClassType : public DataType
	{
		template<typename T, typename Parent, TypeFlags flags, typename TType>
		friend struct TClassTypeBuilder;

		using CreateFunc = TFunction<BaseClass*()>;

		CreateFunc onNew;
		mutable BaseClass* defaultValue;


	public:
		ClassType() : DataType(TypeCategory::Class) {}
		~ClassType()
		{
			delete defaultValue;
		}

		PIPE_API BaseClass* New() const;

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


		PIPE_API BaseClass* GetDefaultPtr() const
		{
			if (!defaultValue)
			{
				defaultValue = New();
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
