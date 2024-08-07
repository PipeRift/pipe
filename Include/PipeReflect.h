// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/Guid.h"
#include "Pipe/Core/Macros.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Tag.h"
#include "Pipe/Core/TypeId.h"
#include "Pipe/Core/TypeName.h"
#include "Pipe/Extern/magic_enum.hpp"
#include "Pipe/Files/STDFileSystem.h"
#include "Pipe/Memory/OwnPtr.h"
#include "PipeColor.h"
#include "PipeSerialize.h"
#include "PipeVectors.h"


#ifndef P_AUTOREGISTER_ENABLED
	#define P_AUTOREGISTER_ENABLED 1
#endif


namespace p
{
	class Object;
	class BaseObject;

#pragma region Traits
	template<typename T>
	concept IsObject = Derived<T, class BaseObject, false>;

	template<typename T>
	consteval bool HasSuper()
	{
		return requires { typename T::Super; };
	}

	template<typename T>
	consteval bool HasMemberBuildType()
	{
		return requires() { T::BuildType(); };
	}

	template<typename T>
	consteval bool HasExternalBuildType()
	{
		return requires(const T v) { BuildType(&v); };
	}

	template<typename T>
	consteval bool CanBuildType()
	{
		return HasMemberBuildType<T>() || HasExternalBuildType<T>();
	}

	template<u32 N>
	struct MetaCounter : MetaCounter<N - 1>
	{
		static constexpr u32 value = N;
	};
	template<>
	struct MetaCounter<0>
	{
		static constexpr u32 value = 0;
	};
#pragma endregion Traits


#pragma region Enums
	template<typename T>
	constexpr sizet GetEnumSize()
	{
		return magic_enum::enum_count<T>();
	}

	template<typename T>
	constexpr std::optional<T> GetEnumValue(StringView str)
	{
		return magic_enum::enum_cast<T>(str);
	}

	template<typename T>
	constexpr auto GetEnumValues()
	{
		return magic_enum::enum_values<T>();
	}

	template<typename T>
	constexpr StringView GetEnumName(T value)
	{
		return magic_enum::enum_name(value);
	}

	template<typename T>
	constexpr auto GetEnumNames()
	{
		return magic_enum::enum_names<T>();
	}

	template<typename T>
	constexpr void GetEnumFlagName(T value, String& outName)
	{
		bool hasAny = false;
		for (auto v : GetEnumValues<T>())
		{
			if (static_cast<UnderlyingType<T>>(v) != 0 && HasFlag(value, v))
			{
				outName.append(GetEnumName(v));
				outName.append(" | ");
				hasAny = true;
			}
		}
		if (hasAny)
		{
			outName.resize(outName.size() - 3);
		}
	}
#pragma endregion Enums

#pragma region Serialization
	template<typename T>
	consteval bool HasReadProperties()
	{
		return requires(T v, p::Reader reader) { v.ReadProperties(reader); };
	}
	template<typename T>
	consteval bool HasWriteProperties()
	{
		return requires(const T v, p::Writer writer) { v.WriteProperties(writer); };
	}

	PIPE_API void Read(Reader& r, TypeId& val);
	PIPE_API void Write(Writer& w, TypeId val);

	template<typename T>
	void Read(Reader& ct, T& value) requires(
	    HasReadProperties<T>() && !TFlags<T>::HasMemberSerialize && !TFlags<T>::HasSingleSerialize)
	{
		ct.BeginObject();
		value.ReadProperties(ct);
	}

	template<typename T>
	void Write(Writer& ct, const T& value) requires(
	    HasWriteProperties<T>() && !TFlags<T>::HasMemberSerialize && !TFlags<T>::HasSingleSerialize)
	{
		ct.BeginObject();
		value.WriteProperties(ct);
	}

