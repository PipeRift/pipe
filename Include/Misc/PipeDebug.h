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
#include "Pipe/Core/Map.h"
#include "Pipe/Core/Set.h"
#include "PipeArrays.h"
#include "PipeECS.h"


namespace p
{
	///////////////////////////////////////////////////////////
	// Definition

#pragma region Inspection
	struct DebugInspectionContext
	{
		using Callback = TFunction<void(StringView label, void* data, TypeId typeId)>;
		TMap<TypeId, Callback> registeredTypes;
	};
	struct DebugECSInspector
	{
	protected:
		static i32 uniqueIdCounter;

	public:
		i32 uniqueId      = 0;
		Id id             = NoId;
		bool pendingFocus = false;
		ImGuiTextFilter filter;


		DebugECSInspector() : uniqueId{++uniqueIdCounter} {}
		bool operator==(const DebugECSInspector& other) const
		{
			return id == other.id;
		}
		bool operator==(Id other) const
		{
			return id == other;
		}
		friend sizet GetHash(const DebugECSInspector& inspector)
		{
			return p::GetHash(inspector.id);
		}
	};

	bool BeginInspection(const char* name, v2 size = v2{0.f, 0.f});
	void EndInspection();

	void RegisterTypeInspection(TypeId typeId, const DebugInspectionContext::Callback& callback);
	template<typename T, typename Callback>
	void RegisterTypeInspection(Callback callback)
	{
		RegisterTypeInspection(GetTypeId<T>(),
		    [callback](StringView label, void* data, TypeId typeId) {
			callback(label, *static_cast<T*>(data));
		});
	}
	void RegisterPipeTypeInspections();
	void RemoveTypeInspection(TypeId typeId);

	void Inspect(StringView label, void* data, TypeId typeId);
	template<typename T>
	void Inspect(StringView label, T* data)
	{
		Inspect(label, data, GetTypeId<T>());
	}
	void InspectSetKeyColumn();
	void InspectSetKeyAsText(StringView label);
	void InspectSetValueColumn();
#pragma endregion Inspection


#pragma region ECS
	struct DebugECSContext
	{
		TArray<TypeId> includeTypes;
		TArray<TypeId> excludeTypes;
		TArray<TypeId> previewTypes;

		ImGuiTextFilter filter;
		ImGuiTextFilter typeChooserFilter;

		TArray<DebugECSInspector> inspectors{1};

		// Updated on tick
		TArray<const BasePool*> includePools;
		TArray<const BasePool*> excludePools;
		TArray<const BasePool*> previewPools;

		// Layout
		ImGuiID leftDockId  = 0;
		ImGuiID rightDockId = 0;
	};

	namespace details
	{
		void DrawEntityInspector(StringView label, DebugECSInspector& inspector,
		    bool* open = nullptr, ImGuiWindowFlags flags = 0);
	}
	void DrawIdRegistry(
	    const char* label = "Id Registry", bool* open = nullptr, ImGuiWindowFlags flags = 0);

#pragma endregion ECS

#pragma region Reflection
	struct DebugReflectContext
	{
		ImGuiTextFilter filter;
		TypeFlags typeFlagsFilter = TF_Native | TF_Enum | TF_Struct | TF_Object;
		String typeFlags;
		String propertyFlags;
	};

	void DrawReflection(
	    const char* label = "Reflection", bool* open = nullptr, ImGuiWindowFlags flags = 0);
#pragma endregion Reflection


	struct DebugContext
	{
		DebugInspectionContext inspection;
		DebugECSContext ecs;
		DebugReflectContext reflect;

		EntityContext* ctx = nullptr;

		bool initialized = false;


		DebugContext() = default;
		DebugContext(EntityContext& ctx) : ctx{&ctx} {}
	};

	bool BeginDebug(DebugContext& Context);
	void EndDebug();


	///////////////////////////////////////////////////////////
	// Implementation
#ifdef P_DEBUG_IMPLEMENTATION

	#define EnsureInsideDebug \
		P_EnsureMsg(currentContext, "No Debug context available! Forgot to call BeginDebug()?")


	static DebugContext* currentContext  = nullptr;
	constexpr LinearColor errorTextColor = LinearColor::FromHex(0xC13E3A);
	constexpr LinearColor includeColor   = LinearColor::FromHex(0x40A832);
	constexpr LinearColor excludeColor   = LinearColor::FromHex(0xA83632);
	constexpr LinearColor previewColor   = LinearColor::FromHex(0x3265A8);


