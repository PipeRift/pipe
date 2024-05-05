// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

// ImGui Tools for debugging Pipe
// Required ImGui (v1.90 or newer). Must be included before this header.

// Use: #define P_DEBUG_IMPLEMENTATION
// In a C++ file, once and before you include this file to create the implementation.

#ifndef IMGUI_VERSION_NUM
static_assert(false, "Imgui not found. v1.90 or newer is required.");
#elif IMGUI_VERSION_NUM < 19000
static_assert(false, "Imgui v" IMGUI_VERSION " found but v1.90 or newer is required.");
#endif


#include "Misc/PipeImGui.h"
#include "PipeArrays.h"
#include "PipeECS.h"


namespace p
{
	///////////////////////////////////////////////////////////
	// Definition

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

	struct ECSDebugContext
	{
		EntityContext* ctx = nullptr;
		TArray<ECSDebugIdRegistry> idRegistries;


		ECSDebugContext() = default;
		ECSDebugContext(EntityContext& ctx) : ctx{&ctx} {}
	};


	static void BeginECSDebug(ECSDebugContext& Context);
	static void EndECSDebug();

	static bool BeginIdRegistry(
	    const char* label = "Id Registry", bool* open = nullptr, ImGuiWindowFlags flags = 0);
	static void EndIdRegistry();
	static void DrawIdRegistry(
	    const char* label = "Id Registry", bool* open = nullptr, ImGuiWindowFlags flags = 0);

	static void DrawEntityInspector(StringView label, ECSDebugInspector& inspector,
	    bool* open = nullptr, ImGuiWindowFlags flags = 0);
	static void DrawOpenEntityInspectors(ImGuiWindowFlags flags = 0);


	///////////////////////////////////////////////////////////
	// Implementation
#ifdef P_DEBUG_IMPLEMENTATION

	#define EnsureInsideECSDebug \
		P_EnsureMsg(             \
		    currentContext, "No ECS Debug context available! Forgot to call BeginECSDebug()?")

	static ECSDebugContext* currentContext = nullptr;

	constexpr p::LinearColor errorColor = p::LinearColor::FromHex(0xD62B2B);

	// For internal use only
	EntityContext& GetDebugCtx()
	{
		return *currentContext->ctx;
	}

	void BeginECSDebug(ECSDebugContext& context)
	{
		if (!P_EnsureMsg(!currentContext,
		        "Called BeginECSDebug() but there was a ECS Debug Context already! "
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
	}
	void EndECSDebug()
	{
		P_CheckMsg(currentContext,
		    "Called EndECSDebug() but there was no current ECS Debug Context! Forgot "
		    "to call "
		    "BeginECSDebug()?");
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
			if (ImGui::BeginMenu(ICON_FA_BARS))
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
			const auto& registry = p::TypeRegistry::Get();
			for (const auto& poolInstance : GetDebugCtx().GetPools())
			{
				if (!poolInstance.GetPool()->Has(inspector.id))
				{
					continue;
				}

				componentLabel.clear();
				p::Type* type = registry.FindType(poolInstance.componentId);
				if (!type)
				{
					p::Strings::FormatTo(
					    componentLabel, "{} (unknown type)", poolInstance.componentId);
					ImGui::CollapsingHeader(componentLabel.c_str(), ImGuiTreeNodeFlags_Leaf);
					continue;
				}

				p::Strings::FormatTo(componentLabel, "{}", type->GetName());

				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
				void* data               = poolInstance.GetPool()->TryGetVoid(inspector.id);
				if (!data)
				{
					flags |= ImGuiTreeNodeFlags_Leaf;
				}
				if (ImGui::CollapsingHeader(componentLabel.c_str(), flags))
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