	template<typename T>
	void Read(Reader& r, T& val) requires(IsEnum<T>)
	{
		if constexpr (GetEnumSize<T>() > 0)
		{
			StringView typeStr;
			r.Serialize(typeStr);
			if (std::optional<T> value = GetEnumValue<T>(typeStr))
			{
				val = value.value();
			}
		}
	}
	template<typename T>
	void Write(Writer& w, T& val) requires IsEnum<T>
	{
		if constexpr (GetEnumSize<T>() > 0)
		{
			// Might not be necessary to cache string since enum name is static
			w.PushAddFlags(WriteFlags_CacheStringValues);
			w.Serialize(GetEnumName(val));
			w.PopFlags();
		}
	}
#pragma endregion Serialization


#pragma region Flags
	using TypeFlags = u64;
	enum TypeFlags_
	{
		TF_None          = 0,         // -> No flags
		TF_NotSerialized = 1 << 0,    // -> This type ignores serialization
		TF_Abstract      = 1 << 1,    // -> Reflection cant create instances on this type

		TF_Native    = 1 << 2,
		TF_Enum      = 1 << 5,
		TF_Struct    = 1 << 3,
		TF_Object    = 1 << 4,
		TF_Container = 1 << 6

		// Any other flags up to 64 bytes are available to the user
	};


	P_DEFINE_FLAG_OPERATORS(TypeFlags_)

	using PropertyFlags = u64;
	enum PropertyFlags_
	{
		PF_None          = 0,
		PF_NotSerialized = 1 << 0,
		PF_View          = 1 << 1,
		PF_Edit          = 1 << 2 | PF_View,

		// Internal use only
		PF_Array   = 1 << 3,    // Assigned automatically when a property is an array
		PF_Pointer = 1 << 4     // Assigned automatically when a property is a pointer
	};
	P_DEFINE_FLAG_OPERATORS(PropertyFlags_)


	// Functions used to surround macro flag values making them template safe
	constexpr TypeFlags InitTypeFlags()
	{
		return TF_None;
	}
	constexpr TypeFlags InitTypeFlags(TypeFlags flags)
	{
		return flags;
	}
	constexpr PropertyFlags InitPropertyFlags()
	{
		return PF_None;
	}
	constexpr PropertyFlags InitPropertyFlags(PropertyFlags flags)
	{
		return flags;
	}
#pragma endregion Flags


#pragma region Runtime
	struct PIPE_API TypeProperty
	{
		using AccessFunc = void*(void*);

		TypeId typeId;
		u64 flags          = 0;
		AccessFunc* access = nullptr;
		Tag name;

		bool HasFlag(PropertyFlags flag) const;
		bool HasAllFlags(PropertyFlags inFlags) const;
		bool HasAnyFlags(PropertyFlags inFlags) const;
	};


	struct PIPE_API TypeOps
	{
		using ReadFunc  = void(Reader&, void*);
		using WriteFunc = void(Writer&, void*);

		ReadFunc* read   = nullptr;
		WriteFunc* write = nullptr;
	};

	struct PIPE_API EnumTypeOps : public TypeOps
	{
		u32 valueSize = 0;
		TArray<u8> values;
		TArray<Tag> names;


		void SetValueFromIndex(void* data, i32 index) const;
		void SetValue(void* data, Tag valueName) const;
		template<Integral T>
		void SetValue(void* data, T value) const
		{
			P_Check(sizeof(T) == valueSize);
			CopyMem(data, &value, valueSize);
		}

		const void* GetValuePtrByIndex(i32 index) const;
		template<Integral T>
		const T& GetValueByIndex(i32 index) const
		{
			return *reinterpret_cast<T*>(GetValuePtrByIndex(index));
		}
		Tag GetNameByIndex(i32 index) const;

		i32 Size() const;
	};

	struct PIPE_API ObjectTypeOps : public TypeOps
	{
		using NewFunc = BaseObject*(Arena& arena);

		NewFunc* onNew = nullptr;
	};

	struct PIPE_API ContainerTypeOps : public TypeOps
	{
		using GetDataFunc    = void*(void*);
		using GetSizeFunc    = i32(void*);
		using GetItemFunc    = void*(void*, i32);
		using AddItemFunc    = void(void*, void*);
		using RemoveItemFunc = void(void*, i32);
		using ClearFunc      = void(void*);

