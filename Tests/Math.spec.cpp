// Copyright 2015-2021 Piperift - All rights reserved

#include <Containers/Array.h>
#include <Math/Search.h>
#include <Platform/Platform.h>
#include <Templates/Greater.h>
#include <bandit/bandit.h>


using namespace snowhouse;
using namespace bandit;
using namespace Rift;


go_bandit([]() {
	describe("Math", []() {
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
	});
});