	#pragma region Inspection
	i32 DebugECSInspector::uniqueIdCounter = 0;


	bool BeginInspection(const char* name, v2 size)
	{
		return false;
	}
	void EndInspection() {}

	void RegisterTypeInspection(TypeId typeId, const DebugInspectionContext::Callback& callback)
	{
		if (EnsureInsideDebug)
		{
			currentContext->inspection.registeredTypes.Insert(typeId, callback);
		}
	}
	void RegisterPipeTypeInspections()
	{
		RegisterTypeInspection<bool>([](StringView label, bool& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::Checkbox("##value", &data);
		});
		RegisterTypeInspection<String>([](StringView label, String& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::InputText("##value", data);
		});
		RegisterTypeInspection<StringView>([](StringView label, StringView& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::InputText("##value", const_cast<char*>(data.data()), data.size(),
			    ImGuiInputTextFlags_ReadOnly);
		});
		RegisterTypeInspection<i8>([](StringView label, i8& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::InputScalar("##value", ImGuiDataType_S8, &data);
		});
		RegisterTypeInspection<u8>([](StringView label, u8& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::InputScalar("##value", ImGuiDataType_U8, &data);
		});
		RegisterTypeInspection<i32>([](StringView label, i32& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::InputScalar("##value", ImGuiDataType_S32, &data);
		});
		RegisterTypeInspection<u32>([](StringView label, u32& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::InputScalar("##value", ImGuiDataType_U32, &data);
		});
		RegisterTypeInspection<i64>([](StringView label, i64& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::InputScalar("##value", ImGuiDataType_S64, &data);
		});
		RegisterTypeInspection<u64>([](StringView label, u64& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::InputScalar("##value", ImGuiDataType_U64, &data);
		});
		RegisterTypeInspection<float>([](StringView label, float& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::InputScalar("##value", ImGuiDataType_Float, &data);
		});
		RegisterTypeInspection<double>([](StringView label, double& data) {
			InspectSetKeyAsText(label);
			InspectSetValueColumn();
			ImGui::InputScalar("##value", ImGuiDataType_Double, &data);
		});
	}
	void RemoveTypeInspection(TypeId typeId)
	{
		if (EnsureInsideDebug)
		{
			currentContext->inspection.registeredTypes.Remove(typeId);
		}
	}
	void Inspect(StringView label, void* data, TypeId typeId)
	{
		if (!EnsureInsideDebug)
		{
			return;
		}

		if (auto* cb = currentContext->inspection.registeredTypes.Find(typeId))
		{
			(*cb)(label, data, typeId);
		}
	}
	void InspectSetKeyColumn()
	{
		ImGui::TableSetColumnIndex(0);
	}
	void InspectSetKeyAsText(StringView label)
	{
		InspectSetKeyColumn();
		ImGui::TextUnformatted(label.data(), label.data() + label.size());
	}
	void InspectSetValueColumn()
	{
		ImGui::TableSetColumnIndex(1);
	}
	#pragma endregion Inspection