		TypeId itemType            = TypeId::None();
		GetDataFunc* getData       = nullptr;
		GetSizeFunc* getSize       = nullptr;
		GetItemFunc* getItem       = nullptr;
		AddItemFunc* addItem       = nullptr;
		RemoveItemFunc* removeItem = nullptr;
		ClearFunc* clear           = nullptr;


		void* GetData(void* container) const;
		i32 GetSize(void* container) const;
		void* GetItem(void* container, i32 index) const;
		void AddItem(void* container, void* item) const;
		void RemoveItem(void* container, i32 index) const;
		void Clear(void* container) const;
	};

	struct TypeRegistry;
	namespace details
	{
		PIPE_API i32 GetTypeIndex(const TypeRegistry& registry, TypeId id);
		PIPE_API bool HasTypeFlags(const TypeRegistry& registry, i32 index, TypeFlags flags);
		PIPE_API bool HasAnyTypeFlags(const TypeRegistry& registry, i32 index, TypeFlags flags);
	}    // namespace details

	PIPE_API bool InitializeReflect();
	PIPE_API void OnReflectInit(void (*callback)());

	PIPE_API TView<TypeId> GetRegisteredTypeIds();
	PIPE_API bool IsTypeRegistered(TypeId id);
	PIPE_API TypeId GetTypeParent(TypeId id);
	PIPE_API bool IsTypeParentOf(TypeId parentId, TypeId childId);
	PIPE_API sizet GetTypeSize(TypeId id);
	PIPE_API StringView GetTypeName(TypeId id);
	PIPE_API TypeFlags GetTypeFlags(TypeId id);
	PIPE_API bool HasTypeFlags(TypeId id, TypeFlags flags);
	PIPE_API bool HasAnyTypeFlags(TypeId id, TypeFlags flags);
	/// @return the properties of this type, excluding those of the parent (if any)
	PIPE_API TView<const TypeProperty> GetOwnTypeProperties(TypeId id);
	/// @return the properties of this type
	PIPE_API TView<const TypeProperty*> GetTypeProperties(TypeId id);
	PIPE_API const TypeOps* GetTypeOps(TypeId id);
	PIPE_API const ObjectTypeOps* GetTypeObjectOps(TypeId id);
	PIPE_API const ContainerTypeOps* GetTypeContainerOps(TypeId id);


#pragma region Registration
	/** To register types, define either of the following functions for that type:
	 * - A member static function BuildType() inside of the type to register.
	 * - An external function BuildType(const T*) where T is the type.
	 *   The parameter is used as a dummy for overloading. Will always be null.
	 *
	 * NOTE: BuildType() is optional! Only needed to add properties, operations, etc.
	 *       Types can also be of any kind, including templates.
	 *
	 * Examples:
	 * - A member BuildType():
	 *     struct A {
	 *         bool value = false;
	 *         static void BuildType()
	 *         {
	 *             SetTypeName("A");
	 *             AddTypeProperty("value", &A::value);
	 *         }
	 *     };
	 *
	 * - An external BuildType():
	 *     struct B {
	 *         bool value = false;
	 *     };
	 *     // NOTE: Should be placed outside of any namespaces (except "p" which is optional)
	 *     void BuildType(const B*)
	 *     {
	 *         SetTypeName("B");
	 *         AddTypeProperty("value", &B::value);
	 *     }
	 */

	// Resolve the right BuildType function to call
	template<typename T>
	void BuildType(const T*) requires(HasMemberBuildType<T>())
	{
		T::BuildType();
	}


	template<typename T>
	TypeId RegisterTypeId();    // Forward declaration

