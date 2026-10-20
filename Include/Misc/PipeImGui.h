// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

// ImGui helpers for Pipe
// Required ImGui (v1.90 or newer). Must be included before this header.

#ifndef IMGUI_VERSION_NUM
static_assert(false, "Imgui not found. PipeImGui requires v1.90 or newer.");
#elif IMGUI_VERSION_NUM < 19000
static_assert(false, "Imgui v" IMGUI_VERSION " found but PipeImGui requires v1.90 or newer.");
#endif


#include "Pipe/Core/Checks.h"
#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/Map.h"
#include "Pipe/Files/Paths.h"
#include "PipeColor.h"
#include "PipeContainers.h"
#include "PipeECS.h"
#include "PipeReflect.h"
#include "PipeStrings.h"
#include "PipeTime.h"

#include <imgui.h>
#include <imgui_internal.h>


namespace p
{
	///////////////////////////////////////////////////////////
	// Conversions

	inline ImVec2 ToIM(v2 value)
	{
		return {value.x, value.y};
	}
	inline v2 FromIM(ImVec2 value)
	{
		return {value.x, value.y};
	}
	inline ImVec4 ToIM(const v4& value)
	{
		return {value.x, value.y, value.z, value.w};
	}
	inline v4 FromIM(const ImVec4& value)
	{
		return {value.x, value.y, value.z, value.w};
	}
	inline ImColor ToIM(const LinearColor& value)
	{
		return {value.r, value.g, value.b, value.a};
	}
	inline ImColor ToIM(const Color& value)
	{
		return ToIM(value.Convert<p::ColorMode::Linear>());
	}
	inline LinearColor FromIMColor(const ImVec4& value)
	{
		return {value.x, value.y, value.z, value.w};
	}
};    // namespace p


namespace ImGui
{
	///////////////////////////////////////////////////////////
	// Internals

	namespace details
	{
		struct InputTextCallbackStringUserData
		{
			p::String* str;
			ImGuiInputTextCallback chainCallback;
			void* chainCallbackUserData;
		};

		inline int InputTextCallback(ImGuiInputTextCallbackData* data)
		{
			auto* userData = static_cast<InputTextCallbackStringUserData*>(data->UserData);
			if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
			{
				// Resize string callback
				// If for some reason we refuse the new length (BufTextLen) and/or capacity
				// (BufSize) we need to set them back to what we want.
				p::String* str = userData->str;
				IM_ASSERT(data->Buf == str->c_str());
				str->resize(data->BufTextLen);
				data->Buf = (char*)str->c_str();
			}
			else if (userData->chainCallback)
			{
				// Forward to user callback, if any
				data->UserData = userData->chainCallbackUserData;
				return userData->chainCallback(data);
			}
			return 0;
		}
	}    // namespace details


	///////////////////////////////////////////////////////////
	// Definitions

	inline void PushID(p::StringView id)
	{
		PushID(id.data(), id.data() + id.size());
	}
	template<p::Integral T>
	inline void PushID(T id)
	{
		PushID(reinterpret_cast<void*>(p::sizet(id)));
	}
	inline void PushID(p::Id id)
	{
		PushID(reinterpret_cast<void*>(p::sizet(id.value)));
	}
	inline ImGuiID GetID(p::StringView id)
	{
		return GetID(id.data(), id.data() + id.size());
	}

	inline void PushStyleColor(ImGuiCol idx, p::Color color)
	{
		PushStyleColor(idx, color.DWColor());
	}
	inline void PushStyleColor(ImGuiCol idx, const p::LinearColor& color)
	{
		PushStyleColor(idx, ImVec4{color.r, color.g, color.b, color.a});
	}
	inline void PushStyleVar(ImGuiStyleVar idx, p::v2 value)
	{
		PushStyleVar(idx, ImVec2{value.x, value.y});
	}

	inline void Text(p::StringView text)
	{
		TextUnformatted(text.data(), text.data() + text.size());
	}

