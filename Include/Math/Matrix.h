// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "TypeTraits.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/matrix.hpp>


namespace Rift
{
	template <u32 X, u32 Y, Number T>
	class Matrix : public glm::mat<X, Y, T, glm::highp>
	{
		union
		{
			T data[X][Y];
		};

	public:
		Matrix() {}

		Matrix Inverse() const
		{
			return glm::inverse(*this);
		}
		Matrix Transpose() const
		{
			return glm::transpose(*this);
		}
		Matrix InverseTranspose() const
		{
			return glm::inverseTranspose(*this);
		}

		T* Data()
		{
			return &(*this)[0].x;
		}
		const T* Data() const
		{
			return &(*this)[0].x;
		}

		static constexpr Matrix Identity()
		{
			return Matrix(1);
		}
	};

	using Matrix4f = Matrix<4, 4, float>;
}    // namespace Rift