	PIPE_API bool BeginTypeId(TypeId id);
	PIPE_API void EndTypeId();
	PIPE_API void SetTypeParent(TypeId parentId);
	PIPE_API void SetTypeSize(sizet size);
	PIPE_API void SetTypeName(StringView name);
	PIPE_API void SetTypeFlags(TypeFlags flags);
	PIPE_API void AddTypeFlags(TypeFlags flags);
	PIPE_API void RemoveTypeFlags(TypeFlags flags);
	PIPE_API void AddTypeProperty(const TypeProperty& property);
	template<typename T>
	constexpr void AddTypeProperty(
	    TypeProperty::AccessFunc* access, StringView name, PropertyFlags flags = PF_None)
	{
		using PropType = std::remove_pointer_t<T>;
		if constexpr (IsPointer<T>)
		{
			if constexpr (IsPointer<PropType>)
			{
				return;    // Nested pointers not supported
			}
			flags |= PF_Pointer;
		}
		AddTypeProperty(TypeProperty{
		    .typeId = RegisterTypeId<PropType>(), .flags = flags, .access = access, .name = name});
	}

	PIPE_API void SetTypeOps(const TypeOps* operations);
	template<typename T>
	void AssignSerializableTypeOps(TypeOps& ops)
	{
		ops.read = [](Reader& r, void* instance) {
			if constexpr (Readable<T>)
			{
				r.Serialize(*static_cast<T*>(instance));
			}
		};
		ops.write = [](Writer& w, void* instance) {
			if constexpr (Writable<T>)
			{
				w.Serialize(*static_cast<T*>(instance));
			}
		};
	}
	template<typename T>
	void AssignEnumTypeOps(EnumTypeOps& ops)
	{
		if constexpr (GetEnumSize<T>() > 0)
		{
			const auto valueNames = GetEnumNames<T>();
			ops.names.Reserve(valueNames.size());
			for (StringView valueName : valueNames)
			{
				ops.names.Add(Tag{valueName});
			}

			const auto values = GetEnumValues<T>();
			ops.valueSize     = sizeof(T);
			ops.values.Resize(values.size() * ops.valueSize);
			for (i32 i = 0; i < values.size(); ++i)
			{
				// Copy value into the correct entry index
				memcpy(ops.values.Data() + (i * ops.valueSize), &values[i], ops.valueSize);
			}
		}
		AddTypeFlags(TF_Enum);
	}
	template<typename T>
	void AssignObjectTypeOps(ObjectTypeOps& ops)
	{
		ops.onNew = [](Arena& arena) -> BaseObject* {
			if constexpr (!IsAbstract<T> && !IsSame<T, BaseObject>)
			{
				return new (p::Alloc<T>(arena)) T();
			}
			return nullptr;    // Can't create instances of abstract objects
		};
		AddTypeFlags(TF_Object);
	}


	template<typename T>
	TypeId RegisterTypeId()
	{    // Static to only register once
		static bool bRegistered = false;
		const TypeId typeId     = GetTypeId<T>();
		if (!bRegistered && BeginTypeId(typeId))
		{
			bRegistered = true;

			// Obvious defaults
			if constexpr (HasSuper<T>())
			{
				SetTypeParent(RegisterTypeId<typename T::Super>());
			}
			SetTypeSize(sizeof(T));
			SetTypeName(GetTypeName<T>());

			if constexpr (IsStructOrClass<T>)
			{
				AddTypeFlags(TF_Struct);
				if constexpr (IsObject<T>)
				{
					static ObjectTypeOps objectOps;
					AssignSerializableTypeOps<T>(objectOps);
					AssignObjectTypeOps<T>(objectOps);
					SetTypeOps(&objectOps);
				}
				else
				{
					static TypeOps basicOps;
					AssignSerializableTypeOps<T>(basicOps);
					SetTypeOps(&basicOps);
				}
			}
			else if constexpr (IsEnum<T>)
			{
				static EnumTypeOps enumOps;
				AssignSerializableTypeOps<T>(enumOps);
				AssignEnumTypeOps<T>(enumOps);
				SetTypeOps(&enumOps);
			}

			if constexpr (CanBuildType<T>())
			{
				if constexpr (HasMemberBuildType<T>())
				{
					T::BuildType();
				}
				else if constexpr (HasExternalBuildType<T>())
				{
					BuildType((const T*)nullptr);
				}
			}

			EndTypeId();
		}
		return typeId;
	}

