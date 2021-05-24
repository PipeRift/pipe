// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Math.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Reflection/Static/NativeType.h"
#include "Reflection/TypeFlags.h"
#include "Rotator.h"
#include "Serialization/ContextsFwd.h"
#include "Strings/StringView.h"
#include "Vector.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/transform.hpp>


namespace Rift
{
	struct Transform
	{
		v3 location   = v3::Zero();
		Quat rotation = Quat::Identity();
		v3 scale      = v3::One();


		Transform() = default;
		Transform(v3 location) : location(location){};


		Rotator GetRotation()
		{
			return rotation.ToRotator();
		}
		void SetRotation(const Rotator& angles)
		{
			rotation = Quat::FromRotator(angles);
		}

		void Read(Serl::ReadContext& ct);
		void Write(Serl::WriteContext& ct) const;


		Matrix4f ToMatrix() const
		{
			// Scale -> Rotate -> Translate
			return glm::translate(location) * rotation.ToMatrix() * glm::scale(scale);
		}

		Matrix4f ToMatrixNoScale() const
		{
			// Rotate -> Translate
			return glm::translate(location) * glm::mat4_cast(rotation);
		}

		Matrix4f ToInverseMatrix() const
		{
			return ToMatrix().Inverse();
		}

		Transform Inverse()
		{
			Transform t{};
			t.SetFromMatrix(ToInverseMatrix());
			return t;
		}

		v3 TransformLocation(const v3& p) const
		{
			return rotation.Rotate(p * scale) + location;
		}
		v3 TransformVector(const v3& vector) const
		{
			return rotation.Rotate(vector * scale);
		}
		Quat TransformRotation(const Quat& q) const
		{
			return rotation * q;
		}
		v3 InverseTransformLocation(const v3& p) const
		{
			return rotation.Unrotate(p - location) * Math::GetSafeScaleReciprocal(scale);
		}
		v3 InverseTransformVector(const v3& vector) const
		{
			return rotation.Unrotate(vector) * Math::GetSafeScaleReciprocal(scale);
		}
		Quat InverseTransformRotation(const Quat& q) const
		{
			return rotation.Inverse() * q;
		}

		v3 GetForward() const
		{
			return rotation.GetForward();
		}


		void SetFromMatrix(const Matrix4f& m)
		{
			v3 skew;
			v4 perpective;
			glm::decompose(m, scale, rotation, location, skew, perpective);
		}

		Transform operator*(const Transform& other) const
		{
			Transform t;
			t.SetFromMatrix(ToMatrix() * other.ToMatrix());
			return t;
		}
	};

	DEFINE_TYPE_FLAGS(Transform, HasMemberSerialize = true);

	REFLECT_NATIVE_TYPE(Transform);
}    // namespace Rift