	inline ImVec2 CalcTextSize(
	    p::StringView text, bool hide_text_after_double_hash = false, float wrap_width = 0.0f)
	{
		return CalcTextSize(
		    text.data(), text.data() + text.size(), hide_text_after_double_hash, wrap_width);
	}

	inline void TextDisabled(p::StringView text)
	{
		PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
		Text(text);
		PopStyleColor();
	}

	inline void TextColoredUnformatted(
	    const p::LinearColor& color, const char* text, const char* textEnd = nullptr)
	{
		PushStyleColor(ImGuiCol_Text, color);
		TextUnformatted(text, textEnd);
		PopStyleColor();
	}
	inline void TextColored(const p::LinearColor& color, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		TextColoredV(ImVec4{color.r, color.g, color.b, color.a}, fmt, args);
		va_end(args);
	}
	inline void TextColored(const p::LinearColor& color, p::StringView text)
	{
		TextColoredUnformatted(color, text.data(), text.data() + text.size());
	}

	inline bool TextLink(p::StringView label)
	{
		if (label.data()[label.size()] != '\0')
		{
			static p::String tmpLabel;
			tmpLabel = label;
			label    = tmpLabel;
		}
		return TextLink(label.data());
	}

	// Text-only button: The text/icon glyph is the only visible content, no background.
	inline bool TextButton(const char* text, const char* id = nullptr)
	{
		const ImVec2 padding   = GetStyle().FramePadding;
		const ImVec2 text_size = CalcTextSize(text);
		const ImVec2 size(text_size.x + padding.x * 2.0f, text_size.y + padding.y * 2.0f);

		PushID(id ? id : text);
		const bool clicked = InvisibleButton("textbtn", size);

		const bool disabled = (GetItemFlags() & ImGuiItemFlags_Disabled) != 0;
		const bool hovered  = IsItemHovered();
		const bool active   = IsItemActive();

		// Base text color, dimmed automatically via the disabled text color.
		const ImVec4& baseStyle =
		    GetStyle().Colors[disabled ? ImGuiCol_TextDisabled : ImGuiCol_Text];
		const p::LinearColor base{baseStyle.x, baseStyle.y, baseStyle.z, baseStyle.w};

		// Per-channel tint of the button background from idle to state.
		const ImVec4& btnStyle = GetStyle().Colors[ImGuiCol_Button];
		const p::LinearColor btn{btnStyle.x, btnStyle.y, btnStyle.z, btnStyle.w};

		p::LinearColor col = base;
		if (!disabled)
		{
			const ImVec4& stateStyle = active  ? GetStyle().Colors[ImGuiCol_ButtonActive]
			                         : hovered ? GetStyle().Colors[ImGuiCol_ButtonHovered]
			                                   : btnStyle;
			const p::LinearColor state{stateStyle.x, stateStyle.y, stateStyle.z, stateStyle.w};
			col = base + (state - btn);
		}

		PushStyleColor(ImGuiCol_Text, col);
		const ImVec2 rectMin = GetItemRectMin();
		RenderText({rectMin.x + padding.x, rectMin.y + padding.y}, text);
		PopStyleColor();
		PopID();
		return clicked;
	}