	template<typename T>
	struct TTypeAutoRegister
	{
		TTypeAutoRegister()
		{
#if P_AUTOREGISTER_ENABLED
			OnReflectInit([] {
				RegisterTypeId<T>();
			});
#endif
		}
	};

	template<typename T>
	struct TStaticTypeAutoRegister
	{
		static inline TTypeAutoRegister<T> instance;
	};
#pragma endregion Registration
#pragma endregion Runtime


	template<typename T>
	void CallSuperReadProperties(T& value, p::Reader& r)
	{}
	template<typename T>
	void CallSuperReadProperties(const T& value, p::Reader& r) requires(p::HasSuper<T>())
	{
		if constexpr (p::HasReadProperties<typename T::Super>())
			value.Super::ReadProperties(r);
	}
	template<typename T>
	void CallSuperWriteProperties(const T& value, p::Writer& w)
	{}
	template<typename T>
	void CallSuperWriteProperties(const T& value, p::Writer& w) requires(p::HasSuper<T>())
	{
		if constexpr (p::HasWriteProperties<typename T::Super>())
			value.Super::WriteProperties(w);
	}
}    // namespace p


#pragma region Macros
#define P_AUTOREGISTER_TYPE(type)                       \
	template<>                                          \
	struct p::TStaticTypeAutoRegister<type>             \
	{                                                   \
		static inline TTypeAutoRegister<type> instance; \
	};

#define P_NATIVE(type)                     \
	namespace p                            \
	{                                      \
		inline void BuildType(const type*) \
		{                                  \
			p::AddTypeFlags(p::TF_Native); \
		};                                 \
	}                                      \
	P_AUTOREGISTER_TYPE(type)

#define P_NATIVE_NAMED(type, name)                                                  \
	template<>                                                                      \
	inline consteval p::StringView p::GetFullTypeName<type>(bool includeNamespaces) \
	{                                                                               \
		return name;                                                                \
	}                                                                               \
	P_NATIVE(type);

#define P_ENUM(type) P_AUTOREGISTER_TYPE(type)


/** Defines a Class */
#define P_CLASS_HEADER_NO_FLAGS(type) P_CLASS_HEADER_FLAGS(type, p::TF_None)
#define P_CLASS_HEADER_FLAGS(type, flags)                                    \
public:                                                                      \
	using Self                                    = type;                    \
	static constexpr p::TypeFlags staticTypeFlags = p::InitTypeFlags(flags); \
                                                                             \
	p::TypeId GetTypeId() const override                                     \
	{                                                                        \
		return p::GetTypeId<Self>();                                         \
	}


/** Defines an Struct */
#define P_STRUCT_HEADER_NO_FLAGS(type) P_STRUCT_HEADER_FLAGS(type, p::TF_None)
#define P_STRUCT_HEADER_FLAGS(type, flags)                                   \
public:                                                                      \
	using Self                                    = type;                    \
	static constexpr p::TypeFlags staticTypeFlags = p::InitTypeFlags(flags); \
	static_assert(!(staticTypeFlags & p::TF_Abstract), "Only objects can use TF_Abstract");


#define P_REFLECTION_BODY(buildCode)                                      \
private:                                                                  \
	static inline p::TTypeAutoRegister<Self> typeAutoRegister;            \
                                                                          \
public:                                                                   \
	static void BuildType()                                               \
	{                                                                     \
		__BuildProperty(p::MetaCounter<0>{});                             \
	}                                                                     \
	void ReadProperties(p::Reader& r)                                     \
	{                                                                     \
		if constexpr (!(staticTypeFlags & p::TF_NotSerialized))           \
		{                                                                 \
			CallSuperReadProperties<Self>(*this, r);                      \
			__ReadProperty(r, p::MetaCounter<0>{});                       \
		}                                                                 \
	}                                                                     \
	void WriteProperties(p::Writer& w) const                              \
	{                                                                     \
		if constexpr (!(staticTypeFlags & p::TF_NotSerialized))           \
		{                                                                 \
			CallSuperWriteProperties<Self>(*this, w);                     \
			__WriteProperty(w, p::MetaCounter<0>{});                      \
		}                                                                 \
	}                                                                     \
                                                                          \
