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
		class CORE_API Property
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
			Property(DataType* owner, Type* type, Name name, Access access, ReflectionTags tags)
			    : owner(owner)
			    , type(type)
			    , name(name)
			    , access(Move(access))
			    , tags(tags)
			{}

		public:
			virtual ~Property() = default;

			DataType* GetOwner() const;
			Type* GetType() const;
			Name GetName() const;
			void* GetDataPtr(void* instance);
			const String& GetDisplayName() const;
			bool HasTag(ReflectionTags tag) const;
			bool HasAllTags(ReflectionTags inTags) const;
			bool HasAnyTags(ReflectionTags inTags) const;
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
