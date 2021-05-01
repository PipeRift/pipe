// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Memory/OwnPtr.h"
#include "Reflection/ReflectionTags.h"
#include "Strings/Name.h"
#include "Strings/String.h"


namespace Rift
{
	class BaseObject;
	struct Struct;


	namespace Refl
	{
		class DataType;

		/**
		 * Static information about a property
		 */
		class Property
		{
			friend class ReflectionRegistry;

			// Returns a pointer to the variable from an owner instance pointer
			using Access = TFunction<void*(void*)>;

		protected:
			DataType* owner;
			Type* type;
			Name name;

			Access access;
			ReflectionTags tags;
			String displayName;


		public:
			Property()                = delete;
			Property(Property&&)      = delete;
			Property(const Property&) = delete;

		protected:
			CORE_API Property(
			    DataType* owner, Type* type, Name name, Access access, ReflectionTags tags)
			    : owner(owner)
			    , type(type)
			    , name(name)
			    , access(Move(access))
			    , tags(tags)
			{}

		public:
			CORE_API virtual ~Property() = default;

			CORE_API DataType* GetOwner() const;
			CORE_API Type* GetType() const;
			CORE_API Name GetName() const;
			CORE_API void* GetDataPtr(void* instance);
			CORE_API const String& GetDisplayName() const;
			CORE_API bool HasTag(ReflectionTags tag) const;
			CORE_API bool HasAllTags(ReflectionTags inTags) const;
			CORE_API bool HasAnyTags(ReflectionTags inTags) const;
		};


		inline DataType* Property::GetOwner() const
		{
			return owner;
		}
		inline Type* Property::GetType() const
		{
			return type;
		}
		inline Name Property::GetName() const
		{
			return name;
		}
		inline void* Property::GetDataPtr(void* instance)
		{
			return access(instance);
		}
		inline const String& Property::GetDisplayName() const
		{
			return displayName;
		}
		inline bool Property::HasTag(ReflectionTags tag) const
		{
			return HasAnyTags(tag);
		}
		inline bool Property::HasAllTags(ReflectionTags inTags) const
		{
			return (tags & inTags) == inTags;
		}
		inline bool Property::HasAnyTags(ReflectionTags inTags) const
		{
			return (tags & inTags) > 0;
		}
	}    // namespace Refl
}    // namespace Rift