private:                                                                  \
	static constexpr p::MetaCounter<0> __refl_Counter(p::MetaCounter<0>); \
	template<p::u32 index>                                                \
	static void __BuildProperty(p::MetaCounter<index>)                    \
	{}                                                                    \
	template<p::u32 index>                                                \
	void __ReadProperty(p::Reader&, p::MetaCounter<index>)                \
	{}                                                                    \
	template<p::u32 index>                                                \
	void __WriteProperty(p::Writer&, p::MetaCounter<index>) const         \
	{}                                                                    \
                                                                          \
public:


#define P_PROPERTY_NO_FLAGS(name) P_PROPERTY_FLAGS(name, p::PF_None)
#define P_PROPERTY_FLAGS(name, flags) __P_PROPERTY_IMPL(name, P_CAT(__refl_id_, name), flags)
#define __P_PROPERTY_IMPL(name, id_name, flags)                                                   \
	static constexpr p::u32 id_name = decltype(__refl_Counter(p::MetaCounter<255>{}))::value;     \
	static constexpr p::MetaCounter<(id_name) + 1> __refl_Counter(p::MetaCounter<(id_name) + 1>); \
                                                                                                  \
	static void __BuildProperty(p::MetaCounter<id_name>)                                          \
	{                                                                                             \
		p::AddTypeProperty<decltype(name)>(                                                       \
		    [](void* instance) {                                                                  \
			return (void*)&static_cast<Self*>(instance)->name;                                    \
		    },                                                                                    \
		    #name, p::InitPropertyFlags(flags));                                                  \
		/* Registry next property if any */                                                       \
		__BuildProperty(p::MetaCounter<id_name + 1>{});                                           \
	}                                                                                             \
	void __ReadProperty(p::Reader& r, p::MetaCounter<id_name>)                                    \
	{                                                                                             \
		if constexpr (!(p::InitPropertyFlags(flags) & p::PF_NotSerialized))                       \
		{ /* Don't serialize property if NotSerialized */                                         \
			r.Next(#name, name);                                                                  \
		}                                                                                         \
		/* Serialize next property if any */                                                      \
		__ReadProperty(r, p::MetaCounter<id_name + 1>{});                                         \
	}                                                                                             \
	void __WriteProperty(p::Writer& w, p::MetaCounter<id_name>) const                             \
	{                                                                                             \
		if constexpr (!(p::InitPropertyFlags(flags) & p::PF_NotSerialized))                       \
		{ /* Don't serialize property if NotSerialized */                                         \
			w.Next(#name, name);                                                                  \
		}                                                                                         \
		/* Serialize next property if any*/                                                       \
		__WriteProperty(w, p::MetaCounter<id_name + 1>{});                                        \
	}


#define P_REFL_INTERNAL_GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
#define P_REFL_GET_3RD_ARG(tuple) P_REFL_INTERNAL_GET_3RD_ARG tuple

#define P_REFL_INTERNAL_GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define P_REFL_GET_4TH_ARG(tuple) P_REFL_INTERNAL_GET_4TH_ARG tuple

#define P_TYPE_INVALID(...) static_assert(false, "Invalid type macro. Missing first parameters");

#define P_TYPE_CHOOSER(TYPE_NO_FLAGS, TYPE_FLAGS, type, ...) \
	P_REFL_GET_3RD_ARG((type, __VA_ARGS__, TYPE_FLAGS, TYPE_NO_FLAGS, TYPE_INVALID))

#define P_PROP_CHOOSER(PROPERTY_NO_FLAGS, PROPERTY_FLAGS, name, ...) \
	P_REFL_GET_3RD_ARG((name, __VA_ARGS__, PROPERTY_FLAGS, PROPERTY_NO_FLAGS, PROPERTY_INVALID))


#define P_CLASS(type, ...)                                                           \
	P_TYPE_CHOOSER(P_CLASS_HEADER_NO_FLAGS, P_CLASS_HEADER_FLAGS, type, __VA_ARGS__) \
	(type, __VA_ARGS__) P_REFLECTION_BODY({})

#define P_STRUCT(type, ...)                                                            \
	P_TYPE_CHOOSER(P_STRUCT_HEADER_NO_FLAGS, P_STRUCT_HEADER_FLAGS, type, __VA_ARGS__) \
	(type, __VA_ARGS__) P_REFLECTION_BODY({})

#define P_PROP(name, ...) \
	P_PROP_CHOOSER(P_PROPERTY_NO_FLAGS, P_PROPERTY_FLAGS, name, __VA_ARGS__)(name, __VA_ARGS__)
#pragma endregion Macros


#pragma region PipeTypesSupport
P_NATIVE_NAMED(p::u8, "u8")
P_NATIVE_NAMED(p::i8, "i8")
P_NATIVE_NAMED(p::u16, "u16")
P_NATIVE_NAMED(p::i16, "i16")
P_NATIVE_NAMED(p::u32, "u32")
P_NATIVE_NAMED(p::i32, "i32")
P_NATIVE_NAMED(p::u64, "u64")
P_NATIVE_NAMED(p::i64, "i64")
P_NATIVE(float)
P_NATIVE(double)
P_NATIVE(char)
P_NATIVE_NAMED(char*, "StringLiteral")
P_NATIVE_NAMED(p::StringView, "StringView")
P_NATIVE_NAMED(p::String, "String")
P_NATIVE_NAMED(p::Path, "Path");
P_NATIVE(p::Tag)
P_NATIVE(p::Guid)

P_NATIVE_NAMED(p::v2, "v2");
P_NATIVE_NAMED(p::v2_u32, "v2_u32");
P_NATIVE_NAMED(p::v2_i32, "v2_i32");
P_NATIVE_NAMED(p::v3, "v3");
P_NATIVE_NAMED(p::v3_u32, "v3_u32");
P_NATIVE_NAMED(p::v3_i32, "v3_i32");
P_NATIVE(p::Quat);

P_NATIVE_NAMED(p::LinearColor, "LinearColor")
P_NATIVE_NAMED(p::sRGBColor, "sRGBColor")
P_NATIVE_NAMED(p::HSVColor, "HSVColor")
P_NATIVE_NAMED(p::Color, "Color")

// Build array types
namespace p
{
	template<typename T>
	inline void BuildType(const T*) requires(p::IsArray<T>())
	{
		// clang-format off
	static p::ContainerTypeOps ops{
		.itemType = p::RegisterTypeId<typename T::ItemType>(),
		.getData = [](void* data) {
			return (void*)static_cast<T*>(data)->Data();
		},
	    .getSize = [](void* data) {
			return static_cast<T*>(data)->Size();
	    },
	    .getItem = [](void* data, p::i32 index) {
			return (void*)(static_cast<T*>(data)->Data() + index);
	    },
	    .addItem = [](void* data, void* item) {
			if (item)
			{
				auto& itemRef = *static_cast<typename T::ItemType*>(item);
				if constexpr (p::IsCopyAssignable<typename T::ItemType>)
				{
					static_cast<T*>(data)->Add(itemRef);
				}
				else
				{
					static_cast<T*>(data)->Add(p::Move(itemRef));
				}
			}
			else
			{
				static_cast<T*>(data)->Add();
			}
	    },
	    .removeItem = [](void* data, p::i32 index) {
			static_cast<T*>(data)->RemoveAt(index);
	    },
	    .clear = [](void* data) {
		    static_cast<T*>(data)->Clear();
	    }
	};
		// clang-format on
		p::AddTypeFlags(p::TF_Container);
		p::SetTypeOps(&ops);
	};
}    // namespace p
#pragma endregion PipeTypesSupport


namespace p
{
#pragma region Objects
	class PIPE_API BaseObject
	{
	protected:
		BaseObject() = default;

	public:
		virtual ~BaseObject() = default;

		TypeId GetTypeId() const;
		TPtr<Object> AsPtr() const;
	};


	// For shared export purposes, we separate pointers from the exported Class
	struct PIPE_API ObjectOwnership
	{
		TPtr<BaseObject> self;
		TPtr<BaseObject> owner;
		static TPtr<BaseObject> nextOwner;


		ObjectOwnership();
		const TPtr<BaseObject>& AsPtr() const;
		const TPtr<BaseObject>& GetOwner() const;
	};


	template<typename T>
	struct TObjectPtrBuilder : public TPtrBuilder<T>
	{
		template<typename... Args>
		static T* New(Arena& arena, Args&&... args, const TPtr<BaseObject>& owner = {})
		{
			// Sets owner during construction
			// TODO: Fix self not existing at the moment of construction
			ObjectOwnership::nextOwner = owner;
			return new (p::Alloc<T>(arena)) T(std::forward<Args>(args)...);
		}

		// Allow creation of classes using reflection
		static T* New(Arena& arena, TypeId type, TPtr<BaseObject> owner = {})
		{
			if (auto* ops = GetTypeObjectOps(type))
			{
				// Sets owner during construction
				// TODO: Fix self not existing at the moment of construction
				ObjectOwnership::nextOwner = owner;
				if (T* instance = dynamic_cast<T*>(ops->onNew(arena)))
				{
					return instance;
				}
			}
			return nullptr;
		}

		static void Delete(Arena& arena, void* rawPtr)
		{
			T* ptr               = static_cast<T*>(rawPtr);
			const sizet typeSize = GetTypeSize(ptr->GetTypeId());
			ptr->~T();
			arena.Free((void*)ptr, typeSize);    // size depends on inheritance!
		}
	};


	class PIPE_API Object : public BaseObject
	{
	public:
		using Self = Object;
		template<typename T>
		using PtrBuilder = TObjectPtrBuilder<T>;

		virtual p::TypeId GetTypeId() const
		{
			return p::GetTypeId<Object>();
		}

		static constexpr p::TypeFlags staticTypeFlags = TF_None;

		P_REFLECTION_BODY({})

	private:
		ObjectOwnership ownership;


	public:
		Object() = default;

		void ChangeOwner(const TPtr<BaseObject>& inOwner);
		template<typename T = Object>
		TPtr<T> AsPtr() const
		{
			return Cast<T>(ownership.AsPtr());
		}
		template<typename T = Object>
		TPtr<T> GetOwner() const
		{
			return Cast<T>(ownership.GetOwner());
		}
	};
#pragma endregion Objects


#pragma region Casts

	// Equal and Down-casting
	template<typename To, typename From, typename ToValue = std::remove_pointer_t<To>>
	ToValue* Cast(From* value) requires(Derived<From, ToValue>)
	{
		return value;
	}

	// Up-casting
	template<typename To, typename From, typename ToValue = std::remove_pointer_t<To>>
	ToValue* Cast(From* value) requires(Derived<ToValue, From, false>)
	{
		if constexpr (Derived<From, Object>)
		{
			const TypeId toId   = GetTypeId<ToValue>();
			const TypeId fromId = value->GetTypeId();
			return IsTypeParentOf(toId, fromId) ? static_cast<ToValue*>(value) : nullptr;
		}
		return nullptr;    // TODO: Implement non-object up casting
	}

	template<typename To, typename From>
	TPtr<To> Cast(const TPtr<From>& value)
	{
		if (value.IsValid() && (Convertible<From, To> || Cast<To*>(value.GetUnsafe()) != nullptr))
		{
			TPtr<To> ptr{};
			ptr.CopyFromUnsafe(value);
			return ptr;
		}
		return {};
	}

	template<typename From, typename To = From>
	TPtr<To> Cast(const TOwnPtr<From>& value)
	{
		if constexpr (Derived<From, To>)    // Is T2 is T or its base
		{
			return TPtr<To>{*this};
		}

		TPtr<From> ptr{*this};
		return Cast<To>(ptr);
	}
#pragma endregion Casts
};    // namespace p
