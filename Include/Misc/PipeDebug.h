// Copyright 2015-2026 Piperift. All Rights Reserved.
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

// Mark debug tools as present for others
#define P_DEBUG_TOOLS 1

#include "Misc/PipeImGui.h"
#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Map.h"
#include "Pipe/Core/Set.h"
#include "PipeArrays.h"
#include "PipeECS.h"


namespace p
{
	///////////////////////////////////////////////////////////
	// Definition

#pragma region Inspection
	struct TypeInspection
	{
		using RowCallback =
		    TFunction<void(StringView label, void* data, TypeId typeId, bool& open)>;
		using ChildrenCallback = TFunction<void(void* data, TypeId typeId)>;

		RowCallback onDrawRow;
		ChildrenCallback onDrawChildren;
	};

	using InspectionDebugFlags = u32;
	enum InspectionDebugFlags_
	{
		IDF_None    = 0,         // -> No flags
		IDF_ViewAll = 1 << 0,    // -> View all properties, even those without PF_View or PF_Edit
		IDF_EditAll = 1 << 1,    // -> Allow editing on all properties, even those without PF_Edit
		IDF_ViewAndEditAll = IDF_ViewAll | IDF_EditAll

		// Any other flags up to 64 bytes are available to the user
	};
	P_DEFINE_FLAG_OPERATORS(InspectionDebugFlags_)

	struct DebugInspectionContext
	{
		struct PropStack
		{
			TypeId typeId;
			void* data = nullptr;
			i32 index  = NO_INDEX;
		};
		TMap<TypeId, TypeInspection> registeredTypes;
		TArray<PropStack> propStack;
		InspectionDebugFlags flags;
	};

	bool BeginInspection(
	    const char* label, v2 size = v2{0.f, 0.f}, InspectionDebugFlags flags = IDF_None);
	void EndInspection();

	void RegisterTypeInspection(TypeId typeId, TypeInspection::RowCallback onDrawRow,
	    TypeInspection::ChildrenCallback onDrawChildren = {});
	template<typename T>
	void RegisterTypeInspection(
	    TypeInspection::RowCallback onDrawRow, TypeInspection::ChildrenCallback onDrawChildren = {})
	{
		RegisterTypeInspection(GetTypeId<T>(), Move(onDrawRow), Move(onDrawChildren));
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
	bool InspectBeginCategory(p::StringView name, bool isLeaf, bool defaultOpen = false);
	void InspectEndCategory();
	void InspectProperties(void* data, TypeId typeId);
#pragma endregion Inspection


#pragma region ECS
	struct DebugECSInspector
	{
	protected:
		static i32 uniqueIdCounter;

	public:
		i32 uniqueId      = 0;
		Id id             = NoId;
		bool pendingFocus = false;
		ImGuiTextFilter filter;
		InspectionDebugFlags flags = IDF_ViewAll;


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

	struct DebugECSContext
	{
		TArray<TypeId> includeTypes;
		TArray<TypeId> excludeTypes;
		TArray<TypeId> previewTypes;

		ImGuiTextFilter filter;
		ImGuiTextFilter typeChooserFilter;

		TArray<DebugECSInspector> inspectors{1};

		// Updated on tick
		TArray<const IPool*> includePools;
		TArray<const IPool*> excludePools;
		TArray<const IPool*> previewPools;

		// Layout
		bool resetLayout    = true;
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

		bool initialized  = false;
		bool isFirstDebug = true;


		DebugContext() = default;
		DebugContext(EntityContext& ctx) : ctx{&ctx} {}
	};

	bool BeginDebug(DebugContext& Context);
	void EndDebug();


	///////////////////////////////////////////////////////////
	// Implementation
#ifdef P_DEBUG_IMPLEMENTATION
	namespace details
	{
		// We replace ImGui's GetCurrentWindow() in case "GImGui" has been overriden which can cause
		// link errors
		inline ImGuiWindow* GetCurrentWindowRead()
		{
			ImGuiContext& g = *ImGui::GetCurrentContext();
			return g.CurrentWindow;
		}
		inline ImGuiWindow* GetCurrentWindow()
		{
			ImGuiContext& g                = *ImGui::GetCurrentContext();
			g.CurrentWindow->WriteAccessed = true;
			return g.CurrentWindow;
		}
	}    // namespace details

	#define EnsureInsideDebug \
		P_EnsureMsg(currentContext, "No Debug context available! Forgot to call BeginDebug()?")

	static DebugContext* currentContext  = nullptr;
	constexpr LinearColor errorTextColor = LinearColor::FromHex(0xC13E3A);
	constexpr LinearColor includeColor   = LinearColor::FromHex(0x40A832);
	constexpr LinearColor excludeColor   = LinearColor::FromHex(0xA83632);
	constexpr LinearColor previewColor   = LinearColor::FromHex(0x3265A8);


	// For internal use only
	EntityContext& GetDebugCtx()
	{
		return *currentContext->ctx;
	}


	#pragma region Inspection

	bool BeginInspection(const char* label, v2 size, InspectionDebugFlags flags)
	{
		if (!EnsureInsideDebug)
		{
			return false;
		}

		const ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable
		                                 | ImGuiTableFlags_SizingStretchProp
		                                 | ImGuiTableFlags_PadOuterX;
		if (ImGui::BeginTable(label, 2, tableFlags, {size.x, size.y}))
		{
			ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 0.5f);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 1.f);

			currentContext->inspection.flags = flags;
			return true;
		}
		return false;
	}
	void EndInspection()
	{
		ImGui::EndTable();
	}

