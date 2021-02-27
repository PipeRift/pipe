// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Memory/OwnPtr.h"
#include "ReflectionTags.h"
#include "Strings/Name.h"
#include "Strings/String.h"


namespace Rift
{
	class BaseObject;
	struct Struct;


	namespace Refl
	{
		class Type;
		struct PropertyHandle;

		/**
		 * Static information about a property
		 */
		class CORE_API Property
		{
		private:
			Type* typePtr;
			Name typeName;
			Name name;
			String displayName;
			ReflectionTags tags;


		public:
			Property()                = delete;
			Property(Property&&)      = delete;
			Property(const Property&) = delete;

		protected:
			Property(Type* typePtr, Name typeName, Name name, ReflectionTags tags)
			    : typePtr(typePtr)
			    , typeName(Move(typeName))
			    , name(Move(name))
			    , tags(tags)
			{
				SetDisplayName(name.ToString());
			}

		public:
			virtual ~Property() = default;

			const String& GetName() const
			{
				return name.ToString();
			}
			const String& GetDisplayName() const
			{
				return displayName;
			}

			bool HasTag(ReflectionTags tag) const
			{
				return HasAnyTags(tag);
			}
			bool HasAllTags(ReflectionTags inTags) const
			{
				return (tags & inTags) == inTags;
			}
			bool HasAnyTags(ReflectionTags inTags) const
			{
				return (tags & inTags) > 0;
			}

			Type* GetContainerType() const
			{
				return typePtr;
			}

			Name GetTypeName() const
			{
				return typeName;
			}

			virtual OwnPtr<PropertyHandle> CreateHandle(const Ptr<BaseObject>& instance) const = 0;
			// virtual std::shared_ptr<PropertyHandle> CreateHandle(BaseStruct* instance) const = 0;

		protected:
			static const Type* GetInstanceType(const Ptr<BaseObject>& instance);
			static const Type* GetInstanceType(BaseObject* instance);

		private:
			void SetDisplayName(const String& inDisplayName);
		};
	}    // namespace Refl
}    // namespace Rift