	// ImGui::InputText() with String
	// Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
	inline bool InputText(const char* label, p::String& str, ImGuiInputTextFlags flags = 0,
	    ImGuiInputTextCallback callback = nullptr, void* userData = nullptr)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		details::InputTextCallbackStringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return InputText(label, (char*)str.c_str(), str.capacity() + 1, flags,
		    details::InputTextCallback, &cbUserData);
	}

	inline bool InputTextMultiline(const char* label, p::String& str,
	    const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0,
	    ImGuiInputTextCallback callback = nullptr, void* userData = nullptr)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		details::InputTextCallbackStringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputTextMultiline(label, (char*)str.c_str(), str.capacity() + 1, size, flags,
		    details::InputTextCallback, &cbUserData);
	}

	inline bool InputTextWithHint(const char* label, const char* hint, p::String& str,
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
	    void* userData = nullptr)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		details::InputTextCallbackStringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return InputTextWithHint(label, hint, (char*)str.c_str(), str.capacity() + 1, flags,
		    details::InputTextCallback, &cbUserData);
	}

	inline bool MenuItem(p::StringView label, const char* shortcut = nullptr, bool selected = false,
	    bool enabled = true)
	{
		// If StringView doesn't end in \0, need to use a temporal
		if (label.data()[label.size()] != '\0')
		{
			static p::String tmpLabel;
			tmpLabel = label;
			label    = tmpLabel;
		}
		return MenuItem(label.data(), shortcut, selected, enabled);
	}

	inline bool MenuItem(
	    p::StringView label, const char* shortcut, bool* p_selected, bool enabled = true)
	{
		// If StringView doesn't end in \0, need to use a temporal
		if (label.data()[label.size()] != '\0')
		{
			static p::String tmpLabel;
			tmpLabel = label;
			label    = tmpLabel;
		}
		return MenuItem(label.data(), shortcut, p_selected, enabled);
	}

	inline bool Selectable(p::StringView label, bool selected = false,
	    ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0))
	{
		// If StringView doesn't end in \0, need to use a temporal
		if (label.data()[label.size()] != '\0')
		{
			static p::String tmpLabel;
			tmpLabel = label;
			label    = tmpLabel;
		}
		return Selectable(label.data(), selected, flags, size);
	}

	inline bool Selectable(p::StringView label, bool* p_selected, ImGuiSelectableFlags flags = 0,
	    const ImVec2& size = ImVec2(0, 0))
	{
		// If StringView doesn't end in \0, need to use a temporal
		if (label.data()[label.size()] != '\0')
		{
			static p::String tmpLabel;
			tmpLabel = label;
			label    = tmpLabel;
		}
		return Selectable(label.data(), p_selected, flags, size);
	}

	inline bool ColorButton(const char* desc_id, const p::LinearColor& col,
	    ImGuiColorEditFlags flags = 0, p::v2 size = {})
	{
		return ColorButton(
		    desc_id, ImVec4{col.r, col.g, col.b, col.a}, flags, ImVec2{size.x, size.y});
	}

	inline void SetTooltip(p::StringView text)
	{
		if (!BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None))
		{
			return;
		}
		Text(text);
		EndTooltip();
	}

	inline void SetItemTooltip(p::StringView text)
	{
		if (IsItemHovered(ImGuiHoveredFlags_ForTooltip))
		{
			SetTooltip(text);
		}
	}

	inline void HelpTooltip(p::StringView text,
	    ImGuiHoveredFlags flags = ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_NoSharedDelay)
	{
		if (IsItemHovered(flags))
		{
			PushStyleVar(ImGuiStyleVar_WindowPadding, p::v2{4.f, 3.f});
			BeginTooltip();
			PushTextWrapPos(GetFontSize() * 35.0f);
			AlignTextToFramePadding();
			TextUnformatted(text.data());
			PopTextWrapPos();
			EndTooltip();
			PopStyleVar();
		}
	}
	inline void HelpMarker(p::StringView text,
	    ImGuiHoveredFlags flags = ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_NoSharedDelay)
	{
		TextDisabled("(?)");
		HelpTooltip(text, flags);
	}

	inline bool DrawFilterWithHint(ImGuiTextFilter& filter, const char* label = "Filter (inc,-exc)",
	    const char* hint = "...", float width = 0.0f)
	{
		if (width != 0.0f)
		{
			SetNextItemWidth(width);
		}
		bool value_changed =
		    InputTextWithHint(label, hint, filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf));
		if (value_changed)
		{
			filter.Build();
		}
		return value_changed;
	}