	void RegisterTypeInspection(TypeId typeId, TypeInspection::RowCallback onDrawRow,
	    TypeInspection::ChildrenCallback onDrawChildren)
	{
		if (EnsureInsideDebug)
		{
			currentContext->inspection.registeredTypes.Insert(
			    typeId, {Move(onDrawRow), Move(onDrawChildren)});
		}
	}
	#define P_DECLARE_COMMON_VALUE_TYPEINSPECTION(type, valueCode)        \
		p::RegisterTypeInspection<type>(                                  \
		    [](StringView label, void* data, TypeId typeId, bool& open) { \
			InspectSetKeyAsText(label);                                   \
			InspectSetValueColumn();                                      \
			type& value = *static_cast<type*>(data);                      \
			valueCode                                                     \
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
		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(v2_i32, {
			// a
			ImGui::InputScalarN("##value", ImGuiDataType_S32, &value, 2);
		});
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

		P_DECLARE_COMMON_VALUE_TYPEINSPECTION(Id, {
			String asString = ToString(value);
			ImGui::SetNextItemWidth(100.f);
			if (ImGui::InputText("##IdValue", asString, ImGuiInputTextFlags_EscapeClearsAll))
			{
				value = IdFromString(asString, &GetDebugCtx());
			}
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
		if (!EnsureInsideDebug || !data)
		{
			return;
		}

		auto& ins              = currentContext->inspection;
		const bool isContainer = p::HasTypeFlags(typeId, p::TF_Container);
		const bool isStruct    = p::HasTypeFlags(typeId, p::TF_Struct);

		ImGui::TableNextRow();
		ImGui::PushID(data);
		bool open = false;
		auto* it  = ins.registeredTypes.Find(typeId);
		if (it && it->onDrawRow)
		{
			it->onDrawRow(label, data, typeId, open);
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

		if (!ins.propStack.IsEmpty())    // Container item buttons
		{
			const auto& prop = ins.propStack[0];
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
				ins.propStack.Add({typeId, data, 0});
				for (i32 i = 0; i < size; ++i)
				{
					tmpLabel.clear();
					Strings::FormatTo(tmpLabel, "Index {}", i);
					Inspect(tmpLabel, ops->GetItem(data, i), ops->itemType);

					++ins.propStack.Last().index;
				}
				ins.propStack.RemoveLast();
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
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_AllowItemOverlap
		                               | ImGuiTreeNodeFlags_SpanAllColumns
		                               | (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0)
		                               | (defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0);
		bool bOpen = ImGui::TreeNodeEx(name.data(), flags);
		ImGui::Indent();
		return bOpen;
	}
	void InspectEndCategory()
	{
		ImGui::TreePop();
	}
	void InspectProperties(void* data, TypeId typeId)
	{
		if (!EnsureInsideDebug || !data)
		{
			return;
		}

		auto& ins          = currentContext->inspection;
		const bool viewAll = HasFlag(ins.flags, IDF_ViewAll);
		const bool editAll = HasFlag(ins.flags, IDF_EditAll);

		auto props = GetTypeProperties(typeId);
		if (!props.IsEmpty())
		{
			ins.propStack.Add({typeId, data, 0});
			for (const auto* prop : props)
			{
				if (viewAll || prop->HasFlag(PF_View))
				{
					ImGui::BeginDisabled(!editAll && !prop->HasFlag(PF_Edit));
					Inspect(prop->name.AsString(), prop->access(data), prop->typeId);
					ImGui::EndDisabled();

					++ins.propStack.Last().index;
				}
			}
			ins.propStack.RemoveLast();
		}
	}
	#pragma endregion Inspection


	#pragma region ECS
	i32 DebugECSInspector::uniqueIdCounter = 0;

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
			else if (auto version = id.GetVersion(); version > 0)
			{
				p::Strings::FormatTo(idText, "{}:{}", id.GetIndex(), version);
			}
			else
			{
				p::Strings::FormatTo(idText, "{}", id.GetIndex());
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
			static TArray<Id> children;
			children.Clear(false);
			if (const CParent* parent = access.TryGet<const CParent>(id))
			{
				children.Reserve(parent->children.Size());
				for (Id childId : parent->children)
				{
					if (access.IsValid(childId))
					{
						children.Add(childId);
					}
				}
			}
			const bool hasChildren = !children.IsEmpty();

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
			for (const IPool* previewPool : ecsDbg.previewPools)
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
				for (Id child : children)
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

					ImGui::CheckboxFlags("Edit Mode", &inspector.flags, IDF_EditAll);

					ImGui::SeparatorText("Advanced");
					ImGui::CheckboxFlags("View All Properties", &inspector.flags, IDF_ViewAll);
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

			if (valid && BeginInspection("##Inspector", {}, inspector.flags))
			{
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

					ImGuiTreeNodeFlags headerFlags =
					    ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAllColumns;
					void* data = poolInstance.GetPool()->TryGetVoid(inspector.id);
					if (!data)
					{
						headerFlags |= ImGuiTreeNodeFlags_Leaf;
					}
					ImGui::TableNextRow();
					InspectSetKeyColumn();
					if (ImGui::CollapsingHeader(componentLabel.c_str(), headerFlags))
					{
						ImGui::Indent();
						InspectProperties(data, poolInstance.componentId);
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
				{
					errorMsg = "No entity";
				}
				else
				{
					errorMsg = removed ? "Removed entity" : "Invalid entity";
				}

				auto regionAvail = ImGui::GetContentRegionAvail();
				auto textSize =
				    ImGui::CalcTextSize(errorMsg) + ImGui::GetStyle().FramePadding * 2.0f;

				ImGui::SetCursorPos(ImGui::GetCursorPos() + ((regionAvail - textSize) * 0.5f));
				ImGui::Text("%s", errorMsg);
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
			ImGui::Begin(label, open, flags | ImGuiWindowFlags_MenuBar);
			ImGui::PopStyleVar();

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Settings"))
				{
					if (ImGui::MenuItem("Reset Layout"))
					{
						ecsDbg.resetLayout = true;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGuiID dockspaceId = ImGui::GetID(label);
			if (ecsDbg.resetLayout || ImGui::DockBuilderGetNode(dockspaceId) == 0)
			{
				ecsDbg.resetLayout = false;
				// Clear any preexisting layouts associated with the ID we just chose
				ImGui::DockBuilderRemoveNode(dockspaceId);
				ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_KeepAliveOnly);

				ImGui::DockBuilderSplitNode(
				    dockspaceId, ImGuiDir_Right, 0.4f, &ecsDbg.rightDockId, &ecsDbg.leftDockId);

				ImGui::DockBuilderGetNode(ecsDbg.leftDockId)->LocalFlags |=
				    ImGuiDockNodeFlags_AutoHideTabBar;
				ImGui::DockBuilderFinish(dockspaceId);

				refreshingLayout = true;
			}
			ImGui::DockSpace(dockspaceId, ImVec2{0.f, 0.f});
			ImGui::End();
		}

		if (ecsDbg.leftDockId != 0)
		{
			ImGui::SetNextWindowDockID(
			    ecsDbg.leftDockId, refreshingLayout ? ImGuiCond_Always : ImGuiCond_Appearing);
		}
	#endif

		ImGui::SetNextWindowSize(ImVec2(400, 700), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(ImVec2(400.f, 500.f), ImVec2(FLT_MAX, FLT_MAX));

		const auto idRegistryFlags = hasDocking ? flags : 0;
		if (ImGui::Begin(
		        hasDocking ? "Id Registry" : label, hasDocking ? nullptr : open, idRegistryFlags))
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

				for (const IPool* pool : ecsDbg.excludePools)
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
				bool inspectorOpen = true;
				details::DrawEntityInspector(
				    "Inspect", inspector, i > 0 ? &inspectorOpen : nullptr);

				if (!inspectorOpen)
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
			{
				ImGui::PushTextColor(ImGui::GetTextColor().Shade(0.3f));
			}
			ImGuiTreeNodeFlags treeNodeFlags =
			    ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			if (typeProperties.IsEmpty())
			{
				treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;
			}
			const bool propsOpen = ImGui::TreeNodeEx(idText.data(), treeNodeFlags);
			if (!passedFilter)
			{
				ImGui::PopTextColor();
			}

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
			ImGui::CheckboxFlags("Native", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Native));
			ImGui::CheckboxFlags("Enum", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Enum));
			ImGui::CheckboxFlags("Struct", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Struct));
			ImGui::CheckboxFlags("Object", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Object));
			ImGui::CheckboxFlags(
			    "Container", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Container));
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
		currentContext->isFirstDebug = false;
		currentContext               = nullptr;
	}
#endif
};    // namespace p
