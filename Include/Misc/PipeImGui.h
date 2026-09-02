// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

// ImGui helpers for Pipe
// Required ImGui (v1.90 or newer). Must be included before this header.

#ifndef IMGUI_VERSION_NUM
static_assert(false, "Imgui not found. PipeImGui requires v1.90 or newer.");
#elif IMGUI_VERSION_NUM < 19000
static_assert(false, "Imgui v" IMGUI_VERSION " found but PipeImGui requires v1.90 or newer.");
#endif


#include "PipeColor.h"
#include "PipeTime.h"

#include <imgui.h>


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
		RenderText(GetItemRectMin() + padding, text);
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