	#pragma region ECS
	// For internal use only
	EntityContext& GetDebugCtx()
	{
		return *currentContext->ctx;
	}

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
		    LinearColor color, bool canModify = false)
		{
			ImGui::PushID(id);
			ImGui::PushButtonColor(color);

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

			if (ImGui::Button(" + "))
			{
				ecsDbg.typeChooserFilter.Clear();
				ImGui::OpenPopup("Select Type##popup");
			}
			TypeId selectedType;
			if (ChooseTypePopup("Select Type##popup", ecsDbg.typeChooserFilter, selectedType))
			{
				poolTypes.Add(selectedType);
			}
			ImGui::PopButtonColor();
			ImGui::PopID();
		}

		void DrawPoolsFilters(DebugECSContext& ecsDbg)
		{
			ImGui::PushStyleCompact();
			ImGui::BeginTable("poolFilterTable", 2, ImGuiTableFlags_SizingStretchProp);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Include");
			ImGui::TableNextColumn();
			DrawPoolsList("Include", ecsDbg, ecsDbg.includeTypes, includeColor, true);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Exclude");
			ImGui::TableNextColumn();
			DrawPoolsList("Exclude", ecsDbg, ecsDbg.excludeTypes, excludeColor, true);
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Preview");
			ImGui::TableNextColumn();
			DrawPoolsList("Preview", ecsDbg, ecsDbg.previewTypes, previewColor, true);
			ImGui::EndTable();
			ImGui::PopStyleCompact();
		}

		void InspectEntity(DebugECSContext& ecsDbg, Id id, bool useMainInspector = true)
		{
			if (!useMainInspector || ecsDbg.inspectors.IsEmpty() || ImGui::IsKeyDown(ImGuiMod_Ctrl))
			{
				ecsDbg.inspectors[ecsDbg.inspectors.Add()].id = id;
			}
			else
			{
				ecsDbg.inspectors[0].id           = id;
				ecsDbg.inspectors[0].pendingFocus = true;
			}
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

			static DebugECSInspector searchInspector;
			searchInspector.id   = id;
			const bool inspected = ecsDbg.inspectors.Contains(searchInspector);
			const char* icon     = inspected ? " × " : "-->";
			p::Strings::FormatTo(inspectLabel, "{}##{}", icon, id);
			ImGui::PushTextColor(
			    inspected ? ImGui::GetTextColor() : ImGui::GetTextColor().Translucency(0.3f));
			ImGui::PushStyleCompact();
			if (ImGui::Button(inspectLabel.c_str()))
			{
				if (inspected)
				{
					for (i32 i = 0; i < ecsDbg.inspectors.Size(); ++i)
					{
						auto& inspector = ecsDbg.inspectors[i];
						if (inspector.id == id)
						{
							if (i == 0)
							{
								inspector.id = NoId;
							}
							else
							{
								ecsDbg.inspectors.RemoveAtSwap(i);
								--i;
							}
						}
					}
				}
				else
				{
					InspectEntity(ecsDbg, id);
				}
			}
			ImGui::PopStyleCompact();
			ImGui::PopTextColor();


			ImGui::TableSetColumnIndex(1);    // Id
			const CParent* parent = access.TryGet<const CParent>(id);
			bool hasChildren      = parent && !parent->children.IsEmpty();

			bool open = false;
			static p::Tag font{"WorkSans"};
			if (hasChildren)
			{
				open = ImGui::TreeNodeEx(idText.c_str(), ImGuiTreeNodeFlags_SpanAllColumns);
			}
			else
			{
				ImGui::SameLine(0, 18.f);
				ImGui::Text(idText);
			}

			ImGui::TableSetColumnIndex(2);    // Previews
			ImGui::PushStyleCompact();
			ImGui::PushStyleCompact();
			static String poolName;
			ImGui::PushButtonColor(previewColor);
			for (const BasePool* previewPool : ecsDbg.previewPools)
			{
				if (previewPool && previewPool->Has(id))
				{
					poolName.assign(GetTypeName(previewPool->GetTypeId()));
					ImGui::Button(poolName.data());
					ImGui::SameLine();
				}
			}
			ImGui::PopButtonColor();
			ImGui::PopStyleCompact();
			ImGui::PopStyleCompact();


			if (hasChildren && open)
			{
				for (Id child : parent->children)
				{
					DrawIdInTable(access, child, ecsDbg);
				}

				ImGui::TreePop();
			}
		}

		void DrawEntityInspector(
		    StringView label, DebugECSInspector& inspector, bool* open, ImGuiWindowFlags flags)
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
			Strings::FormatTo(name, "{}: {}{}###inspector{}", label, inspector.id,
			    removed ? " (removed)" : "", inspector.uniqueId);

			if (inspector.pendingFocus)
			{
				ImGui::SetNextWindowFocus();
				inspector.pendingFocus = false;
			}

			ImGui::SetNextWindowPos(
			    ImGui::GetCursorScreenPos() + ImVec2(20, 20), ImGuiCond_Appearing);
			ImGui::SetNextWindowSizeConstraints(ImVec2(300.f, 200.f), ImVec2(800, FLT_MAX));
			ImGui::Begin(name.c_str(), open, ImGuiWindowFlags_MenuBar);

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
					        ImGuiInputTextFlags_EnterReturnsTrue
					            | ImGuiInputTextFlags_EscapeClearsAll))
					{
						nextId = IdFromString(asString, &GetDebugCtx());
					}
					ImGui::EndMenu();
				}
				ImGui::DrawFilterWithHint(
				    inspector.filter, "##filter", "Search components...", -36.f);

				if (ImGui::MenuItem("-->"))
				{
					clone = true;
				}
				ImGui::EndMenuBar();
			}

			if (valid)
			{
				BeginInspection("EntityInspector");
				String componentLabel;
				for (const auto& poolInstance : GetDebugCtx().GetPools())
				{
					if (!poolInstance.GetPool()->Has(inspector.id))
					{
						continue;
					}

					componentLabel.clear();

					Strings::FormatTo(componentLabel, "{}",
					    RemoveNamespace(GetTypeName(poolInstance.componentId)));

					if (!inspector.filter.PassFilter(componentLabel.c_str()))
					{
						continue;
					}

					ImGuiTreeNodeFlags headerFlags = ImGuiTreeNodeFlags_DefaultOpen;
					void* data = poolInstance.GetPool()->TryGetVoid(inspector.id);
					if (!data)
					{
						headerFlags |= ImGuiTreeNodeFlags_Leaf;
					}
					if (ImGui::CollapsingHeader(componentLabel.c_str(), headerFlags))
					{
						ImGui::Indent();
						if (data)
						{
							Inspect(componentLabel.c_str(), data, poolInstance.componentId);
						}
						ImGui::Unindent();
					}
				}
				EndInspection();
			}
			else
			{
				ImGui::PushTextColor(errorTextColor);
				const char* errorMsg;
				if (inspector.id == NoId)
					errorMsg = "No entity";
				else
					errorMsg = removed ? "Removed entity" : "Invalid entity";

				auto regionAvail = ImGui::GetContentRegionAvail();
				auto textSize =
				    ImGui::CalcTextSize(errorMsg) + ImGui::GetStyle().FramePadding * 2.0f;

				ImGui::SetCursorPos(ImGui::GetCursorPos() + ((regionAvail - textSize) * 0.5f));
				ImGui::Text(errorMsg);
				ImGui::PopTextColor();
			}

			ImGui::End();

			// Update after drawing
			if (nextId != inspector.id)
			{
				inspector.id = nextId;
			}

			if (clone)
			{
				auto& ecsDbg = currentContext->ecs;
				InspectEntity(ecsDbg, inspector.id, false);
			}
		}
	}    // namespace details


	void DrawIdRegistry(const char* label, bool* open, ImGuiWindowFlags flags)
	{
		if (!EnsureInsideDebug)
		{
			return;
		}

		auto& ecsDbg = currentContext->ecs;

		bool hasDocking = false;
	#ifdef IMGUI_HAS_DOCK
		bool refreshingLayout = false;
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			hasDocking = true;
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin(label, open, flags);
			ImGui::PopStyleVar();
			{
				ImGuiID dockspaceId   = ImGui::GetID(label);
				static bool everBuilt = false;
				if (!everBuilt || ImGui::DockBuilderGetNode(dockspaceId) == 0)
				{
					everBuilt = true;
					// Clear any preexisting layouts associated with the ID we just chose
					ImGui::DockBuilderRemoveNode(dockspaceId);
					ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_KeepAliveOnly);

					ImGui::DockBuilderSplitNode(
					    dockspaceId, ImGuiDir_Right, 0.4f, &ecsDbg.rightDockId, &ecsDbg.leftDockId);

					ImGui::DockBuilderGetNode(ecsDbg.leftDockId)->LocalFlags |=
					    ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoUndocking;
					ImGui::DockBuilderFinish(dockspaceId);

					refreshingLayout = true;
				}
				ImGui::DockSpace(dockspaceId, ImVec2{0.f, 0.f}, ImGuiDockNodeFlags_NoSplit);
				ImGui::End();
			}
		}

		if (ecsDbg.leftDockId != 0)
		{
			ImGui::SetNextWindowDockID(ecsDbg.leftDockId, ImGuiCond_Always);
		}
	#endif

		ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(ImVec2(400.f, 500.f), ImVec2(FLT_MAX, FLT_MAX));

		if (ImGui::Begin(hasDocking ? "Id Registry" : label, hasDocking ? nullptr : open, flags))
		{
			ImGui::DrawFilterWithHint(
			    ecsDbg.filter, "##filter", "Search...", ImGui::GetContentRegionAvail().x);
			details::DrawPoolsFilters(ecsDbg);

			ImGui::Separator();
			ImGui::Dummy({0.f, 0.f});

			{    // Cache pools
				ecsDbg.includePools.Clear(false);
				ecsDbg.excludePools.Clear(false);
				ecsDbg.previewPools.Clear(false);
				GetDebugCtx().GetPools(ecsDbg.includeTypes, ecsDbg.includePools);
				GetDebugCtx().GetPools(ecsDbg.excludeTypes, ecsDbg.excludePools);
				GetDebugCtx().GetPools(ecsDbg.previewTypes, ecsDbg.previewPools);
			}

			TArray<Id> ids;
			{    // Filtering
				if (ecsDbg.includeTypes.IsEmpty())
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
					FindAllIdsWith(ecsDbg.includePools, ids);
				}

				for (const BasePool* pool : ecsDbg.excludePools)
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
				ImGui::TableSetupColumn("View",
				    ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed
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

		{    // DrawECSInspectors
			String uniqueId;
			for (i32 i = 0; i < ecsDbg.inspectors.Size(); ++i)
			{
				auto& inspector = ecsDbg.inspectors[i];
	#ifdef IMGUI_HAS_DOCK
				if (ecsDbg.rightDockId != 0)
				{
					ImGui::SetNextWindowDockID(ecsDbg.rightDockId,
					    refreshingLayout ? ImGuiCond_Always : ImGuiCond_Appearing);
				}
	#endif
				bool open = true;
				details::DrawEntityInspector("Inspect", inspector, i > 0 ? &open : nullptr);

				if (!open)
				{
					ecsDbg.inspectors.RemoveAtSwap(i);
					--i;
				}
			}
		}

	#ifdef IMGUI_HAS_DOCK
		refreshingLayout = false;
	#endif
	}
	#pragma endregion ECS


	#pragma region Reflection
	namespace details
	{
		void DrawProperty(DebugReflectContext& ctx, TypeId type, const TypeProperty& property)
		{
			static String idText;
			idText.clear();
			Strings::FormatTo(idText, "{}", type);

			StringView name = property.name.AsString();
			if (!ctx.filter.PassFilter(name.data(), name.data() + name.size()))
			{
				return;
			}

			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);    // Id

			ImGui::TableSetColumnIndex(1);    // Name
			ImGui::Text(name);

			ImGui::TableSetColumnIndex(2);    // Flags
			ctx.propertyFlags.clear();
			GetEnumFlagName<PropertyFlags_>(PropertyFlags_(property.flags), ctx.propertyFlags);
			ImGui::Text(ctx.propertyFlags);
		}

		void DrawType(DebugReflectContext& ctx, TypeId type)
		{
			if (!HasAnyTypeFlags(type, ctx.typeFlagsFilter))
			{
				return;
			}

			const auto& typeProperties = GetOwnTypeProperties(type);

			static String idText;
			idText.clear();
			Strings::FormatTo(idText, "{}", type);

			bool passedFilter  = true;
			StringView rawName = GetTypeName(type);
			if (!ctx.filter.PassFilter(idText.c_str(), idText.c_str() + idText.size())
			    && !ctx.filter.PassFilter(rawName.data(), rawName.data() + rawName.size()))
			{
				bool anyPropsPassedFilter = false;
				for (const auto& prop : typeProperties)
				{
					StringView name = prop.name.AsString();
					if (ctx.filter.PassFilter(name.data(), name.data() + name.size()))
					{
						anyPropsPassedFilter = true;
						break;
					}
				}

				if (!anyPropsPassedFilter)
				{
					return;
				}
				passedFilter = false;
			}

			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);    // Id
			if (!passedFilter)
				ImGui::PushTextColor(ImGui::GetTextColor().Shade(0.3f));
			ImGuiTreeNodeFlags treeNodeFlags =
			    ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			if (typeProperties.IsEmpty())
			{
				treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
			}
			const bool propsOpen = ImGui::TreeNodeEx(idText.data(), treeNodeFlags);
			if (!passedFilter)
				ImGui::PopTextColor();

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

			if (propsOpen)
			{
				for (const auto& prop : typeProperties)
				{
					DrawProperty(ctx, type, prop);
				}
			}
		}
	}    // namespace details

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

		static ImGuiTableFlags tableFlags =
		    ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable
		    | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;
		ImGui::BeginChild("typesTableChild", ImVec2(0.f, ImGui::GetContentRegionAvail().y));
		if (ImGui::BeginTable("typesTable", 4, tableFlags))
		{
			ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_IndentEnable);
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Flags");
			ImGui::TableSetupColumn("Parent");

			ImGui::TableSetupScrollFreeze(0, 1);
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
	#pragma endregion Reflection


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

		if (!currentContext->initialized)
		{
			RegisterPipeTypeInspections();
			currentContext->initialized = true;
		}
		return true;
	}
	void EndDebug()
	{
		P_CheckMsg(currentContext,
		    "Called EndDebug() but there was no current ECS Debug Context! Forgot "
		    "to call "
		    "BeginDebug()?");
		currentContext = nullptr;
	}
#endif
};    // namespace p
