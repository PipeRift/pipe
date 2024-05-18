// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

// ImGui Tools for debugging Pipe
// Required ImGui (v1.90 or newer). Must be included before this header.

// Use: #define P_DEBUG_IMPLEMENTATION
// In a C++ file, once and before you include this file to create the implementation.

#ifndef IMGUI_VERSION_NUM
static_assert(false, "Imgui not found. PipeDebug requires v1.90 or newer.");
#elif IMGUI_VERSION_NUM < 19000
static_assert(false, "Imgui v" IMGUI_VERSION " found but PipeDebug requires v1.90 or newer.");
#endif


#include "Misc/PipeImGui.h"
#include "PipeArrays.h"
#include "PipeECS.h"


namespace p
{
	///////////////////////////////////////////////////////////
	// Definition

#pragma region Inspection
	bool BeginInspector(const char* name, p::v2 size = p::v2{0.f, 0.f});
	void EndInspector();


#pragma endregion Inspection


#pragma region ECS
	struct ECSDebugIdRegistry
	{
		bool visible = false;

		// Filters
		TArray<TypeId> includedPools;
		TArray<TypeId> excludedPools;
	};

	struct ECSDebugInspector
	{
		Id id             = NoId;
		bool pendingFocus = false;
	};

	struct DebugContext
	{
		EntityContext* ctx = nullptr;
		TArray<ECSDebugIdRegistry> idRegistries;


		DebugContext() = default;
		DebugContext(EntityContext& ctx) : ctx{&ctx} {}
	};


	static bool BeginDebug(DebugContext& Context);
	static void EndDebug();

	static bool BeginIdRegistry(
	    const char* label = "Id Registry", bool* open = nullptr, ImGuiWindowFlags flags = 0);
	static void EndIdRegistry();
	static void DrawIdRegistry(
	    const char* label = "Id Registry", bool* open = nullptr, ImGuiWindowFlags flags = 0);

	static void DrawReflection(
	    const char* label = "Reflection", bool* open = nullptr, ImGuiWindowFlags flags = 0);

	static void DrawEntityInspector(StringView label, ECSDebugInspector& inspector,
	    bool* open = nullptr, ImGuiWindowFlags flags = 0);
	static void DrawOpenEntityInspectors(ImGuiWindowFlags flags = 0);
#pragma endregion ECS


	///////////////////////////////////////////////////////////
	// Implementation
#ifdef P_DEBUG_IMPLEMENTATION

	#define EnsureInsideECSDebug                              \
		P_EnsureMsg(currentContext,                           \
		    "No ECS Debug context available! Forgot to call " \
		    "BeginDebug()?")

	static DebugContext* currentContext = nullptr;

	constexpr p::LinearColor errorColor = p::LinearColor::FromHex(0xD62B2B);

	// For internal use only
	EntityContext& GetDebugCtx()
	{
		return *currentContext->ctx;
	}

	bool BeginDebug(DebugContext& context)
	{
		if (!P_EnsureMsg(!currentContext,
		        "Called BeginDebug() but there was a ECS Debug Context already! "
		        "Forgot to call "
		        "EndECSDebug()?"))
		{
			return false;
		}

		if (!P_EnsureMsg(context.ctx, "Debug Context does not contain a valid EntityContext."))
		{
			return false;
		}

		currentContext = &context;
		return true;
	}
	void EndDebug()
	{
		P_CheckMsg(currentContext,
		    "Called EndECSDebug() but there was no current ECS Debug Context! Forgot "
		    "to call "
		    "BeginDebug()?");
		currentContext = nullptr;
	}

	bool BeginIdRegistry(const char* label, bool* open, ImGuiWindowFlags flags)
	{
		return ImGui::Begin(label, open, flags);
	}
	void EndIdRegistry()
	{
		ImGui::End();
	}
	void DrawIdRegistry(const char* label, bool* open, ImGuiWindowFlags flags)
	{
		if (BeginIdRegistry(label, open, flags))
		{
			EndIdRegistry();
		}
	}

	void DrawReflection(const char* label, bool* open, ImGuiWindowFlags flags)
	{
		if (ImGui::Begin(label, open, flags))
		{
			ImGui::End();
		}
	}

	void DrawEntityInspector(
	    StringView label, ECSDebugInspector& inspector, bool* open, ImGuiWindowFlags flags)
	{
		if (!EnsureInsideECSDebug)
		{
			return;
		}

		const bool valid   = GetDebugCtx().IsValid(inspector.id);
		const bool removed = GetDebugCtx().WasRemoved(inspector.id);
		bool clone         = false;
		p::Id nextId       = inspector.id;

		p::String name;
		p::Strings::FormatTo(name, "{}: {}{}", label, inspector.id, removed ? " (removed)" : "");

		if (inspector.pendingFocus)
		{
			ImGui::SetNextWindowFocus();
			inspector.pendingFocus = false;
		}

		ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos() + ImVec2(20, 20), ImGuiCond_Appearing);
		ImGui::SetNextWindowSizeConstraints(ImVec2(300.f, 200.f), ImVec2(800, FLT_MAX));
		if (!valid || removed)
		{
			ImGui::PushTextColor(errorColor);
		}
		ImGui::Begin(name.c_str(), open, ImGuiWindowFlags_MenuBar);
		if (!valid || removed)
		{
			ImGui::PopTextColor();
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("..."))
			{
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Id");
				ImGui::SameLine();
				p::String asString = p::ToString(inspector.id);
				ImGui::SetNextItemWidth(100.f);
				if (ImGui::InputText("##IdValue", asString,
				        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll))
				{
					nextId = p::IdFromString(asString);
				}
				ImGui::EndMenu();
			}

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 40.f);
			if (ImGui::MenuItem("Clone"))
			{
				clone = true;
			}
			ImGui::EndMenuBar();
		}

		if (valid)
		{
			p::String componentLabel;
			for (const auto& poolInstance : GetDebugCtx().GetPools())
			{
				if (!poolInstance.GetPool()->Has(inspector.id))
				{
					continue;
				}

				componentLabel.clear();

				p::Strings::FormatTo(
				    componentLabel, "{}", RemoveNamespace(GetTypeName(poolInstance.componentId)));

				ImGuiTreeNodeFlags headerFlags = ImGuiTreeNodeFlags_DefaultOpen;
				void* data                     = poolInstance.GetPool()->TryGetVoid(inspector.id);
				if (!data)
				{
					headerFlags |= ImGuiTreeNodeFlags_Leaf;
				}
				if (ImGui::CollapsingHeader(componentLabel.c_str(), headerFlags))
				{
					// UI::Indent();
					// auto* dataType = Cast<p::DataType>(type);
					// if (data && dataType && UI::BeginInspector("EntityInspector"))
					//{
					//	UI::InspectChildrenProperties({data, dataType});
					//	UI::EndInspector();
					// }
					// UI::Unindent();
				}
			}
		}
		ImGui::End();

		// Update after drawing
		if (nextId != inspector.id)
		{
			inspector.id = nextId;
		}
	}
	void DrawOpenEntityInspectors(ImGuiWindowFlags flags) {}
#endif
};    // namespace p