#pragma region Style
	template<p::ColorMode mode>
	p::TColor<mode> ToHovered(const p::TColor<mode>& color)
	{
		return color.Shade(0.1f);
	}

	template<p::ColorMode mode>
	p::TColor<mode> ToDisabled(const p::TColor<mode>& color)
	{
		return color.Shade(0.2f);
	}

	inline void PushStyleCompact()
	{
		ImGuiStyle& style = GetStyle();
		PushStyleVar(ImGuiStyleVar_FramePadding,
		    ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
		PushStyleVar(ImGuiStyleVar_ItemSpacing,
		    ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
	}
	inline void PopStyleCompact()
	{
		PopStyleVar(2);
	}
	inline void PushFrameBgColor(p::LinearColor color)
	{
		PushStyleColor(ImGuiCol_FrameBg, color.Shade(0.3f));
		PushStyleColor(ImGuiCol_FrameBgHovered, ToHovered(color));
		PushStyleColor(ImGuiCol_FrameBgActive, color);
	}
	inline void PopFrameBgColor()
	{
		PopStyleColor(3);
	}
	inline void PushButtonColor(p::LinearColor color)
	{
		PushStyleColor(ImGuiCol_Button, color);
		PushStyleColor(ImGuiCol_ButtonHovered, ToHovered(color));
		PushStyleColor(ImGuiCol_ButtonActive, color.Tint(0.1f));
	}
	inline void PopButtonColor()
	{
		PopStyleColor(3);
	}
	inline void PushHeaderColor(p::LinearColor color)
	{
		PushStyleColor(ImGuiCol_Header, color);
		PushStyleColor(ImGuiCol_HeaderHovered, ToHovered(color));
		PushStyleColor(ImGuiCol_HeaderActive, color.Tint(0.1f));
	}
	inline void PopHeaderColor()
	{
		PopStyleColor(3);
	}
	inline void PushTextColor(p::LinearColor color)
	{
		PushStyleColor(ImGuiCol_Text, color);
		PushStyleColor(ImGuiCol_TextDisabled, color.Shade(0.15f));
	}
	inline void PopTextColor()
	{
		PopStyleColor(2);
	}
	inline p::LinearColor GetTextColor()
	{
		auto color = GetStyleColorVec4(ImGuiCol_TextDisabled);
		return {color.x, color.y, color.z, color.w};
	}
#pragma endregion Style


	///////////////////////////////////////////////////////////
	// Implementation
#ifdef P_IMGUI_IMPLEMENTATION
#endif
}    // namespace ImGui


namespace p
{
	///////////////////////////////////////////////////////////
	// Inspection
	//
	// Immediate mode reflection inspection.

	struct TypeInspection
	{
		using RowCallback =
		    TFunction<void(StringView label, void* data, TypeId typeId, bool& open)>;
		using ChildrenCallback = TFunction<void(void* data, TypeId typeId)>;

		RowCallback onDrawRow;
		ChildrenCallback onDrawChildren;
	};

	using InspectionFlags = u32;
	enum InspectionFlags_
	{
		IF_None    = 0,         // -> No flags
		IF_ViewAll = 1 << 0,    // -> View all properties, even those without PF_View or PF_Edit
		IF_EditAll = 1 << 1,    // -> Allow editing on all properties, even those without PF_Edit
		IF_ViewAndEditAll = IF_ViewAll | IF_EditAll

		// Any other flags up to 64 bytes are available to the user
	};
	P_DEFINE_FLAG_OPERATORS(InspectionFlags_)

	struct InspectionContext
	{
		struct PropStack
		{
			TypeId typeId;
			void* data = nullptr;
			i32 index  = NO_INDEX;
		};
		TMap<TypeId, TypeInspection> registeredTypes;
		TArray<PropStack> propStack;
		InspectionFlags flags   = IF_None;
		ImGuiTextFilter* filter = nullptr;
		// Optional Id registry used by Id related inspections
		IdContext* idContext = nullptr;
		bool initialized     = false;
	};

	namespace details
	{
		// We replace ImGui's GetCurrentWindow() in case "GImGui" has been overriden which can
		// cause link errors
		inline ImGuiWindow* GetCurrentWindow()
		{
			ImGuiContext& g                = *ImGui::GetCurrentContext();
			g.CurrentWindow->WriteAccessed = true;
			return g.CurrentWindow;
		}
	}    // namespace details


	// Type inspections are registered into the current inspection context
	void RegisterTypeInspection(TypeId typeId, TypeInspection::RowCallback onDrawRow,
	    TypeInspection::ChildrenCallback onDrawChildren = {});
	template<typename T>
	inline void RegisterTypeInspection(
	    TypeInspection::RowCallback onDrawRow, TypeInspection::ChildrenCallback onDrawChildren = {})
	{
		RegisterTypeInspection(GetTypeId<T>(), Move(onDrawRow), Move(onDrawChildren));
	}

	void RemoveTypeInspection(TypeId typeId);

	// Registers the inspections of Pipe's native types into the current context
	void RegisterPipeTypeInspections();


	bool BeginInspection(InspectionContext& ctx, const char* label, v2 size = v2{0.f, 0.f},
	    InspectionFlags flags = IF_None, ImGuiTextFilter* filter = nullptr,
	    IdContext* idContext = nullptr);
	void EndInspection();

	void InspectSetKeyColumn();
	void InspectSetKeyAsText(StringView label);
	void InspectSetValueColumn();
	bool InspectBeginCategory(p::StringView name, bool isLeaf, bool defaultOpen = false);
	void InspectEndCategory();
	void Inspect(StringView label, void* data, TypeId typeId);
	template<typename T>
	inline void Inspect(StringView label, T* data)
	{
		Inspect(label, data, GetTypeId<T>());
	}
	void InspectProperties(void* data, TypeId typeId);


	///////////////////////////////////////////////////////////
	// Implementation
#ifdef P_IMGUI_IMPLEMENTATION
	static InspectionContext* currentInspection = nullptr;


	#define EnsureInsideInspection     \
		P_EnsureMsg(currentInspection, \
		    "No inspection context available! Forgot to call BeginInspection()?")


	void RegisterTypeInspection(TypeId typeId, TypeInspection::RowCallback onDrawRow,
	    TypeInspection::ChildrenCallback onDrawChildren)
	{
		if (EnsureInsideInspection)
		{
			currentInspection->registeredTypes.Insert(
			    typeId, {Move(onDrawRow), Move(onDrawChildren)});
		}
	}

	void RemoveTypeInspection(TypeId typeId)
	{
		if (EnsureInsideInspection)
		{
			currentInspection->registeredTypes.Remove(typeId);
		}
	}

	#define P_DECLARE_COMMON_VALUE_TYPEINSPECTION(type, valueCode)      \
		p::RegisterTypeInspection<type>(                                \
		    [](StringView label, void* data, TypeId typeId, bool& open) \
		{                                                               \
			InspectSetKeyAsText(label);                                 \
			InspectSetValueColumn();                                    \
			type& value = *static_cast<type*>(data);                    \
			valueCode                                                   \
		})
	void RegisterPipeTypeInspections()
	{
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(bool, { ImGui::Checkbox("##value", &value); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(String, { ImGui::InputText("##value", value); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(StringView, {
			ImGui::InputText("##value", const_cast<char*>(value.data()), value.size(),
			    ImGuiInputTextFlags_ReadOnly);
		});
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(Tag, {
			String str{value.AsString()};
			if (ImGui::InputText("##value", str))
			{
				value = Tag{str};
			}
		});
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    i8, { ImGui::InputScalar("##value", ImGuiDataType_S8, &value); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    u8, { ImGui::InputScalar("##value", ImGuiDataType_U8, &value); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    i32, { ImGui::InputScalar("##value", ImGuiDataType_S32, &value); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    u32, { ImGui::InputScalar("##value", ImGuiDataType_U32, &value); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    i64, { ImGui::InputScalar("##value", ImGuiDataType_S64, &value); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    u64, { ImGui::InputScalar("##value", ImGuiDataType_U64, &value); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    float, { ImGui::InputScalar("##value", ImGuiDataType_Float, &value); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    double, { ImGui::InputScalar("##value", ImGuiDataType_Double, &value); });

		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v2_i8, { ImGui::InputScalarN("##value", ImGuiDataType_S8, &value, 2); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v2_u8, { ImGui::InputScalarN("##value", ImGuiDataType_U8, &value, 2); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v2_i32, { ImGui::InputScalarN("##value", ImGuiDataType_S32, &value, 2); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v2_u32, { ImGui::InputScalarN("##value", ImGuiDataType_U32, &value, 2); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v2_i64, { ImGui::InputScalarN("##value", ImGuiDataType_S64, &value, 2); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v2_u64, { ImGui::InputScalarN("##value", ImGuiDataType_U64, &value, 2); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v2, { ImGui::InputScalarN("##value", ImGuiDataType_Float, &value, 2); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v2d, { ImGui::InputScalarN("##value", ImGuiDataType_Double, &value, 2); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v3_i8, { ImGui::InputScalarN("##value", ImGuiDataType_S8, &value, 3); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v3_u8, { ImGui::InputScalarN("##value", ImGuiDataType_U8, &value, 3); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v3_i32, { ImGui::InputScalarN("##value", ImGuiDataType_S32, &value, 3); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v3_u32, { ImGui::InputScalarN("##value", ImGuiDataType_U32, &value, 3); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v3_i64, { ImGui::InputScalarN("##value", ImGuiDataType_S64, &value, 3); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v3_u64, { ImGui::InputScalarN("##value", ImGuiDataType_U64, &value, 3); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v3, { ImGui::InputScalarN("##value", ImGuiDataType_Float, &value, 3); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v3d, { ImGui::InputScalarN("##value", ImGuiDataType_Double, &value, 3); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v4_i8, { ImGui::InputScalarN("##value", ImGuiDataType_S8, &value, 4); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v4_u8, { ImGui::InputScalarN("##value", ImGuiDataType_U8, &value, 4); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v4_i32, { ImGui::InputScalarN("##value", ImGuiDataType_S32, &value, 4); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v4_u32, { ImGui::InputScalarN("##value", ImGuiDataType_U32, &value, 4); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v4_i64, { ImGui::InputScalarN("##value", ImGuiDataType_S64, &value, 4); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v4_u64, { ImGui::InputScalarN("##value", ImGuiDataType_U64, &value, 4); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v4, { ImGui::InputScalarN("##value", ImGuiDataType_Float, &value, 4); });
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(
		    v4d, { ImGui::InputScalarN("##value", ImGuiDataType_Double, &value, 4); });

		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(LinearColor, {
			ImGui::ColorEdit4("##value", reinterpret_cast<float*>(&value),
			    ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar
			        | ImGuiColorEditFlags_AlphaPreviewHalf);
		});
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(HSVColor, {
			LinearColor linear{value};
			if (ImGui::ColorEdit4("##value", reinterpret_cast<float*>(&linear),
			        ImGuiColorEditFlags_Float | ImGuiColorEditFlags_DisplayHSV
			            | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
			{
				value = HSVColor{linear};
			}
		});
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(Color, {
			LinearColor linear{value};
			if (ImGui::ColorEdit4("##value", reinterpret_cast<float*>(&linear),
			        ImGuiColorEditFlags_Uint8 | ImGuiColorEditFlags_DisplayRGB
			            | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
			{
				value = Color{linear};
			}
		});

		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(Path, {
			ImGui::SetNextItemWidth(p::Min(300.f, ImGui::GetContentRegionAvail().x));
			String asString = ToString(value);
			if (ImGui::InputText("##value", asString))
			{
				value = Path{ToSTDPath(asString)};
			}
		});

		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(Id, {
			String asString = ToString(value);
			ImGui::SetNextItemWidth(100.f);
			if (ImGui::InputText("##IdValue", asString, ImGuiInputTextFlags_EscapeClearsAll))
			{
				value = IdFromString(
				    asString, currentInspection ? currentInspection->idContext : nullptr);
			}
		});
	}


	bool BeginInspection(InspectionContext& ctx, const char* label, v2 size, InspectionFlags flags,
	    ImGuiTextFilter* filter, IdContext* idContext)
	{
		if (!P_EnsureMsg(!currentInspection,
		        "Called BeginInspection() but there was a Inspection Context already! "
		        "Forgot to call EndInspection()?"))
		{
			return false;
		}
		currentInspection            = &ctx;
		currentInspection->flags     = flags;
		currentInspection->filter    = filter;
		currentInspection->idContext = idContext;

		if (!currentInspection->initialized)
		{
			RegisterPipeTypeInspections();
			currentInspection->initialized = true;
		}

		const ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable
		                                 | ImGuiTableFlags_SizingStretchProp
		                                 | ImGuiTableFlags_PadOuterX;
		if (ImGui::BeginTable(label, 2, tableFlags, {size.x, size.y}))
		{
			ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 0.5f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 1.f);
			return true;
		}
		return false;
	}

	void EndInspection()
	{
		ImGui::EndTable();
		if (P_EnsureMsg(currentInspection,
		        "Called EndInspection() but there was no current Inspection Context! Forgot "
		        "to call BeginInspection()?"))
		{
			currentInspection->filter = nullptr;
			currentInspection->propStack.Clear();
		}
		currentInspection = nullptr;
	}


	void InspectSetKeyColumn()
	{
		ImGui::TableSetColumnIndex(0);
	}

	void InspectSetKeyAsText(StringView label)
	{
		InspectSetKeyColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label.data(), label.data() + label.size());
	}

	void InspectSetValueColumn()
	{
		ImGui::TableSetColumnIndex(1);
	}

	bool InspectBeginCategory(p::StringView name, bool isLeaf, bool defaultOpen)
	{
		InspectSetKeyColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Unindent();
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_AllowOverlap
		                               | ImGuiTreeNodeFlags_SpanAllColumns
		                               | (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0)
		                               | (defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0);
		// Categories are always enabled to navigate contents, but keep the disabled look
		const bool disabled = (GImGui->CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
		if (disabled)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		}
		bool open = ImGui::TreeNodeEx(name.data(), flags);
		if (disabled)
		{
			ImGui::PopStyleColor();
		}
		ImGui::PopItemFlag();
		ImGui::Indent();
		return open;
	}
	void InspectEndCategory()
	{
		ImGui::TreePop();
	}

	void Inspect(StringView label, void* data, TypeId typeId)
	{
		if (!EnsureInsideInspection || !data)
		{
			return;
		}

		const bool isContainer = p::HasTypeFlags(typeId, p::TF_Container);
		const bool isStruct    = p::HasTypeFlags(typeId, p::TF_Struct);
		const bool isEnum      = p::HasTypeFlags(typeId, p::TF_Enum);

		ImGui::TableNextRow();
		ImGui::PushID(data);
		bool open = false;
		auto* it  = currentInspection->registeredTypes.Find(typeId);
		if (it && it->onDrawRow)
		{
			it->onDrawRow(label, data, typeId, open);
		}
		else if (isEnum)
		{
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			const auto* ops = static_cast<const EnumTypeOps*>(GetTypeOps(typeId));
			if (P_EnsureMsg(ops, "Type is an enum but has no enum ops!"))
			{
				const i32 count = ops->Size();

				// Compare the enum value bytes against all registered values to find its index
				i32 currentIndex = NO_INDEX;
				u64 value        = 0;
				memcpy(&value, data, ops->valueSize);
				for (i32 i = 0; i < count; ++i)
				{
					u64 registered = 0;
					memcpy(&registered, ops->GetValuePtrByIndex(i), ops->valueSize);
					if (registered == value)
					{
						currentIndex = i;
						break;
					}
				}

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				String preview{
				    currentIndex >= 0 ? ops->names[currentIndex].AsString() : StringView{}};
				if (ImGui::BeginCombo("##enumValue", preview.c_str()))
				{
					for (i32 i = 0; i < count; ++i)
					{
						String name{ops->names[i].AsString()};
						if (ImGui::Selectable(name.c_str(), i == currentIndex))
						{
							ops->SetValueFromIndex(data, i);
						}
						if (i == currentIndex)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}
		}
		else if (isContainer)
		{
			const auto* ops = GetTypeContainerOps(typeId);
			P_Check(ops);
			const i32 size = ops->GetSize(data);
			open           = InspectBeginCategory(label, size <= 0);

			InspectSetValueColumn();
			ImGui::Text("%i items", size);

			const float widthAvailable =
			    ImGui::GetContentRegionAvail().x + details::GetCurrentWindow()->DC.Indent.x;
			ImGui::SameLine(widthAvailable - 20.f);
			if (ImGui::SmallButton("..."))
			{
				ImGui::OpenPopup("ContextualSettings");
			}

			if (ImGui::BeginPopupContextItem("ContextualSettings"))
			{
				if (ImGui::MenuItem("Add"))
				{
					ops->AddItem(data, nullptr);
					ImGui::CloseCurrentPopup();
				}
				ImGui::HelpTooltip("Add: Add one item");
				if (ImGui::MenuItem("Clear"))
				{
					ops->Clear(data);
					ImGui::CloseCurrentPopup();
				}
				ImGui::HelpTooltip("Clear: Remove all items of the array");
				ImGui::EndPopup();
			}
		}
		else if (isStruct)
		{
			auto props = GetTypeProperties(typeId);
			open       = InspectBeginCategory(label, props.Size() <= 0);
		}

		if (!currentInspection->propStack.IsEmpty())    // Container item buttons
		{
			const auto& prop = currentInspection->propStack[0];
			if (p::HasTypeFlags(prop.typeId, p::TF_Container))
			{
				const float widthAvailable =
				    ImGui::GetContentRegionAvail().x + details::GetCurrentWindow()->DC.Indent.x;
				ImGui::SameLine(widthAvailable - 20.f);
				if (ImGui::SmallButton("..."))
				{
					ImGui::OpenPopup("ContextualSettings");
				}
				if (ImGui::BeginPopupContextItem("ContextualSettings"))
				{
					if (p::HasTypeFlags(prop.typeId, p::TF_Container) && ImGui::MenuItem("Remove"))
					{
						const auto* ops = GetTypeContainerOps(prop.typeId);
						ops->RemoveItem(prop.data, prop.index);
						ImGui::CloseCurrentPopup();
					}
					ImGui::HelpTooltip("Remove: Remove this item from its array");
					ImGui::EndPopup();
				}
			}
		}

		if (open)
		{
			if (it && it->onDrawChildren)
			{
				it->onDrawChildren(data, typeId);
			}
			else if (isContainer)
			{
				const auto* ops = GetTypeContainerOps(typeId);
				String tmpLabel;
				const i32 size = ops->GetSize(data);
				currentInspection->propStack.Add({typeId, data, 0});
				for (i32 i = 0; i < size; ++i)
				{
					tmpLabel.clear();
					FormatTo(tmpLabel, "{}", i);
					Inspect(tmpLabel, ops->GetItem(data, i), ops->itemType);

					++currentInspection->propStack.Last().index;
				}
				currentInspection->propStack.RemoveLast();
				InspectEndCategory();
			}
			else if (isStruct)
			{
				InspectProperties(data, typeId);
				InspectEndCategory();
			}
		}
		ImGui::PopID();
	}
	void InspectProperties(void* data, TypeId typeId)
	{
		if (!EnsureInsideInspection || !data)
		{
			return;
		}

		const bool viewAll = HasFlag(currentInspection->flags, IF_ViewAll);
		const bool editAll = HasFlag(currentInspection->flags, IF_EditAll);

		auto props = GetTypeProperties(typeId);
		if (!props.IsEmpty())
		{
			currentInspection->propStack.Add({typeId, data, 0});
			for (const auto* prop : props)
			{
				if (viewAll || prop->HasFlag(PF_View))
				{
					StringView name = prop->name.AsString();
					if (!currentInspection->filter || !currentInspection->filter->IsActive()
					    || currentInspection->filter->PassFilter(
					        name.data(), name.data() + name.size()))
					{
						ImGui::BeginDisabled(!editAll && !prop->HasFlag(PF_Edit));
						Inspect(name, prop->access(data), prop->typeId);
						ImGui::EndDisabled();
					}
				}

				++currentInspection->propStack.Last().index;
			}
			currentInspection->propStack.RemoveLast();
		}
	}
#endif
}    // namespace p
