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
#include "Pipe/Core/Set.h"
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
	struct ECSDebugInspector
	{
		Id id             = NoId;
		bool pendingFocus = false;
		ImGuiTextFilter filter;

		bool operator==(const ECSDebugInspector& other) const
		{
			return id == other.id;
		}
		bool operator==(Id other) const
		{
			return id == other;
		}
		sizet GetHash() const
		{
			return p::GetHash(id);
		}
	};

	struct DebugECSContext
	{
		TArray<TypeId> includedTypes;
		TArray<TypeId> excludedTypes;
		TArray<TypeId> previewedTypes;
		ImGuiTextFilter filter;
		ImGuiTextFilter typeChooserFilter;

		TSet<ECSDebugInspector> inspectors;
	};
	struct DebugReflectContext
	{
		ImGuiTextFilter filter;
		TypeFlags typeFlagsFilter = TF_Native | TF_Enum | TF_Struct | TF_Object;
		String typeFlags;
	};

	struct DebugContext
	{
		DebugECSContext ecs;
		DebugReflectContext reflect;

		EntityContext* ctx = nullptr;


		DebugContext() = default;
		DebugContext(EntityContext& ctx) : ctx{&ctx} {}
	};


	bool BeginDebug(DebugContext& Context);
	void EndDebug();

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

	using DrawNodeAccess = TAccessRef<CParent, CChild>;
	namespace details
	{
		bool ChooseTypePopup(const char* label, ImGuiTextFilter& filter, TypeId& selectedTypeId)
		{
			bool selectedAny = false;
			if (ImGui::BeginPopup(label))
			{
				if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
				    && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
				{
					ImGui::SetKeyboardFocusHere(0);
				}

				filter.Draw("##Filter", ImGui::GetContentRegionAvail().x);
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::BeginChild("types", {300.f, 400.f});
				String typeName;
				for (const TypeId typeId : GetRegisteredTypeIds())
				{
					typeName.assign(GetTypeName(typeId));
					if (filter.PassFilter(typeName.data()) && ImGui::Selectable(typeName.data()))
					{
						selectedTypeId = typeId;
						selectedAny    = true;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndChild();
				ImGui::EndPopup();
			}
			return selectedAny;
		}

		void DrawPoolsList(StringView id, DebugECSContext& ecsDbg, TArray<TypeId>& poolTypes,
		    bool canModify = false)
		{
			ImGui::PushID(id);

			String typeName;
			TArray<TypeId> typesToRemove;
			for (TypeId typeId : poolTypes)
			{
				typeName.assign(GetTypeName(typeId));
				if (ImGui::Button(typeName.data()))
				{
					typesToRemove.Add(typeId);
				}
				ImGui::SameLine();
			}
			poolTypes.Remove(typesToRemove);

			if (ImGui::Button("+"))
			{
				ecsDbg.typeChooserFilter.Clear();
				ImGui::OpenPopup("Select Type##popup");
			}
			TypeId selectedType;
			if (ChooseTypePopup("Select Type##popup", ecsDbg.typeChooserFilter, selectedType))
			{
				poolTypes.Add(selectedType);
			}
			ImGui::PopID();
		}

		void DrawPoolsFilters(DebugECSContext& ecsDbg)
		{
			ImGui::PushStyleCompact();
			ImGui::BeginTable("poolFilterTable", 2, ImGuiTableFlags_SizingStretchProp);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Include");
			ImGui::TableNextColumn();
			DrawPoolsList("Include", ecsDbg, ecsDbg.includedTypes, true);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Exclude");
			ImGui::TableNextColumn();
			DrawPoolsList("Exclude", ecsDbg, ecsDbg.excludedTypes, true);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Preview");
			ImGui::TableNextColumn();
			DrawPoolsList("Preview", ecsDbg, ecsDbg.previewedTypes, true);
			ImGui::EndTable();
			ImGui::PopStyleCompact();
		}

		void DrawIdInTable(DrawNodeAccess access, Id id, DebugECSContext& ecsDbg)
		{
			static p::String idText;
			idText.clear();
			if (id == NoId)
			{
				idText = "Invalid";
			}
			else if (auto version = p::GetIdVersion(id); version > 0)
			{
				p::Strings::FormatTo(idText, "{}:{}", p::GetIdIndex(id), version);
			}
			else
			{
				p::Strings::FormatTo(idText, "{}", p::GetIdIndex(id));
			}

			if (!ecsDbg.filter.PassFilter(idText.c_str(), idText.c_str() + idText.size()))
			{
				return;
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);    // View
			static p::String inspectLabel;
			inspectLabel.clear();

			const bool inspected = ecsDbg.inspectors.Contains(id);
			const char* icon     = inspected ? " × " : " » ";
			p::Strings::FormatTo(inspectLabel, "{}##{}", icon, id);
			ImGui::PushTextColor(
			    inspected ? ImGui::GetTextColor() : ImGui::GetTextColor().Translucency(0.3f));
			if (ImGui::Button(inspectLabel.c_str()))
			{
				//
			}
			ImGui::PopTextColor();


			ImGui::TableSetColumnIndex(1);    // Id
			const CParent* parent = access.TryGet<const CParent>(id);
			bool hasChildren      = parent && !parent->children.IsEmpty();

			bool open = false;
			static p::Tag font{"WorkSans"};
			if (hasChildren)
			{
				open = ImGui::TreeNodeEx(idText.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
			}
			else
			{
				ImGui::SameLine(0, 18.f);
				ImGui::Text(idText);
			}

			ImGui::TableSetColumnIndex(2);    // Previews


			if (hasChildren && open)
			{
				for (Id child : parent->children)
				{
					DrawIdInTable(access, child, ecsDbg);
				}

				ImGui::TreePop();
			}
		}
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
			if (ns.size() > 0)
			{
				ImGui::PushTextColor(ImGui::GetTextColor().Shade(0.3f));
				ImGui::Text(ns);
				ImGui::PopTextColor();
				ImGui::SameLine(0, 10.f);
			}
			ImGui::Text(name);

			ImGui::TableSetColumnIndex(2);    // Flags
			ctx.typeFlags.clear();
			GetEnumFlagName<TypeFlags_>(TypeFlags_(GetTypeFlags(type)), ctx.typeFlags);
			ImGui::Text(ctx.typeFlags);

			TypeId parentId = GetTypeParent(type);
			if (parentId.IsValid())
			{
				ImGui::TableSetColumnIndex(3);    // Parent
				rawName = GetTypeName(parentId);
				name    = RemoveNamespace(rawName, ns);
				if (ns.size() > 0)
				{
					ImGui::PushTextColor(ImGui::GetTextColor().Shade(0.3f));
					ImGui::Text(ns);
					ImGui::PopTextColor();
					ImGui::SameLine(0, 10.f);
				}
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

	void DrawIdRegistry(const char* label, bool* open, ImGuiWindowFlags flags)
	{
		if (!EnsureInsideDebug)
		{
			return;
		}

		auto& ecsDbg = currentContext->ecs;

		ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(label, open, flags))
		{
			ImGui::DrawFilterWithHint(
			    ecsDbg.filter, "##filter", "Search...", ImGui::GetContentRegionAvail().x);
			details::DrawPoolsFilters(ecsDbg);

			ImGui::Separator();
			ImGui::Dummy({0.f, 0.f});

			TArray<Id> ids;
			{    // Filtering
				if (ecsDbg.includedTypes.IsEmpty())
				{
					ids.AddUninitialized(GetDebugCtx().Size());
					i32 idx = 0;
					GetDebugCtx().Each([&ids, &idx](Id id) {
						ids[idx] = id;
						++idx;
					});
				}
				else
				{
					TArray<const BasePool*> includedPools;
					GetDebugCtx().GetPools(ecsDbg.includedTypes, includedPools);
					FindAllIdsWith(includedPools, ids);
				}

				TArray<BasePool*> excludedPools;
				GetDebugCtx().GetPools(ecsDbg.excludedTypes, excludedPools);
				for (const BasePool* pool : excludedPools)
				{
					ExcludeIdsWithStable(pool, ids, false);
				}
			}


			ImGui::Text("%i entities", ids.Size());

			static const ImGuiTableFlags tableFlags =
			    ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable
			    | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;
			if (ImGui::BeginTable("entityTable", 3, tableFlags))
			{
				ImGui::TableSetupColumn("View", ImGuiTableColumnFlags_IndentDisable
				                                    | ImGuiTableColumnFlags_WidthFixed
				                                    | ImGuiTableColumnFlags_NoResize);    // Inspect
				ImGui::TableSetupColumn(
				    "Id", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_IndentEnable);
				ImGui::TableSetupColumn("Previews");

				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableHeadersRow();

				DrawNodeAccess access{GetDebugCtx()};

				ImGuiListClipper clipper;
				clipper.Begin(ids.Size());
				while (clipper.Step())
				{
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
					{
						details::DrawIdInTable(access, ids[i], ecsDbg);
					}
				}
				ImGui::EndTable();
			}
			ImGui::End();
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
		if (ImGui::Button("Settings"))
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
			if (ImGui::BeginMenu("Settings"))
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
			ImGui::DrawFilterWithHint(inspector.filter, "##filter", "Search components...");

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
