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


namespace ImGui
{
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
	// Definition

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
}    // namespace ImGui
