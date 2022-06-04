// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Reflection/DataType.h"
#include "Types/BaseClass.h"


namespace Pipe::Refl
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

		CORE_API BaseClass* New() const;

		CORE_API ClassType* GetParent() const
		{
			return static_cast<ClassType*>(parent);
		}

		CORE_API const TArray<ClassType*>& GetChildren() const
		{
			// Classes only have Class children. It is safe to reinterpret_cast.
			return reinterpret_cast<const TArray<ClassType*>&>(DataType::GetChildren());
		}

		CORE_API void GetChildrenDeep(TArray<ClassType*>& outChildren) const
		{
			// Classes only have Class children. It is safe to reinterpret_cast.
			DataType::GetChildrenDeep(reinterpret_cast<TArray<DataType*>&>(outChildren));
		}

		CORE_API ClassType* FindChild(const Name& className) const
		{
			// Classes only have Class children. It is safe to static_cast.
			return static_cast<ClassType*>(DataType::FindChild(className));
		}

		CORE_API bool IsA(ClassType* other) const
		{
			return this == other;
		}


		CORE_API BaseClass* GetDefaultPtr() const
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
}    // namespace Pipe::Refl
