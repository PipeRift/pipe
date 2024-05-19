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
	bool BeginInspector(const char* name, v2 size = v2{0.f, 0.f});
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

	struct DebugReflectContext
	{
		ImGuiTextFilter filter;
		TypeFlags typeFlagsFilter = TF_Native | TF_Enum | TF_Struct | TF_Object;
	};

	struct DebugContext
	{
		DebugReflectContext reflect;

		EntityContext* ctx = nullptr;
		TArray<ECSDebugIdRegistry> idRegistries;


		DebugContext() = default;
		DebugContext(EntityContext& ctx) : ctx{&ctx} {}
	};


	bool BeginDebug(DebugContext& Context);
	void EndDebug();

	bool BeginIdRegistry(
	    const char* label = "Id Registry", bool* open = nullptr, ImGuiWindowFlags flags = 0);
	void EndIdRegistry();
	void DrawIdRegistry(
	    const char* label = "Id Registry", bool* open = nullptr, ImGuiWindowFlags flags = 0);

	void DrawReflection(
	    const char* label = "Reflection", bool* open = nullptr, ImGuiWindowFlags flags = 0);

	void DrawEntityInspector(StringView label, ECSDebugInspector& inspector, bool* open = nullptr,
	    ImGuiWindowFlags flags = 0);
	void DrawOpenEntityInspectors(ImGuiWindowFlags flags = 0);
#pragma endregion ECS


	///////////////////////////////////////////////////////////
	// Implementation
#ifdef P_DEBUG_IMPLEMENTATION

	#define EnsureInsideDebug                                 \
		P_EnsureMsg(currentContext,                           \
		    "No ECS Debug context available! Forgot to call " \
		    "BeginDebug()?")

	static DebugContext* currentContext = nullptr;
	constexpr LinearColor errorColor    = LinearColor::FromHex(0xD62B2B);


	namespace details
	{
		void DrawType(DebugReflectContext& ctx, TypeId type)
		{
			if (!HasAnyTypeFlags(type, ctx.typeFlagsFilter))
			{
				return;
			}

			static String idText;
			idText.clear();
			Strings::FormatTo(idText, "{}", type);

			StringView rawName = GetTypeName(type);
			if (!ctx.filter.PassFilter(idText.c_str(), idText.c_str() + idText.size())
			    && !ctx.filter.PassFilter(rawName.data(), rawName.data() + rawName.size()))
			{
				return;
			}

			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);    // Id
			ImGui::Text(idText);

			ImGui::TableSetColumnIndex(1);    // Name
			StringView ns;
			StringView name = RemoveNamespace(rawName, ns);
			ImGui::PushTextColor(ImGui::GetTextColor().Shade(0.3f));
			ImGui::Text(ns);
			ImGui::PopTextColor();
			ImGui::SameLine(0, 10.f);
			ImGui::Text(name);

			ImGui::TableSetColumnIndex(2);    // Flags
			static String flags;
			flags.clear();
			GetEnumFlagName<TypeFlags_>(TypeFlags_(GetTypeFlags(type)), flags);
			ImGui::Text(flags);

			TypeId parentId = GetTypeParent(type);
			if (parentId.IsValid())
			{
				ImGui::TableSetColumnIndex(3);    // Parent
				rawName = GetTypeName(parentId);
				name    = RemoveNamespace(rawName, ns);
				ImGui::PushTextColor(ImGui::GetTextColor().Shade(0.3f));
				ImGui::Text(ns);
				ImGui::PopTextColor();
				ImGui::SameLine(0, 10.f);
				ImGui::Text(name);
			}
		}
	}    // namespace details

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
		if (!EnsureInsideDebug)
		{
			return;
		}

		auto& reflectDbg = currentContext->reflect;

		ImGui::Begin(label, open, flags);

		if (ImGui::BeginPopup("Filter"))
		{
			ImGui::CheckboxFlags("Native", &reflectDbg.typeFlagsFilter, u64(TF_Native));
			ImGui::CheckboxFlags("Enum", &reflectDbg.typeFlagsFilter, u64(TF_Enum));
			ImGui::CheckboxFlags("Struct", &reflectDbg.typeFlagsFilter, u64(TF_Struct));
			ImGui::CheckboxFlags("Object", &reflectDbg.typeFlagsFilter, u64(TF_Object));
			ImGui::CheckboxFlags("Container", &reflectDbg.typeFlagsFilter, u64(TF_Container));
			ImGui::EndPopup();
		}
		if (ImGui::Button("Filter"))
		{
			ImGui::OpenPopup("Filter");
		}

		ImGui::SameLine();
		reflectDbg.filter.Draw("##Filter", -100.0f);

		static ImGuiTableFlags tableFlags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable
		                                  | ImGuiTableFlags_Hideable
		                                  | ImGuiTableFlags_SizingStretchProp;
		ImGui::BeginChild("typesTableChild", ImVec2(0.f, ImGui::GetContentRegionAvail().y));
		if (ImGui::BeginTable("typesTable", 4, tableFlags))
		{
			ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_IndentEnable);
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Flags");
			ImGui::TableSetupColumn("Parent");
			ImGui::TableHeadersRow();

			for (TypeId type : GetRegisteredTypeIds())
			{
				details::DrawType(reflectDbg, type);
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
		ImGui::End();
	}

	void DrawEntityInspector(
	    StringView label, ECSDebugInspector& inspector, bool* open, ImGuiWindowFlags flags)
	{
		if (!EnsureInsideDebug)
		{
			return;
		}

		const bool valid   = GetDebugCtx().IsValid(inspector.id);
		const bool removed = GetDebugCtx().WasRemoved(inspector.id);
		bool clone         = false;
		Id nextId          = inspector.id;

		String name;
		Strings::FormatTo(name, "{}: {}{}", label, inspector.id, removed ? " (removed)" : "");

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
				String asString = ToString(inspector.id);
				ImGui::SetNextItemWidth(100.f);
				if (ImGui::InputText("##IdValue", asString,
				        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll))
				{
					nextId = IdFromString(asString);
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
			String componentLabel;
			for (const auto& poolInstance : GetDebugCtx().GetPools())
			{
				if (!poolInstance.GetPool()->Has(inspector.id))
				{
					continue;
				}

				componentLabel.clear();

				Strings::FormatTo(
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
					// auto* dataType = Cast<DataType>(type);
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
