// Copyright 2015-2026 Piperift. All Rights Reserved.

#include <PipeTest.h>
#include <Pipe/Core/Limits.h>
#include <PipeAlgorithms.h>
#include <PipeContainers.h>
#include <PipePlatform.h>

#include <cmath>


using namespace p;


namespace
{
	TArray<i32> bottomUp{23, 34, 50, 100, 120};
	TArray<i32> topDown{120, 100, 50, 34, 23};
}    // namespace


namespace
{
// Auto-registers via static init (macro-free go_bandit equivalent).
const bool autoRegistered = []()
{
Spec("Math.Math", []()
{
	Describe("Binary Search", []()
	{
		It("LowerBound", [=]()
		{
			Expect(bottomUp.LowerBound(34)).ToEqual(1);
			Expect(bottomUp.LowerBound(100)).ToEqual(3);
			Expect(bottomUp.LowerBound(51)).ToEqual(3);


			Expect(topDown.LowerBound(34, TGreater<>())).ToEqual(3);
			Expect(topDown.LowerBound(100, TGreater<>())).ToEqual(1);
			Expect(topDown.LowerBound(51, TGreater<>())).ToEqual(2);
		});

		It("UpperBound", [=]()
		{
			Expect(bottomUp.UpperBound(34)).ToEqual(2);
			Expect(bottomUp.UpperBound(100)).ToEqual(4);

			Expect(topDown.UpperBound(34, TGreater<>())).ToEqual(4);
			Expect(topDown.UpperBound(100, TGreater<>())).ToEqual(2);
		});

		It("Can find equal", [=]()
		{
			Expect(bottomUp.FindSorted(0)).ToEqual(NO_INDEX);
			Expect(bottomUp.FindSorted(34)).ToEqual(1);
			Expect(bottomUp.FindSorted(33)).ToEqual(NO_INDEX);
			Expect(bottomUp.FindSorted(121)).ToEqual(NO_INDEX);

			Expect(topDown.FindSorted(34, TGreater<>())).ToEqual(3);
		});

		Describe("FindSortedMax", []()
		{
			Describe("Ordered by a < b", []()
			{
				TArray<i32> bottomUp{23, 34, 50, 50, 100, 120};

				It("Find first item", [=]()
				{
					auto i4 = bottomUp.FindSortedMax(23, false);
					Expect(i4).ToEqual(NO_INDEX);

					auto i5 = bottomUp.FindSortedMax(23, true);
					Expect(i5).ToEqual(0);

					auto i6 = bottomUp.FindSortedMax(22, true);
					Expect(i6).ToEqual(NO_INDEX);
				});

				It("Find any item", [=]()
				{
					auto i1 = bottomUp.FindSortedMax(34, true);
					Expect(i1).ToEqual(1);

					auto i2 = bottomUp.FindSortedMax(33, true);
					Expect(i2).ToEqual(0);

					auto i3 = bottomUp.FindSortedMax(34, false);
					Expect(i3).ToEqual(0);
				});

				It("Find last item", [=]()
				{
					auto i4 = bottomUp.FindSortedMax(120, false);
					Expect(i4).ToEqual(4);

					auto i5 = bottomUp.FindSortedMax(120, true);
					Expect(i5).ToEqual(5);

					auto i6 = bottomUp.FindSortedMax(121, true);
					Expect(i6).ToEqual(5);

					auto i7 = bottomUp.FindSortedMax(100, false);
					Expect(i7).ToEqual(3);
				});
			});

			Describe("Ordered by a > b", []()
			{
				TArray<i32> topDown{120, 100, 50, 50, 34, 23};

				It("Find first item", [=]()
				{
					auto i4 = topDown.FindSortedMax(120, true);
					Expect(i4).ToEqual(0);

					auto i5 = topDown.FindSortedMax(120, false);
					Expect(i5).ToEqual(1);

					auto i6 = topDown.FindSortedMax(121, true);
					Expect(i6).ToEqual(0);
				});

				It("Find any item", [=]()
				{
					auto i1 = topDown.FindSortedMax(34, true);
					Expect(i1).ToEqual(4);

					auto i2 = topDown.FindSortedMax(33, true);
					Expect(i2).ToEqual(5);

					auto i3 = topDown.FindSortedMax(34, false);
					Expect(i3).ToEqual(5);
				});

				It("Find last item", [=]()
				{
					auto i4 = topDown.FindSortedMax(23, false);
					Expect(i4).ToEqual(NO_INDEX);

					auto i5 = topDown.FindSortedMax(23, true);
					Expect(i5).ToEqual(5);

					auto i6 = topDown.FindSortedMax(22, true);
					Expect(i6).ToEqual(NO_INDEX);
				});
			});

			Describe("All same values", []()
			{
				TArray<i32> allEqual{10, 10, 10};

				It("Doesnt find smaller", [=]()
				{
					auto i1 = allEqual.FindSortedMax(9, false);
					Expect(i1).ToEqual(NO_INDEX);

					auto i2 = allEqual.FindSortedMax(10, false);
					Expect(i2).ToEqual(NO_INDEX);
				});

				It("Finds smaller", [=]()
				{
					auto i1 = allEqual.FindSortedMax(10, true);
					Expect(i1).ToEqual(0);

					auto i2 = allEqual.FindSortedMax(11, false);
					Expect(i2).ToEqual(0);
				});
			});
		});
		Describe("FindSortedMin", []()
		{
			Describe("Ordered by a < b", []()
			{
				TArray<i32> bottomUp{23, 34, 50, 50, 100, 120};

				It("Find first item", [=]()
				{
					auto i1 = bottomUp.FindSortedMin(23, true);
					Expect(i1).ToEqual(0);

					auto i2 = bottomUp.FindSortedMin(20, true);
					Expect(i2).ToEqual(0);

					auto i3 = bottomUp.FindSortedMin(23, false);
					Expect(i3).ToEqual(1);
				});

				It("Find any item", [=]()
				{
					auto i1 = bottomUp.FindSortedMin(33, false);
					Expect(i1).ToEqual(1);

					auto i2 = bottomUp.FindSortedMin(34, true);
					Expect(i2).ToEqual(1);

					auto i3 = bottomUp.FindSortedMin(34, false);
					Expect(i3).ToEqual(2);
				});

				It("Find last item", [=]()
				{
					auto i1 = bottomUp.FindSortedMin(100, false);
					Expect(i1).ToEqual(5);

					auto i2 = bottomUp.FindSortedMin(120, false);
					Expect(i2).ToEqual(NO_INDEX);

					auto i3 = bottomUp.FindSortedMin(120, true);
					Expect(i3).ToEqual(5);

					auto i4 = bottomUp.FindSortedMin(121, true);
					Expect(i4).ToEqual(NO_INDEX);
				});
			});

			Describe("Ordered by a > b", []()
			{
				TArray<i32> topDown{120, 100, 50, 50, 34, 23};

				It("Find first item", [=]()
				{
					auto i4 = topDown.FindSortedMin(120, true);
					Expect(i4).ToEqual(0);

					auto i5 = topDown.FindSortedMin(120, false);
					Expect(i5).ToEqual(NO_INDEX);

					auto i6 = topDown.FindSortedMin(121, true);
					Expect(i6).ToEqual(NO_INDEX);
				});

				It("Find any item", [=]()
				{
					auto i1 = topDown.FindSortedMin(34, true);
					Expect(i1).ToEqual(4);

					auto i2 = topDown.FindSortedMin(33, true);
					Expect(i2).ToEqual(4);

					auto i3 = topDown.FindSortedMin(34, false);
					Expect(i3).ToEqual(3);
				});

				It("Find last item", [=]()
				{
					auto i4 = topDown.FindSortedMin(23, false);
					Expect(i4).ToEqual(4);

					auto i5 = topDown.FindSortedMin(23, true);
					Expect(i5).ToEqual(5);

					auto i6 = topDown.FindSortedMin(22, true);
					Expect(i6).ToEqual(5);
				});
			});

			Describe("All same values", []()
			{
				TArray<i32> allEqual{10, 10, 10};

				It("Doesnt find bigger", [=]()
				{
					auto i1 = allEqual.FindSortedMin(11, false);
					Expect(i1).ToEqual(NO_INDEX);

					auto i2 = allEqual.FindSortedMin(10, false);
					Expect(i2).ToEqual(NO_INDEX);
				});

				It("Finds bigger", [=]()
				{
					auto i1 = allEqual.FindSortedMin(10, true);
					Expect(i1).ToEqual(0);

					auto i2 = allEqual.FindSortedMin(9, false);
					Expect(i2).ToEqual(0);
				});
			});
		});
	});

	It("Can check Infinite", [=]()
	{
		Expect(IsInf(0.0)).ToEqual(false);
		Expect(IsInf(-0.0)).ToEqual(false);
		Expect(IsInf(1.0)).ToEqual(false);
		Expect(IsInf(-1.0)).ToEqual(false);

		static constexpr double dInfinite = Limits<double>::Infinity();
		Expect(IsInf(dInfinite)).ToEqual(true);
		Expect(IsInf(-dInfinite)).ToEqual(true);
		Expect(IsPosInf(-dInfinite)).ToEqual(false);
		Expect(IsNegInf(dInfinite)).ToEqual(false);
		Expect(IsInf(Limits<double>::Max())).ToEqual(false);
		Expect(IsInf(Limits<double>::Lowest())).ToEqual(false);
		Expect(IsInf(double(bigNumber))).ToEqual(false);
	});

	It("Can check NAN", [=]()
	{
		Expect(IsNAN(0.0)).ToEqual(false);
		Expect(IsNAN(Limits<double>::QuietNaN())).ToEqual(true);
	});

	Describe("Roundings", []()
	{
		It("Can Floor", [=]()
		{
			Expect(Floor(0.0)).ToEqual(std::floor(0.0));
			Expect(Floor(-0.0)).ToEqual(std::floor(-0.0));
			Expect(Floor(4.2)).ToEqual(std::floor(4.2));
			Expect(Floor(4.5)).ToEqual(std::floor(4.5));
			Expect(Floor(4.7)).ToEqual(std::floor(4.7));
			Expect(Floor(5.0)).ToEqual(std::floor(5.0));
			Expect(Floor(-4.2)).ToEqual(std::floor(-4.2));
			Expect(Floor(-4.7)).ToEqual(std::floor(-4.7));
			Expect(Floor(-5.0)).ToEqual(std::floor(-5.0));
			Expect(Floor(99999999999999999.0 + 0.5)).ToEqual(99999999999999999.0);

			static constexpr double dInfinite = Limits<double>::Infinity();
			Expect(Floor(-dInfinite)).ToEqual(std::floor(-dInfinite));
			Expect(Floor(dInfinite)).ToEqual(std::floor(dInfinite));
			Expect(IsNAN(Floor(Limits<double>::QuietNaN()))).ToEqual(true);
		});
		It("Can Ceil", [=]()
		{
			Expect(Ceil(0.0)).ToEqual(std::ceil(0.0));
			Expect(Ceil(-0.0)).ToEqual(std::ceil(-0.0));
			Expect(Ceil(4.2)).ToEqual(std::ceil(4.2));
			Expect(Ceil(4.5)).ToEqual(std::ceil(4.5));
			Expect(Ceil(4.7)).ToEqual(std::ceil(4.7));
			Expect(Ceil(5.0)).ToEqual(std::ceil(5.0));
			Expect(Ceil(-4.2)).ToEqual(std::ceil(-4.2));
			Expect(Ceil(-4.7)).ToEqual(std::ceil(-4.7));
			Expect(Ceil(-5.0)).ToEqual(std::ceil(-5.0));
			Expect(Ceil(99999999999999999.0 - 0.5)).ToEqual(99999999999999999.0);

			static constexpr double dInfinite = Limits<double>::Infinity();
			Expect(Ceil(-dInfinite)).ToEqual(std::ceil(-dInfinite));
			Expect(Ceil(dInfinite)).ToEqual(std::ceil(dInfinite));
			Expect(IsNAN(Ceil(Limits<double>::QuietNaN()))).ToEqual(true);
		});

		It("Can Round", [=]()
		{
			Expect(Round(0.0)).ToEqual(std::round(0.0));
			Expect(Round(-0.0)).ToEqual(std::round(-0.0));
			Expect(Round(4.2)).ToEqual(std::round(4.2));
			Expect(Round(4.5)).ToEqual(std::round(4.5));
			Expect(Round(4.7)).ToEqual(std::round(4.7));
			Expect(Round(5.0)).ToEqual(std::round(5.0));
			Expect(Round(-4.2)).ToEqual(std::round(-4.2));
			Expect(Round(-4.7)).ToEqual(std::round(-4.7));
			Expect(Round(-5.0)).ToEqual(std::round(-5.0));
			Expect(Round(99999999999999999.0 - 0.4)).ToEqual(99999999999999999.0);

			static constexpr double dInfinite = Limits<double>::Infinity();
			Expect(Round(-dInfinite)).ToEqual(std::round(-dInfinite));
			Expect(Round(dInfinite)).ToEqual(std::round(dInfinite));
			Expect(IsNAN(Round(Limits<double>::QuietNaN()))).ToEqual(true);
		});
	});
});
return true;
}();
}    // namespace
