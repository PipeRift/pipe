// Copyright 2015-2024 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Core/Limits.h>
#include <Pipe/Core/Platform.h>
#include <PipeAlgorithms.h>
#include <PipeArrays.h>

#include <cmath>


using namespace snowhouse;
using namespace bandit;
using namespace p;


go_bandit([]() {
	describe("Math.Math", []() {
		describe("Binary Search", []() {
			TArray<i32> bottomUp{23, 34, 50, 100, 120};
			TArray<i32> topDown{120, 100, 50, 34, 23};

			it("LowerBound", [&]() {
				AssertThat(bottomUp.LowerBound(34), Equals(1));
				AssertThat(bottomUp.LowerBound(100), Equals(3));
				AssertThat(bottomUp.LowerBound(51), Equals(3));


				AssertThat(topDown.LowerBound(34, TGreater<>()), Equals(3));
				AssertThat(topDown.LowerBound(100, TGreater<>()), Equals(1));
				AssertThat(topDown.LowerBound(51, TGreater<>()), Equals(2));
			});

			it("UpperBound", [&]() {
				AssertThat(bottomUp.UpperBound(34), Equals(2));
				AssertThat(bottomUp.UpperBound(100), Equals(4));

				AssertThat(topDown.UpperBound(34, TGreater<>()), Equals(4));
				AssertThat(topDown.UpperBound(100, TGreater<>()), Equals(2));
			});

			it("Can find equal", [&]() {
				AssertThat(bottomUp.FindSortedEqual(0), Equals(NO_INDEX));
				AssertThat(bottomUp.FindSortedEqual(34), Equals(1));
				AssertThat(bottomUp.FindSortedEqual(33), Equals(NO_INDEX));
				AssertThat(bottomUp.FindSortedEqual(121), Equals(NO_INDEX));

				AssertThat(topDown.FindSortedEqual(34, TGreater<>()), Equals(3));
			});

			describe("FindSortedMax", []() {
				describe("Ordered by a < b", []() {
					TArray<i32> bottomUp{23, 34, 50, 50, 100, 120};

					it("Find first item", [&]() {
						auto i4 = bottomUp.FindSortedMax(23, false);
						AssertThat(i4, Equals(NO_INDEX));

						auto i5 = bottomUp.FindSortedMax(23, true);
						AssertThat(i5, Equals(0));

						auto i6 = bottomUp.FindSortedMax(22, true);
						AssertThat(i6, Equals(NO_INDEX));
					});

					it("Find any item", [&]() {
						auto i1 = bottomUp.FindSortedMax(34, true);
						AssertThat(i1, Equals(1));

						auto i2 = bottomUp.FindSortedMax(33, true);
						AssertThat(i2, Equals(0));

						auto i3 = bottomUp.FindSortedMax(34, false);
						AssertThat(i3, Equals(0));
					});

					it("Find last item", [&]() {
						auto i4 = bottomUp.FindSortedMax(120, false);
						AssertThat(i4, Equals(4));

						auto i5 = bottomUp.FindSortedMax(120, true);
						AssertThat(i5, Equals(5));

						auto i6 = bottomUp.FindSortedMax(121, true);
						AssertThat(i6, Equals(5));

						auto i7 = bottomUp.FindSortedMax(100, false);
						AssertThat(i7, Equals(3));
					});
				});

				describe("Ordered by a > b", []() {
					TArray<i32> topDown{120, 100, 50, 50, 34, 23};

					it("Find first item", [&]() {
						auto i4 = topDown.FindSortedMax(120, true);
						AssertThat(i4, Equals(0));

						auto i5 = topDown.FindSortedMax(120, false);
						AssertThat(i5, Equals(1));

						auto i6 = topDown.FindSortedMax(121, true);
						AssertThat(i6, Equals(0));
					});

					it("Find any item", [&]() {
						auto i1 = topDown.FindSortedMax(34, true);
						AssertThat(i1, Equals(4));

						auto i2 = topDown.FindSortedMax(33, true);
						AssertThat(i2, Equals(5));

						auto i3 = topDown.FindSortedMax(34, false);
						AssertThat(i3, Equals(5));
					});

					it("Find last item", [&]() {
						auto i4 = topDown.FindSortedMax(23, false);
						AssertThat(i4, Equals(NO_INDEX));

						auto i5 = topDown.FindSortedMax(23, true);
						AssertThat(i5, Equals(5));

						auto i6 = topDown.FindSortedMax(22, true);
						AssertThat(i6, Equals(NO_INDEX));
					});
				});

				describe("All same values", []() {
					TArray<i32> allEqual{10, 10, 10};

					it("Doesnt find smaller", [&]() {
						auto i1 = allEqual.FindSortedMax(9, false);
						AssertThat(i1, Equals(NO_INDEX));

						auto i2 = allEqual.FindSortedMax(10, false);
						AssertThat(i2, Equals(NO_INDEX));
					});

					it("Finds smaller", [&]() {
						auto i1 = allEqual.FindSortedMax(10, true);
						AssertThat(i1, Equals(0));

						auto i2 = allEqual.FindSortedMax(11, false);
						AssertThat(i2, Equals(0));
					});
				});
			});
			describe("FindSortedMin", []() {
				describe("Ordered by a < b", []() {
					TArray<i32> bottomUp{23, 34, 50, 50, 100, 120};

					it("Find first item", [&]() {
						auto i1 = bottomUp.FindSortedMin(23, true);
						AssertThat(i1, Equals(0));

						auto i2 = bottomUp.FindSortedMin(20, true);
						AssertThat(i2, Equals(0));

						auto i3 = bottomUp.FindSortedMin(23, false);
						AssertThat(i3, Equals(1));
					});

					it("Find any item", [&]() {
						auto i1 = bottomUp.FindSortedMin(33, false);
						AssertThat(i1, Equals(1));

						auto i2 = bottomUp.FindSortedMin(34, true);
						AssertThat(i2, Equals(1));

						auto i3 = bottomUp.FindSortedMin(34, false);
						AssertThat(i3, Equals(2));
					});

					it("Find last item", [&]() {
						auto i1 = bottomUp.FindSortedMin(100, false);
						AssertThat(i1, Equals(5));

						auto i2 = bottomUp.FindSortedMin(120, false);
						AssertThat(i2, Equals(NO_INDEX));

						auto i3 = bottomUp.FindSortedMin(120, true);
						AssertThat(i3, Equals(5));

						auto i4 = bottomUp.FindSortedMin(121, true);
						AssertThat(i4, Equals(NO_INDEX));
					});
				});

				describe("Ordered by a > b", [&]() {
					TArray<i32> topDown{120, 100, 50, 50, 34, 23};

					it("Find first item", [&]() {
						auto i4 = topDown.FindSortedMin(120, true);
						AssertThat(i4, Equals(0));

						auto i5 = topDown.FindSortedMin(120, false);
						AssertThat(i5, Equals(NO_INDEX));

						auto i6 = topDown.FindSortedMin(121, true);
						AssertThat(i6, Equals(NO_INDEX));
					});

					it("Find any item", [&]() {
						auto i1 = topDown.FindSortedMin(34, true);
						AssertThat(i1, Equals(4));

						auto i2 = topDown.FindSortedMin(33, true);
						AssertThat(i2, Equals(4));

						auto i3 = topDown.FindSortedMin(34, false);
						AssertThat(i3, Equals(3));
					});

					it("Find last item", [&]() {
						auto i4 = topDown.FindSortedMin(23, false);
						AssertThat(i4, Equals(4));

						auto i5 = topDown.FindSortedMin(23, true);
						AssertThat(i5, Equals(5));

						auto i6 = topDown.FindSortedMin(22, true);
						AssertThat(i6, Equals(5));
					});
				});

				describe("All same values", []() {
					TArray<i32> allEqual{10, 10, 10};

					it("Doesnt find bigger", [&]() {
						auto i1 = allEqual.FindSortedMin(11, false);
						AssertThat(i1, Equals(NO_INDEX));

						auto i2 = allEqual.FindSortedMin(10, false);
						AssertThat(i2, Equals(NO_INDEX));
					});

					it("Finds bigger", [&]() {
						auto i1 = allEqual.FindSortedMin(10, true);
						AssertThat(i1, Equals(0));

						auto i2 = allEqual.FindSortedMin(9, false);
						AssertThat(i2, Equals(0));
					});
				});
			});
		});

		it("Can check Infinite", [&]() {
			AssertThat(IsInf(0.0), Equals(false));
			AssertThat(IsInf(-0.0), Equals(false));
			AssertThat(IsInf(1.0), Equals(false));
			AssertThat(IsInf(-1.0), Equals(false));

			static constexpr double dInfinite = Limits<double>::Infinity();
			AssertThat(IsInf(dInfinite), Equals(true));
			AssertThat(IsInf(-dInfinite), Equals(true));
			AssertThat(IsPosInf(-dInfinite), Equals(false));
			AssertThat(IsNegInf(dInfinite), Equals(false));
			AssertThat(IsInf(Limits<double>::Max()), Equals(false));
			AssertThat(IsInf(Limits<double>::Lowest()), Equals(false));
			AssertThat(IsInf(double(bigNumber)), Equals(false));
		});

		it("Can check NAN", [&]() {
			AssertThat(IsNAN(0.0), Equals(false));
			AssertThat(IsNAN(Limits<double>::QuietNaN()), Equals(true));
		});

		describe("Roundings", []() {
			it("Can Floor", [&]() {
				AssertThat(Floor(0.0), Equals(std::floor(0.0)));
				AssertThat(Floor(-0.0), Equals(std::floor(-0.0)));
				AssertThat(Floor(4.2), Equals(std::floor(4.2)));
				AssertThat(Floor(4.5), Equals(std::floor(4.5)));
				AssertThat(Floor(4.7), Equals(std::floor(4.7)));
				AssertThat(Floor(5.0), Equals(std::floor(5.0)));
				AssertThat(Floor(-4.2), Equals(std::floor(-4.2)));
				AssertThat(Floor(-4.7), Equals(std::floor(-4.7)));
				AssertThat(Floor(-5.0), Equals(std::floor(-5.0)));
				AssertThat(Floor(99999999999999999.0 + 0.5), Equals(99999999999999999.0));

				static constexpr double dInfinite = Limits<double>::Infinity();
				AssertThat(Floor(-dInfinite), Equals(std::floor(-dInfinite)));
				AssertThat(Floor(dInfinite), Equals(std::floor(dInfinite)));
				AssertThat(IsNAN(Floor(Limits<double>::QuietNaN())), Equals(true));
			});
			it("Can Ceil", [&]() {
				AssertThat(Ceil(0.0), Equals(std::ceil(0.0)));
				AssertThat(Ceil(-0.0), Equals(std::ceil(-0.0)));
				AssertThat(Ceil(4.2), Equals(std::ceil(4.2)));
				AssertThat(Ceil(4.5), Equals(std::ceil(4.5)));
				AssertThat(Ceil(4.7), Equals(std::ceil(4.7)));
				AssertThat(Ceil(5.0), Equals(std::ceil(5.0)));
				AssertThat(Ceil(-4.2), Equals(std::ceil(-4.2)));
				AssertThat(Ceil(-4.7), Equals(std::ceil(-4.7)));
				AssertThat(Ceil(-5.0), Equals(std::ceil(-5.0)));
				AssertThat(Ceil(99999999999999999.0 - 0.5), Equals(99999999999999999.0));

				static constexpr double dInfinite = Limits<double>::Infinity();
				AssertThat(Ceil(-dInfinite), Equals(std::ceil(-dInfinite)));
				AssertThat(Ceil(dInfinite), Equals(std::ceil(dInfinite)));
				AssertThat(IsNAN(Ceil(Limits<double>::QuietNaN())), Equals(true));
			});

			it("Can Round", [&]() {
				AssertThat(Round(0.0), Equals(std::round(0.0)));
				AssertThat(Round(-0.0), Equals(std::round(-0.0)));
				AssertThat(Round(4.2), Equals(std::round(4.2)));
				AssertThat(Round(4.5), Equals(std::round(4.5)));
				AssertThat(Round(4.7), Equals(std::round(4.7)));
				AssertThat(Round(5.0), Equals(std::round(5.0)));
				AssertThat(Round(-4.2), Equals(std::round(-4.2)));
				AssertThat(Round(-4.7), Equals(std::round(-4.7)));
				AssertThat(Round(-5.0), Equals(std::round(-5.0)));
				AssertThat(Round(99999999999999999.0 - 0.4), Equals(99999999999999999.0));

				static constexpr double dInfinite = Limits<double>::Infinity();
				AssertThat(Round(-dInfinite), Equals(std::round(-dInfinite)));
				AssertThat(Round(dInfinite), Equals(std::round(dInfinite)));
				AssertThat(IsNAN(Round(Limits<double>::QuietNaN())), Equals(true));
			});
		});
	});
});
