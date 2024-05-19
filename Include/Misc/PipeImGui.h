// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

// ImGui helpers for Pipe
// Required ImGui (v1.90 or newer). Must be included before this header.

// Use: #define P_IMGUI_IMPLEMENTATION
// In a C++ file, once and before you include this file to create the implementation.

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
	///////////////////////////////////////////////////////////
	// Definition


	inline void PushID(p::StringView id)
	{
		PushID(id.data(), id.data() + id.size());
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
	void PushTextColor(p::LinearColor color);
	void PopTextColor();
	p::LinearColor GetTextColor();

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
	bool InputText(const char* label, p::String& str, ImGuiInputTextFlags flags = 0,
	    ImGuiInputTextCallback callback = nullptr, void* userData = nullptr);
	bool InputTextMultiline(const char* label, p::String& str, const ImVec2& size = ImVec2(0, 0),
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
	    void* userData = nullptr);
	bool InputTextWithHint(const char* label, const char* hint, p::String& str,
	    ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr,
	    void* userData = nullptr);

	void HelpTooltip(p::StringView text, float delay = 1.f);
	void HelpMarker(p::StringView text);

	bool DrawFilterWithHint(ImGuiTextFilter& filter, const char* label = "Filter (inc,-exc)",
	    const char* hint = "...", float width = 0.0f);


	///////////////////////////////////////////////////////////
	// Implementation
#ifdef P_IMGUI_IMPLEMENTATION

	void PushTextColor(p::LinearColor color)
	{
		PushStyleColor(ImGuiCol_Text, color);
		PushStyleColor(ImGuiCol_TextDisabled, color.Shade(0.15f));
	}

	void PopTextColor()
	{
		PopStyleColor(2);
	}

	p::LinearColor GetTextColor()
	{
		auto color = GetStyleColorVec4(ImGuiCol_TextDisabled);
		return {color.x, color.y, color.z, color.w};
	}

	struct InputTextCallbackStringUserData
	{
		p::String* str;
		ImGuiInputTextCallback chainCallback;
		void* chainCallbackUserData;
	};

	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		auto* userData = static_cast<InputTextCallbackStringUserData*>(data->UserData);
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			// Resize string callback
			// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we
			// need to set them back to what we want.
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


	bool InputText(const char* label, p::String& str, ImGuiInputTextFlags flags,
	    ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallbackStringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputText(
		    label, (char*)str.c_str(), str.capacity() + 1, flags, InputTextCallback, &cbUserData);
	}

	bool InputTextMultiline(const char* label, p::String& str, const ImVec2& size,
	    ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallbackStringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputTextMultiline(label, (char*)str.c_str(), str.capacity() + 1, size, flags,
		    InputTextCallback, &cbUserData);
	}

	bool InputTextWithHint(const char* label, const char* hint, p::String& str,
	    ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* userData)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallbackStringUserData cbUserData;
		cbUserData.str                   = &str;
		cbUserData.chainCallback         = callback;
		cbUserData.chainCallbackUserData = userData;
		return ImGui::InputTextWithHint(label, hint, (char*)str.c_str(), str.capacity() + 1, flags,
		    InputTextCallback, &cbUserData);
	}

	static ImGuiID gPendingEditingId = 0;

	void HelpTooltip(p::StringView text, float delay)
	{
		static ImGuiID currentHelpItemId = 0;

		ImGuiID itemId = ImGui::GetCurrentContext()->LastItemData.ID;
		if (ImGui::IsItemHovered())
		{
			bool show = true;
			if (delay > 0.f)
			{
				static p::DateTime hoverStartTime;
				const p::DateTime now = p::DateTime::Now();
				if (itemId != currentHelpItemId)
				{
					// Reset help tooltip countdown
					currentHelpItemId = itemId;
					hoverStartTime    = now;
				}
				show = (now - hoverStartTime).GetTotalSeconds() > delay;
			}

			if (show)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, p::v2{4.f, 3.f});
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted(text.data());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
				ImGui::PopStyleVar();
			}
		}
		else if (itemId == currentHelpItemId)
		{
			currentHelpItemId = 0;
		}
	}
	void HelpMarker(p::StringView text)
	{
		ImGui::TextDisabled("(?)");
		HelpTooltip(text, 0.f);
	}

	bool DrawFilterWithHint(
	    ImGuiTextFilter& filter, const char* label, const char* hint, float width)
	{
		if (width != 0.0f)
			ImGui::SetNextItemWidth(width);
		bool value_changed =
		    ImGui::InputTextWithHint(label, hint, filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf));
		if (value_changed)
			filter.Build();
		return value_changed;
	}
#endif
};    // namespace ImGui
