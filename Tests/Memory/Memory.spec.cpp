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

struct BoolsType
{
	bool value1 = true;
	bool value2 = false;
};

struct ConstructedType
{
	float value;

	ConstructedType(float value = 0.f) : value(value) {}
};

struct CopyType
{
	i32 value = 0;

	CopyType(i32 value) : value(value) {}
	CopyType(const CopyType& other)
	{
		value = other.value;
	}
	CopyType& operator=(const CopyType& other)
	{
		value = other.value;
		return *this;
	}
};

struct MoveType
{
	i32 value = 0;

	MoveType(i32 value) : value(value) {}
	MoveType(MoveType&& other) noexcept
	{
		value = Exchange(other.value, 0);
	}
	MoveType& operator=(MoveType&& other) noexcept
	{
		value = Exchange(other.value, 0);
		return *this;
	}
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

			ConstructedType constructedValues[2];
			constructedValues[0].value = 0.234f;    // Assign simulated garbage
			constructedValues[1].value = 0.234f;
			ConstructItems(constructedValues, 2);
			AssertThat(constructedValues[0].value, Is().EqualTo(0.f));
			AssertThat(constructedValues[1].value, Is().EqualTo(0.f));

			BoolsType boolsValues[2];
			boolsValues[0].value1 = false;    // Assign simulated garbage
			boolsValues[0].value2 = true;
			boolsValues[1].value1 = false;
			boolsValues[1].value2 = false;
			ConstructItems(boolsValues, 2);
			AssertThat(boolsValues[0].value1, Is().EqualTo(true));
			AssertThat(boolsValues[0].value2, Is().EqualTo(false));
			AssertThat(boolsValues[1].value1, Is().EqualTo(true));
			AssertThat(boolsValues[1].value2, Is().EqualTo(false));
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

			ConstructedType constructedValues[2]{{0.234f}, {0.234f}};    // Assign simulated garbage
			ConstructItems(constructedValues, 2, ConstructedType(1.f));
			AssertThat(constructedValues[0].value, Is().EqualTo(1.f));
			AssertThat(constructedValues[1].value, Is().EqualTo(1.f));

			BoolsType boolsValues[2]{
			    {.value1 = false, .value2 = true},
			    {.value1 = false, .value2 = true}
            };    // Assign simulated garbage
			ConstructItems(boolsValues, 2, BoolsType{.value1 = true, .value2 = true});
			AssertThat(boolsValues[0].value1, Is().EqualTo(true));
			AssertThat(boolsValues[0].value2, Is().EqualTo(true));
			AssertThat(boolsValues[1].value1, Is().EqualTo(true));
			AssertThat(boolsValues[1].value2, Is().EqualTo(true));
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

			ConstructedType constructedValues[2]{{0.234f}, {0.234f}};    // Assign simulated garbage
			ConstructedType srcConstructedValues[2]{{1.f}, {2.f}};
			CopyConstructItems(constructedValues, 2, srcConstructedValues);
			AssertThat(constructedValues[0].value, Is().EqualTo(1.f));
			AssertThat(constructedValues[1].value, Is().EqualTo(2.f));

			BoolsType boolsValues[2]{
			    {false, true},
                {false, true}
            };    // Assign simulated garbage
			BoolsType srcBoolsValues[2]{
			    {true,  false},
                {false, true }
            };
			CopyConstructItems(boolsValues, 2, srcBoolsValues);
			AssertThat(boolsValues[0].value1, Is().EqualTo(true));
			AssertThat(boolsValues[0].value2, Is().EqualTo(false));
			AssertThat(boolsValues[1].value1, Is().EqualTo(false));
			AssertThat(boolsValues[1].value2, Is().EqualTo(true));

			CopyType copyValues[2]{5, 6};    // Assign simulated garbage
			CopyType srcCopyValues[2]{34, 75};
			CopyConstructItems(copyValues, 2, srcCopyValues);
			AssertThat(copyValues[0].value, Is().EqualTo(34));
			AssertThat(copyValues[1].value, Is().EqualTo(75));
		});

		it("Can move construct", [&]() {
			bool boolValues[2]{false, false};    // Assign simulated garbage
			bool srcBoolValues[2]{true, false};
			MoveConstructItems(boolValues, 2, srcBoolValues);
			AssertThat(boolValues[0], Is().EqualTo(true));
			AssertThat(boolValues[1], Is().EqualTo(false));

			u8 u8Values[2]{34, 45};    // Assign simulated garbage
			u8 srcU8Values[2]{128, 129};
			MoveConstructItems(u8Values, 2, srcU8Values);
			AssertThat(u8Values[0], Is().EqualTo(128));
			AssertThat(u8Values[1], Is().EqualTo(129));

			u32 u32Values[2]{34, 45};    // Assign simulated garbage
			u32 srcU32Values[2]{128, 129};
			MoveConstructItems(u32Values, 2, srcU32Values);
			AssertThat(u32Values[0], Is().EqualTo(128));
			AssertThat(u32Values[1], Is().EqualTo(129));

			u32* ptrValues[2]{(u32*)1, (u32*)2};    // Assign simulated garbage
			u32* srcPtrValues[2]{(u32*)34, (u32*)23433};
			MoveConstructItems(ptrValues, 2, srcPtrValues);
			AssertThat(ptrValues[0], Is().EqualTo((u32*)34));
			AssertThat(ptrValues[1], Is().EqualTo((u32*)23433));

			ConstructedType constructedValues[2]{{0.234f}, {0.234f}};    // Assign simulated garbage
			ConstructedType srcConstructedTypeValues[2]{{1.f}, {2.f}};
			MoveConstructItems(constructedValues, 2, srcConstructedTypeValues);
			AssertThat(constructedValues[0].value, Is().EqualTo(1.f));
			AssertThat(constructedValues[1].value, Is().EqualTo(2.f));

			BoolsType boolsValues[2]{
			    {.value1 = false, .value2 = true},
			    {.value1 = false, .value2 = true}
            };    // Assign simulated garbage
			BoolsType srcConstructedType2Values[2]{
			    {.value1 = true,  .value2 = false},
                {.value1 = false, .value2 = true }
            };
			MoveConstructItems(boolsValues, 2, srcConstructedType2Values);
			AssertThat(boolsValues[0].value1, Is().EqualTo(true));
			AssertThat(boolsValues[0].value2, Is().EqualTo(false));
			AssertThat(boolsValues[1].value1, Is().EqualTo(false));
			AssertThat(boolsValues[1].value2, Is().EqualTo(true));

			MoveType moveValues[2]{5, 6};    // Assign simulated garbage
			MoveType srcMoveValues[2]{34, 75};
			MoveConstructItems(moveValues, 2, srcMoveValues);
			AssertThat(moveValues[0].value, Is().EqualTo(34));
			AssertThat(moveValues[1].value, Is().EqualTo(75));
			AssertThat(srcMoveValues[0].value, Is().EqualTo(0));
			AssertThat(srcMoveValues[1].value, Is().EqualTo(0));
		});
	});
});
