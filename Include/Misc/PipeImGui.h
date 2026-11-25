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

#if defined(__has_include)
	#if __has_include(<IconsFontAwesome7.h>)
		#include <IconsFontAwesome7.h>
	#elif __has_include(<IconsFontAwesome6.h>)
		#include <IconsFontAwesome6.h>
	#endif
#endif

#ifndef ICON_FA_ARROW_ROTATE_LEFT
	#define ICON_FA_ARROW_ROTATE_LEFT "\xe2\x86\x90"
	#define P_DEFINED_ICON_FA_ARROW_ROTATE_LEFT
#endif
#ifndef ICON_FA_EYE
	#define ICON_FA_EYE "\xe2\x97\x89"
	#define P_DEFINED_ICON_FA_EYE
#endif


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

	/**
	 * Single-line input whose contents stay horizontally centered, both while
	 * editing and when inactive. Honors SetNextItemWidth().
	 */
	inline bool InputTextCentered(const char* label, p::String& str, const char* hint = nullptr,
	    ImGuiInputTextFlags flags = 0)
	{
		const ImGuiStyle& style = GetStyle();
		const float paddingX    = style.FramePadding.x;
		const float paddingY    = style.FramePadding.y;
		const float width       = CalcItemWidth();
		const float textWidth   = CalcTextSize(str.c_str()).x;
		const float innerWidth  = width - paddingX * 2.0f;
		const float extra       = ImMax((innerWidth - textWidth) * 0.5f, 0.0f);

		PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(paddingX + extra, paddingY));
		const bool changed = InputText(label, str, flags);
		PopStyleVar();

		// The hint is drawn separately so the caret can stay centered while the
		// hint itself remains centered in the field.
		if (str.empty() && !IsItemActive() && hint && hint[0] != '\0')
		{
			const ImVec2 fieldMin = GetItemRectMin();
			const ImVec2 fieldMax = GetItemRectMax();
			const ImVec2 textSize = CalcTextSize(hint);
			const ImVec2 textPos{fieldMin.x + (fieldMax.x - fieldMin.x - textSize.x) * 0.5f,
			    fieldMin.y + (fieldMax.y - fieldMin.y - textSize.y) * 0.5f};
			ImDrawList* drawList = GetWindowDrawList();
			drawList->PushClipRect(fieldMin, fieldMax, true);
			drawList->AddText(textPos, GetColorU32(ImGuiCol_TextDisabled), hint);
			drawList->PopClipRect();
		}
		return changed;
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

	inline void SetItemTooltip(p::StringView text, ImGuiHoveredFlags flags = 0)
	{
		flags |= ImGuiHoveredFlags_ForTooltip;
		if (IsItemHovered(flags))
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
		IF_None      = 0,         // -> No flags
		IF_ViewAll   = 1 << 0,    // -> View all properties, even those without PF_View or PF_Edit
		IF_EditAll   = 1 << 1,    // -> Allow editing on all properties, even those without PF_Edit
		IF_NoPadding = 1 << 2,    // -> Remove table outer/inner horizontal padding
		IF_ViewAndEditAll = IF_ViewAll | IF_EditAll,
		IF_HideDefaults   = 1 << 3    // -> Dont draw reset to default buttons

		// Any other flags up to 64 bytes are available to the user
	};
	P_DEFINE_FLAG_OPERATORS(InspectionFlags_)

	struct InspectionContext
	{
		struct PropStack
		{
			TypeId typeId           = TypeId::None();
			void* data              = nullptr;
			const void* defaultData = nullptr;
			i32 index               = NO_INDEX;
			bool modified           = false;
			bool isDefault          = true;
			float rowY              = 0.0f;
		};

		TMap<TypeId, TypeInspection> registeredTypes;
		/** Parent properties of the currently inspected property. */
		TArray<PropStack> propStack;
		/** Property currently being inspected. */
		PropStack currentProperty;
		InspectionFlags flags = IF_None;
		/** Saved flags for InspectPushFlags()/InspectPopFlags(). */
		TArray<InspectionFlags> flagsStack;
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

	/** Whether the current inspection scope was edited. */
	bool IsInspectionModified();
	/** Whether the current inspection scope equals its default value. */
	bool IsInspectionDefault();

	/** Adds flags for the following inspected values until InspectPopFlags(). */
	void InspectPushFlags(InspectionFlags flags);
	void InspectPopFlags();

	void InspectSetKeyColumn();
	void InspectSetKeyAsText(StringView label);
	void InspectSetValueColumn();
	bool InspectBeginCategory(p::StringView name, bool isLeaf, bool defaultOpen = false);
	void InspectEndCategory();
	/** Pushes a property value frame; structs should wrap their category with this. */
	void InspectPushProperty(TypeId typeId, void* data, const void* defaultData = nullptr);
	/** Pops the property frame and draws its deferred default button. */
	void InspectPopProperty();
	/** Inspects a value, optionally comparing it against a default value. */
	void Inspect(StringView label, void* data, TypeId typeId, const void* defaultData = nullptr,
	    const Metadata* metadata = nullptr);
	template<typename T>
	inline void Inspect(StringView label, T* data, const T* defaultData = nullptr,
	    const Metadata* metadata = nullptr)
	{
		Inspect(label, data, GetTypeId<T>(), defaultData, metadata);
	}
	void InspectProperties(TypeId typeId, void* data, const void* defaultData = nullptr);
	template<typename T>
	inline void InspectProperties(T* data, const T* defaultData = nullptr)
	{
		InspectProperties(GetTypeId<T>(), data, defaultData);
	}


	///////////////////////////////////////////////////////////
	// Implementation
#ifdef P_IMGUI_IMPLEMENTATION
	static InspectionContext* currentInspection = nullptr;

	bool IsInspectionModified()
	{
		return currentInspection && currentInspection->currentProperty.modified;
	}

	bool IsInspectionDefault()
	{
		return !currentInspection || currentInspection->currentProperty.isDefault;
	}

	void InspectPushFlags(InspectionFlags flags)
	{
		if (!P_EnsureMsg(currentInspection,
		        "No inspection context available! Forgot to call BeginInspection()?"))
		{
			return;
		}
		currentInspection->flagsStack.Add(currentInspection->flags);
		currentInspection->flags |= flags;
	}

	void InspectPopFlags()
	{
		if (!currentInspection || currentInspection->flagsStack.IsEmpty())
		{
			return;
		}
		currentInspection->flags = currentInspection->flagsStack.Last();
		currentInspection->flagsStack.RemoveLast();
	}

	namespace details
	{
		inline void MarkModified()
		{
			for (InspectionContext::PropStack& scope : currentInspection->propStack)
			{
				scope.modified = true;
			}
			currentInspection->currentProperty.modified = true;
		}

		inline void MarkNonDefault()
		{
			for (InspectionContext::PropStack& scope : currentInspection->propStack)
			{
				scope.isDefault = false;
			}
			currentInspection->currentProperty.isDefault = false;
		}

		inline void CaptureRow(InspectionContext::PropStack& scope)
		{
			ImGuiTable* table = ImGui::GetCurrentTable();
			if (table && table->ColumnsCount >= 3)
			{
				scope.rowY = table->RowPosY1;
			}
		}

		inline void DrawResetButton(const InspectionContext::PropStack& scope)
		{
			if (HasFlag(currentInspection->flags, IF_HideDefaults) || scope.isDefault || !scope.data
			    || !scope.defaultData)
			{
				return;
			}

			ImGuiTable* table = ImGui::GetCurrentTable();
			if (!table)
			{
				return;
			}
			const int savedRow    = table->CurrentRow;
			const int savedColumn = table->CurrentColumn;
			const float savedY1   = table->RowPosY1;
			const float savedY2   = table->RowPosY2;
			table->RowPosY1       = scope.rowY;
			table->RowPosY2       = scope.rowY + ImGui::GetFrameHeight();
			ImGui::TableSetColumnIndex(2);
			ImGui::PushID(scope.data);

			const float size = ImGui::GetFrameHeight();
			ImGui::InvisibleButton("reset##reset", ImVec2(size, size));
			const bool hovered = ImGui::IsItemHovered();
			if (ImGui::IsItemClicked())
			{
				p::CopyType(scope.typeId, scope.data, scope.defaultData);
				MarkModified();
			}
			const char* icon      = ICON_FA_ARROW_ROTATE_LEFT;
			const ImVec2 iconSize = ImGui::CalcTextSize(icon);
			const ImVec2 min      = ImGui::GetItemRectMin();
			const ImVec2 max      = ImGui::GetItemRectMax();
			ImGui::GetWindowDrawList()->AddText(
			    ImVec2((min.x + max.x - iconSize.x) * 0.5f, (min.y + max.y - iconSize.y) * 0.5f),
			    ImGui::GetColorU32(hovered ? ImGuiCol_Text : ImGuiCol_TextDisabled), icon);

			ImGui::PopID();
			table->RowPosY1   = savedY1;
			table->RowPosY2   = savedY2;
			table->CurrentRow = savedRow;
			if (savedColumn >= 0)
			{
				ImGui::TableSetColumnIndex(savedColumn);
			}
			else
			{
				ImGui::TableEndCell(table);
				table->CurrentColumn = -1;
			}
		}

		inline void DrawRowTooltip(StringView hint)
		{
			const ImGuiTable* table = ImGui::GetCurrentTable();
			if (hint.empty() || !table)
			{
				return;
			}

			// Item tooltips submitted earlier in this row take priority over the row tooltip.
			ImGuiContext& g                = *GImGui;
			static ImGuiWindow* rowTooltip = nullptr;
			static int rowTooltipFrame     = -1;
			const bool itemTooltipActive =
			    g.TooltipPreviousWindow && g.TooltipPreviousWindow->LastFrameActive == g.FrameCount
			    && !(g.TooltipPreviousWindow == rowTooltip && rowTooltipFrame == g.FrameCount);
			if (itemTooltipActive || !ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)
			    || ImGui::IsPopupOpen(
			        nullptr, ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
			{
				return;
			}

			const ImVec2 min{table->WorkRect.Min.x, table->RowPosY1};
			const ImVec2 max{table->WorkRect.Max.x, table->RowPosY2};
			if (ImGui::IsMouseHoveringRect(min, max, false))
			{
				ImGui::SetTooltip("%.*s", static_cast<int>(hint.size()), hint.data());
				rowTooltip      = g.TooltipPreviousWindow;
				rowTooltipFrame = g.FrameCount;
			}
		}

		inline bool HeaderRow(StringView name, bool isLeaf, bool defaultOpen, StringView hint = {})
		{
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_AllowOverlap
			                               | ImGuiTreeNodeFlags_SpanAllColumns
			                               | (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0)
			                               | (defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0);
			// Headers are always enabled to navigate contents, but keep the disabled look
			const bool disabled = (GImGui->CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
			if (disabled)
			{
				ImGui::PushStyleColor(
				    ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			}
			const bool open = ImGui::TreeNodeEx(name.data(), flags);
			DrawRowTooltip(hint);
			if (disabled)
			{
				ImGui::PopStyleColor();
			}
			ImGui::PopItemFlag();
			return open;
		}
	}    // namespace details


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

		ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp
		                           | ImGuiTableFlags_NoBordersInBody;
		if (HasFlag(flags, IF_NoPadding))
		{
			tableFlags |= ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;
		}
		else
		{
			tableFlags |= ImGuiTableFlags_PadOuterX;
		}
		if (ImGui::BeginTable(label, 3, tableFlags, {size.x, size.y}))
		{
			ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 0.5f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 1.f);
			ImGui::TableSetupColumn("Reset", ImGuiTableColumnFlags_WidthFixed, 22.0f);
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
			currentInspection->propStack.Clear();
			currentInspection->flagsStack.Clear();
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
		ImGui::SetNextItemWidth(-1.0f);
	}

	bool InspectBeginCategory(p::StringView name, bool isLeaf, bool defaultOpen)
	{
		return details::HeaderRow(name, isLeaf, defaultOpen);
	}

	void InspectEndCategory()
	{
		ImGui::TreePop();
	}

	void InspectPushProperty(TypeId typeId, void* data, const void* defaultData)
	{
		currentInspection->propStack.Add(currentInspection->currentProperty);
		currentInspection->currentProperty = {.typeId = typeId,
		    .data                                     = data,
		    .defaultData                              = defaultData,
		    .index                                    = NO_INDEX,
		    .modified                                 = false,
		    .isDefault                                = true};
		details::CaptureRow(currentInspection->currentProperty);
	}

	void InspectPopProperty()
	{
		InspectionContext::PropStack scope = currentInspection->currentProperty;
		if (scope.isDefault && scope.defaultData
		    && !p::EqualsType(scope.typeId, scope.data, scope.defaultData))
		{
			scope.isDefault = false;
			details::MarkNonDefault();
		}
		details::DrawResetButton(scope);
		if (!currentInspection->propStack.IsEmpty())
		{
			currentInspection->currentProperty = currentInspection->propStack.Last();
			currentInspection->propStack.RemoveLast();
		}
		else
		{
			currentInspection->currentProperty = {};
		}
	}

	void Inspect(StringView label, void* data, TypeId typeId, const void* defaultData,
	    const Metadata* metadata)
	{
		if (!EnsureInsideInspection || !data)
		{
			return;
		}

		if (!defaultData)
		{
			defaultData = GetTypeDefault(typeId);
		}

		const bool isContainer = p::HasTypeFlags(typeId, p::TF_Container);
		const bool isStruct    = p::HasTypeFlags(typeId, p::TF_Struct);
		const bool isEnum      = p::HasTypeFlags(typeId, p::TF_Enum);

		ImGui::TableNextRow();
		ImGui::PushID(data);
		InspectPushProperty(typeId, data, defaultData);
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
			open           = details::HeaderRow(
			    label, size <= 0, false, metadata ? metadata->hint : StringView{});

			InspectSetValueColumn();
			ImGui::Text("%i items", size);

			const float widthAvailable =
			    ImGui::GetContentRegionAvail().x + details::GetCurrentWindow()->DC.Indent.x;
			ImGui::SameLine(widthAvailable - 20.f);
			if (ImGui::TextButton(ICON_FA_EYE, "container_settings"))
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
			open       = details::HeaderRow(
			    label, props.Size() <= 0, defaultData, metadata ? metadata->hint : StringView{});
		}

		if (!currentInspection->propStack.IsEmpty())    // Container item buttons
		{
			InspectionContext::PropStack& parent = currentInspection->propStack.Last();
			if (p::HasTypeFlags(parent.typeId, p::TF_Container))
			{
				const float widthAvailable =
				    ImGui::GetContentRegionAvail().x + details::GetCurrentWindow()->DC.Indent.x;
				ImGui::SameLine(widthAvailable - 20.f);
				if (ImGui::TextButton(ICON_FA_EYE, "container_settings"))
				{
					ImGui::OpenPopup("ContextualSettings");
				}
				if (ImGui::BeginPopupContextItem("ContextualSettings"))
				{
					if (ImGui::MenuItem("Remove"))
					{
						const auto* ops = GetTypeContainerOps(parent.typeId);
						ops->RemoveItem(parent.data, parent.index);
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
				for (i32 i = 0; i < size; ++i)
				{
					currentInspection->currentProperty.index = i;
					tmpLabel.clear();
					FormatTo(tmpLabel, "{}", i);
					Inspect(tmpLabel, ops->GetItem(data, i), ops->itemType);
				}
				ImGui::TreePop();
			}
			else if (isStruct)
			{
				InspectProperties(typeId, data, defaultData);
				ImGui::TreePop();
			}
		}
		if (ImGui::IsItemEdited())
		{
			details::MarkModified();
		}
		if (metadata && !isStruct && !isContainer)
		{
			details::DrawRowTooltip(metadata->hint);
		}
		InspectPopProperty();
		ImGui::PopID();
	}

	void InspectProperties(TypeId typeId, void* data, const void* defaultData)
	{
		if (!EnsureInsideInspection || !data)
		{
			return;
		}

		if (!defaultData)
		{
			defaultData = GetTypeDefault(typeId);
		}

		const bool viewAll = HasFlag(currentInspection->flags, IF_ViewAll);
		const bool editAll = HasFlag(currentInspection->flags, IF_EditAll);

		auto props = GetTypeProperties(typeId);
		if (props.IsEmpty())
		{
			return;
		}

		auto passesFilter = [&](StringView text)
		{
			return !currentInspection->filter
			    || currentInspection->filter->PassFilter(text.data(), text.data() + text.size());
		};

		auto isVisible = [&](const TypeProperty* prop)
		{
			if (!viewAll && !prop->HasFlag(PF_View))
			{
				return false;
			}
			return passesFilter(prop->meta.category) || passesFilter(prop->name.AsString());
		};

		auto drawProperty = [&](const TypeProperty* prop)
		{
			StringView name = prop->name.AsString();
			ImGui::BeginDisabled(!editAll && !prop->HasFlag(PF_Edit));
			const void* defaultProperty =
			    defaultData ? prop->access(const_cast<void*>(defaultData)) : nullptr;
			Inspect(name, prop->access(data), prop->typeId, defaultProperty, &prop->meta);
			ImGui::EndDisabled();
		};

		TArray<const TypeProperty*> visibleProperties;
		for (const TypeProperty* prop : props)
		{
			if (isVisible(prop))
			{
				visibleProperties.Add(prop);
			}
		}

		for (const TypeProperty* prop : visibleProperties)
		{
			if (prop->meta.category.empty())
			{
				drawProperty(prop);
			}
		}

		TArray<StringView> categories;
		for (const TypeProperty* prop : visibleProperties)
		{
			if (!prop->meta.category.empty() && !categories.Contains(prop->meta.category))
			{
				categories.Add(prop->meta.category);
			}
		}

		for (StringView category : categories)
		{
			ImGui::TableNextRow();
			if (InspectBeginCategory(category, false, true))
			{
				for (const TypeProperty* prop : visibleProperties)
				{
					if (prop->meta.category == category)
					{
						drawProperty(prop);
					}
				}
				InspectEndCategory();
			}
		}
	}
#endif
}    // namespace p

#ifdef P_DEFINED_ICON_FA_ARROW_ROTATE_LEFT
	#undef ICON_FA_ARROW_ROTATE_LEFT
	#undef P_DEFINED_ICON_FA_ARROW_ROTATE_LEFT
#endif
#ifdef P_DEFINED_ICON_FA_EYE
	#undef ICON_FA_EYE
	#undef P_DEFINED_ICON_FA_EYE
#endif
