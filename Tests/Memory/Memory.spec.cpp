// Copyright 2015-2023 Piperift - All rights reserved

#include <bandit/bandit.h>
#include <Pipe/Memory/Memory.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;

template<u32 Size>
struct TypeOfSize
{
	static constexpr u32 size = Size;
	u8 data[size]{1};    // Fill data for debugging
};

struct ComplexWithBools
{
	bool value1 = true;
	bool value2 = false;
};

struct Complex
{
	float value;

	Complex(float value = 0.f) : value(value) {}
};

go_bandit([]() {
	describe("Memory.Operations", []() {
		it("Can default construct", [&]() {
			// Check that it inits to 0
			bool boolValues[2]{true, true};    // Assign simulated garbage
			ConstructItems(boolValues, 2);
			AssertThat(boolValues[0], Is().EqualTo(false));
			AssertThat(boolValues[1], Is().EqualTo(false));

			u8 u8Values[2]{34, 45};    // Assign simulated garbage
			ConstructItems(u8Values, 2, u8(128));
			AssertThat(u8Values[0], Is().EqualTo(128));
			AssertThat(u8Values[1], Is().EqualTo(128));

			u32 u32Values[2]{34, 45};    // Assign simulated garbage
			ConstructItems(u32Values, 2);
			AssertThat(u32Values[0], Is().EqualTo(0));
			AssertThat(u32Values[1], Is().EqualTo(0));

			u32* ptrValues[2]{(u32*)1, (u32*)2};    // Assign simulated garbage
			ConstructItems(ptrValues, 2);
			AssertThat(ptrValues[0], Is().EqualTo(nullptr));
			AssertThat(ptrValues[1], Is().EqualTo(nullptr));

			Complex complexValues[2];
			complexValues[0].value = 0.234f;    // Assign simulated garbage
			complexValues[1].value = 0.234f;
			ConstructItems(complexValues, 2);
			AssertThat(complexValues[0].value, Is().EqualTo(0.f));
			AssertThat(complexValues[1].value, Is().EqualTo(0.f));

			ComplexWithBools complex2Values[2];
			complex2Values[0].value1 = false;    // Assign simulated garbage
			complex2Values[0].value2 = true;
			complex2Values[1].value1 = false;
			complex2Values[1].value2 = false;
			ConstructItems(complex2Values, 2);
			AssertThat(complex2Values[0].value1, Is().EqualTo(true));
			AssertThat(complex2Values[0].value2, Is().EqualTo(false));
			AssertThat(complex2Values[1].value1, Is().EqualTo(true));
			AssertThat(complex2Values[1].value2, Is().EqualTo(false));
		});

		it("Can value construct", [&]() {
			bool boolValues[2]{false, false};    // Assign simulated garbage
			ConstructItems(boolValues, 2, true);
			AssertThat(boolValues[0], Is().EqualTo(true));
			AssertThat(boolValues[1], Is().EqualTo(true));

			u8 u8Values[2]{34, 45};    // Assign simulated garbage
			ConstructItems(u8Values, 2, u8(128));
			AssertThat(u8Values[0], Is().EqualTo(128));
			AssertThat(u8Values[1], Is().EqualTo(128));

			u32 u32Values[2]{34, 45};    // Assign simulated garbage
			ConstructItems(u32Values, 2, u32(128));
			AssertThat(u32Values[0], Is().EqualTo(128));
			AssertThat(u32Values[1], Is().EqualTo(128));

			u32* ptrValues[2]{(u32*)1, (u32*)2};    // Assign simulated garbage
			ConstructItems(ptrValues, 2, (u32*)32);
			AssertThat(ptrValues[0], Is().EqualTo((u32*)32));
			AssertThat(ptrValues[1], Is().EqualTo((u32*)32));

			Complex complexValues[2]{{0.234f}, {0.234f}};    // Assign simulated garbage
			ConstructItems(complexValues, 2, Complex(1.f));
			AssertThat(complexValues[0].value, Is().EqualTo(1.f));
			AssertThat(complexValues[1].value, Is().EqualTo(1.f));

			ComplexWithBools complex2Values[2]{
			    {.value1 = false, .value2 = true},
			    {.value1 = false, .value2 = true}
            };    // Assign simulated garbage
			ConstructItems(complex2Values, 2, ComplexWithBools{.value1 = true, .value2 = true});
			AssertThat(complex2Values[0].value1, Is().EqualTo(true));
			AssertThat(complex2Values[0].value2, Is().EqualTo(true));
			AssertThat(complex2Values[1].value1, Is().EqualTo(true));
			AssertThat(complex2Values[1].value2, Is().EqualTo(true));
		});

		it("Can copy construct", [&]() {
			bool boolValues[2]{false, false};    // Assign simulated garbage
			bool srcBoolValues[2]{true, false};
			CopyConstructItems(boolValues, 2, srcBoolValues);
			AssertThat(boolValues[0], Is().EqualTo(true));
			AssertThat(boolValues[1], Is().EqualTo(false));

			u8 u8Values[2]{34, 45};    // Assign simulated garbage
			u8 srcU8Values[2]{128, 129};
			CopyConstructItems(u8Values, 2, srcU8Values);
			AssertThat(u8Values[0], Is().EqualTo(128));
			AssertThat(u8Values[1], Is().EqualTo(129));

			u32 u32Values[2]{34, 45};    // Assign simulated garbage
			u32 srcU32Values[2]{128, 129};
			CopyConstructItems(u32Values, 2, srcU32Values);
			AssertThat(u32Values[0], Is().EqualTo(128));
			AssertThat(u32Values[1], Is().EqualTo(129));

			u32* ptrValues[2]{(u32*)1, (u32*)2};    // Assign simulated garbage
			u32* srcPtrValues[2]{(u32*)34, (u32*)23433};
			CopyConstructItems(ptrValues, 2, srcPtrValues);
			AssertThat(ptrValues[0], Is().EqualTo((u32*)34));
			AssertThat(ptrValues[1], Is().EqualTo((u32*)23433));

			Complex complexValues[2]{{0.234f}, {0.234f}};    // Assign simulated garbage
			Complex srcComplexValues[2]{{1.f}, {2.f}};
			CopyConstructItems(complexValues, 2, srcComplexValues);
			AssertThat(complexValues[0].value, Is().EqualTo(1.f));
			AssertThat(complexValues[1].value, Is().EqualTo(2.f));

			ComplexWithBools complex2Values[2]{
			    {.value1 = false, .value2 = true},
			    {.value1 = false, .value2 = true}
            };    // Assign simulated garbage
			ComplexWithBools srcComplex2Values[2]{
			    {.value1 = true,  .value2 = false},
                {.value1 = false, .value2 = true }
            };
			CopyConstructItems(complex2Values, 2, srcComplex2Values);
			AssertThat(complex2Values[0].value1, Is().EqualTo(true));
			AssertThat(complex2Values[0].value2, Is().EqualTo(false));
			AssertThat(complex2Values[1].value1, Is().EqualTo(false));
			AssertThat(complex2Values[1].value2, Is().EqualTo(true));
		});
	});
});
