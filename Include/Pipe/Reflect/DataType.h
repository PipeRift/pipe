// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Reflect/Builders/CompiledTypeRegister.h"
#include "Pipe/Reflect/Property.h"
#include "Pipe/Reflect/ReflectionFlags.h"
#include "Pipe/Reflect/ReflectionTraits.h"
#include "Pipe/Reflect/Type.h"
#include "PipeArrays.h"


namespace p
{
	/** Smallest reflection type that contains all basic class or struct data */
	class DataType : public Type
	{
		template<typename T, typename Parent, TypeFlags flags, typename TType>
		friend struct TDataTypeBuilder;
		using ReadFunc  = void(Reader&, void*);
		using WriteFunc = void(Writer&, void*);

	protected:
		TypeFlags flags = Type_NoFlag;

		DataType* parent = nullptr;
		TArray<DataType*> children;

		TArray<Property*> properties;

		ReadFunc* read;
		WriteFunc* write;

	public:
		static constexpr TypeCategory typeCategory = TypeCategory::Data;


	protected:
		PIPE_API DataType(TypeCategory category) : Type(category | typeCategory) {}

	public:
		DataType(const DataType&)            = delete;
		DataType& operator=(const DataType&) = delete;
		PIPE_API ~DataType() override;

		/** Inheritance */
		PIPE_API bool IsChildOf(const DataType* other) const;
		template<typename T>
		bool IsChildOf() const;
		PIPE_API DataType* GetParent() const;
		PIPE_API const TArray<DataType*>& GetChildren() const;
		PIPE_API void GetChildrenDeep(TArray<DataType*>& outChildren) const;
		PIPE_API DataType* FindChild(const Tag& className) const;
		PIPE_API bool IsParentOf(const DataType* other) const;

		/** Flags */
		PIPE_API bool HasFlag(TypeFlags flag) const;
		PIPE_API bool HasAllFlags(TypeFlags inFlags) const;
		PIPE_API bool HasAnyFlags(TypeFlags inFlags) const;

		/** Properties */
		PIPE_API const Property* FindProperty(const Tag& propertyName) const;
		PIPE_API const TArray<Property*>& GetSelfProperties() const;
		PIPE_API void GetProperties(TArray<Property*>& outProperties) const;
		PIPE_API bool IsEmpty() const;

		/** Serialization */
		PIPE_API void Read(Reader& r, void* container);
		PIPE_API void Write(Writer& w, void* container);
	};


	template<typename T>
	inline bool DataType::IsChildOf() const
	{
		static_assert(
		    IsStruct<T>() || IsClass<T>(), "IsChildOf only valid with Structs or Classes.");
		return IsChildOf(static_cast<DataType*>(TCompiledTypeRegister<T>::GetType()));
	}

	inline void DataType::Read(Reader& r, void* container)
	{
		read(r, container);
	}
	inline void DataType::Write(Writer& w, void* container)
	{
		write(w, container);
	}
}    // namespace p
