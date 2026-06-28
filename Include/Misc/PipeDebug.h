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

// Optional ImPlot support for arena debugger
#ifdef IMPLOT_VERSION
	#include "implot.h"
#endif

// Mark debug tools as present for others
#define P_DEBUG_TOOLS 1

#include "Misc/PipeImGui.h"
#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Map.h"
#include "Pipe/Core/Set.h"
#include "PipeContainers.h"
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
		ImGuiTextFilter* filter = nullptr;
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
		bool drawAsList = true;    // Draw as list or tree

		TSet<Id> openIds;
		ImGuiTextFilter filter;
		ImGuiTextFilter typeChooserFilter;

		TArray<DebugECSInspector> inspectors{};

		// Multi-selection
		TArray<Id> selectedIds;
		Id lastSelectedId = NoId;
		Id hoveredId      = NoId;
		TArray<Id, 1> contextMenuIds;

		// Updated on tick
		TArray<Id> ids;    // Ids being displayed in the id list
		TArray<const IPool*> includePools;
		TArray<const IPool*> excludePools;
		TArray<const IPool*> previewPools;
		TMap<Id, i32> idToDisplayIndex;

		// Layout
		bool resetLayout    = true;
		ImGuiID leftDockId  = 0;
		ImGuiID rightDockId = 0;

		// Context menu extensibility
		using ContextMenuCallback = TFunction<void(TView<const Id> ids)>;
		TArray<ContextMenuCallback> contextMenuCallbacks;
	};

	namespace details
	{
		void DrawEntityInspector(StringView label, DebugECSInspector& inspector,
		    bool* open = nullptr, ImGuiWindowFlags flags = 0);
	}
	void DrawIdRegistry(
	    const char* label = "Id Registry", bool* open = nullptr, ImGuiWindowFlags flags = 0);

	bool IsInspectingId(const DebugECSContext& ecsDbg, Id id);
	void StartInspectingId(DebugECSContext& ecsDbg, Id id, bool useMainInspector = true);
	void StopInspectingId(DebugECSContext& ecsDbg, Id id);
	void DrawInspectIdButton(DebugECSContext& ecsDbg, Id id);
	void AddIdContextMenuCallback(DebugECSContext::ContextMenuCallback callback);
#pragma endregion ECS


#pragma region Reflection
	struct DebugReflectContext
	{
		ImGuiTextFilter filter;
		bool bFilterWithAny       = true;
		TypeFlags typeFlagsFilter = TF_Native | TF_Enum | TF_Struct | TF_Object;
		String typeFlags;
		String propertyFlags;
	};

	void DrawReflection(
	    const char* label = "Reflection", bool* open = nullptr, ImGuiWindowFlags flags = 0);
#pragma endregion Reflection


#pragma region Memory
	struct DebugMemoryContext
	{
		i32 selectedArena = -1;
		bool horizontal   = true;
		bool showHex      = true;
		bool showAscii    = true;

		// Zoom/pan for memory graph
		float zoom     = 1.0f;
		float pan      = 0.0f; // fraction of range
		bool panning   = false;
		ImVec2 panPos;
		float panStart = 0.0f;

		// Cached arena blocks for rendering
		struct ArenaBlockInfo
		{
			Arena* arena     = nullptr;
			TypeId typeId    = TypeId{};
			void* blockBegin = nullptr;
			sizet blockSize  = 0;
			sizet usedSize   = 0;
			sizet freeSize   = 0;
			TArray<void*> blocks;
			TArray<sizet> blockSizes;
		};
		TArray<ArenaBlockInfo> arenaInfos;
	};

	void DrawMemory(const char* label = "Memory", bool* open = nullptr, ImGuiWindowFlags flags = 0);
#pragma endregion Memory

	struct DebugContext
	{
		DebugInspectionContext inspection;
		DebugECSContext ecs;
		DebugReflectContext reflect;
		DebugMemoryContext memory;

		IdContext* ctx = nullptr;

		bool initialized  = false;
		bool isFirstDebug = true;


		DebugContext() = default;
		DebugContext(IdContext& ctx) : ctx{&ctx} {}
	};

	bool BeginDebug(DebugContext& Context);
	void EndDebug();


	///////////////////////////////////////////////////////////
	// Implementation
#ifdef P_DEBUG_IMPLEMENTATION

	// Even if the imgui implementation doesn't have math operators enabled, we force those we need
	#if !defined(IMGUI_DEFINE_MATH_OPERATORS) && !defined(IMGUI_DEFINE_MATH_OPERATORS_IMPLEMENTED)
	// ImVec2 operators
	inline ImVec2 operator*(const ImVec2& lhs, const float rhs)
	{
		return ImVec2(lhs.x * rhs, lhs.y * rhs);
	}
	inline ImVec2 operator/(const ImVec2& lhs, const float rhs)
	{
		return ImVec2(lhs.x / rhs, lhs.y / rhs);
	}
	inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
	{
		return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
	}
	inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)
	{
		return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
	}
	inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs)
	{
		return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y);
	}
	inline ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs)
	{
		return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y);
	}
	inline ImVec2 operator-(const ImVec2& lhs)
	{
		return ImVec2(-lhs.x, -lhs.y);
	}
	inline ImVec2& operator*=(ImVec2& lhs, const float rhs)
	{
		lhs.x *= rhs;
		lhs.y *= rhs;
		return lhs;
	}
	inline ImVec2& operator/=(ImVec2& lhs, const float rhs)
	{
		lhs.x /= rhs;
		lhs.y /= rhs;
		return lhs;
	}
	inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs)
	{
		lhs.x += rhs.x;
		lhs.y += rhs.y;
		return lhs;
	}
	inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs)
	{
		lhs.x -= rhs.x;
		lhs.y -= rhs.y;
		return lhs;
	}
	inline ImVec2& operator*=(ImVec2& lhs, const ImVec2& rhs)
	{
		lhs.x *= rhs.x;
		lhs.y *= rhs.y;
		return lhs;
	}
	inline ImVec2& operator/=(ImVec2& lhs, const ImVec2& rhs)
	{
		lhs.x /= rhs.x;
		lhs.y /= rhs.y;
		return lhs;
	}
	inline bool operator==(const ImVec2& lhs, const ImVec2& rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}
	inline bool operator!=(const ImVec2& lhs, const ImVec2& rhs)
	{
		return lhs.x != rhs.x || lhs.y != rhs.y;
	}
	#endif


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
	IdContext& GetDebugCtx()
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
		const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_AllowOverlap
		                               | ImGuiTreeNodeFlags_SpanAllColumns
		                               | (isLeaf ? ImGuiTreeNodeFlags_Leaf : 0)
		                               | (defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0);
		bool open                      = ImGui::TreeNodeEx(name.data(), flags);
		ImGui::Indent();
		return open;
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
					StringView name = prop->name.AsString();
					if (!ins.filter || !ins.filter->IsActive()
					    || ins.filter->PassFilter(name.data(), name.data() + name.size()))
					{
						ImGui::BeginDisabled(!editAll && !prop->HasFlag(PF_Edit));
						Inspect(name, prop->access(data), prop->typeId);
						ImGui::EndDisabled();
					}

					++ins.propStack.Last().index;
				}
			}
			ins.propStack.RemoveLast();
		}
	}
	#pragma endregion Inspection


	#pragma region ECS
	i32 DebugECSInspector::uniqueIdCounter = 0;

	using DrawIdAccess = TIdScopeRef<CParent, CChild>;
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

		bool ChooseAddComponentPopup(const char* label, ImGuiTextFilter& filter, IdContext& ctx,
		    TypeId& selectedTypeId, Id entity)
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

				TMap<TypeId, bool> existingTypes;
				for (const auto& pool : ctx.GetPools())
				{
					if (pool.GetPool()->Has(entity))
					{
						existingTypes.Insert(pool.GetId(), true);
					}
				}

				struct TypeEntry
				{
					TypeId id;
					StringView name;
					Color color  = Color::Black();
					bool tag     = false;
					i16 priority = -1;
				};
				TArray<TypeEntry> entries;
				for (const TypeId typeId : GetRegisteredTypeIds())
				{
					if (existingTypes.Contains(typeId))
					{
						continue;
					}
					TypeEntry entry{typeId, GetTypeName(typeId)};
					if (!filter.PassFilter(
					        entry.name.data(), entry.name.data() + entry.name.size()))
					{
						continue;
					}


					if (p::HasTypeFlags(typeId, p::TF_Native))
					{
						entry.color    = Color::FromHex(0x595959);
						entry.priority = 25;
					}
					else if (p::HasTypeFlags(typeId, p::TF_Enum))
					{
						entry.color    = Color::FromHex(0x30b282);
						entry.priority = 20;
					}
					else if (p::HasTypeFlags(typeId, p::TF_Struct))
					{
						entry.color    = Color::FromHex(0x3087b2);
						entry.priority = 15;
						// Empty types have size 1, this means having a 1 byte variable inside will
						// be considered a tag in debug. We accept this small error for simplicity
						entry.tag = GetTypeSize(typeId) <= 1;
					}
					else if (p::HasTypeFlags(typeId, p::TF_Object))
					{
						entry.color    = Color::FromHex(0x3052b2);
						entry.priority = 10;
						entry.tag      = GetTypeSize(typeId) <= 1;
					}
					else if (p::HasTypeFlags(typeId, p::TF_Container))
					{
						entry.color    = Color::FromHex(0xb230ae);
						entry.priority = 5;
					}
					entries.Add(entry);
				}
				entries.Sort([](const TypeEntry& a, const TypeEntry& b)
				{
					if (a.priority != b.priority)
					{
						return a.priority > b.priority;
					}
					return a.name < b.name;
				});

				ImGui::BeginChild("types", {300.f, 400.f});
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
				bool first = true;
				for (const auto& entry : entries)
				{
					if (entry.color != Color::Black())
					{
						ImGui::PushHeaderColor(entry.color);
					}
					static p::String typeLabel;
					typeLabel.clear();
					p::Strings::FormatTo(typeLabel, "{}###{}", entry.name, entry.id.GetId());
					if (ImGui::Selectable(typeLabel, true))
					{
						selectedTypeId = entry.id;
						selectedAny    = true;
						ImGui::CloseCurrentPopup();
					}
					if (entry.color != Color::Black())
					{
						ImGui::PopHeaderColor();
					}

					auto postSelectablePos = ImGui::GetCursorPos();
					if (entry.tag)    // Colored tag right-aligned
					{
						ImGui::PushStyleCompact();
						ImGui::PushButtonColor(Color::FromHex(0xb29e30));
						const float tagWidth       = ImGui::CalcTextSize("Tag").x;
						const float widthAvailable = ImGui::GetContentRegionAvail().x
						                           + details::GetCurrentWindow()->DC.Indent.x;
						ImGui::SameLine(
						    widthAvailable - tagWidth - ImGui::GetStyle().ItemSpacing.x);
						ImGui::SmallButton("Tag");
						ImGui::PopButtonColor();
						ImGui::PopStyleCompact();
					}
					// Avoid weird padding issues with tag button
					ImGui::SetCursorPos(postSelectablePos);
					ImGui::Dummy(ImVec2(0, 1.0f));
				}
				ImGui::PopStyleVar();
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
				typeName.clear();
				p::Strings::FormatTo(typeName, "{}###{}", GetTypeName(typeId), typeId.GetId());
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

		void DrawIdContextMenu(DebugECSContext& ecsDbg)
		{
			if (ImGui::BeginPopup("IdContextMenu"))
			{
				if (ImGui::MenuItem("Delete"))
				{
					RmId(GetDebugCtx(), ecsDbg.contextMenuIds);
					ecsDbg.selectedIds.Clear();
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Inspect"))
				{
					// Inspect a max of 10 entities to not clog the ui
					for (i32 i = 0; i < ecsDbg.contextMenuIds.Size() && i < 10; ++i)
					{
						StartInspectingId(ecsDbg, ecsDbg.contextMenuIds[i]);
					}
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Duplicate"))
				{
					TArray<Id> dups(ecsDbg.contextMenuIds.Size());
					AddId(GetDebugCtx(), dups);    // Assign new ids
					for (i32 i = 0; i < ecsDbg.contextMenuIds.Size(); ++i)
					{
						const Id target = ecsDbg.contextMenuIds[i];
						const Id clone  = dups[i];

						// TODO: Copy components
					}
					ecsDbg.selectedIds = Move(dups);
					ImGui::CloseCurrentPopup();
				}

				ImGui::Separator();

				for (auto& cb : ecsDbg.contextMenuCallbacks)
				{
					cb(ecsDbg.contextMenuIds);
				}

				ImGui::EndPopup();
			}
		}

		bool DrawIdInTable(DrawIdAccess access, DebugECSContext& ecsDbg, Id id, bool hasChildren)
		{
			static p::String idText;
			idText.clear();
			ToString(id, idText);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);    // Inspect
			DrawInspectIdButton(ecsDbg, id);

			ImGui::TableSetColumnIndex(1);    // Id
			ImGuiTreeNodeFlags flags =
			    ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_OpenOnArrow
			    | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NavLeftJumpsToParent
			    | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			const bool isSelected = ecsDbg.selectedIds.Contains(id);
			if (isSelected)
			{
				flags |= ImGuiTreeNodeFlags_Selected;
			}
			if (!hasChildren)
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}
			ImGui::SetNextItemStorageID((ImGuiID)id.value);
			bool open = ImGui::TreeNodeEx(idText.c_str(), flags);
			if (!hasChildren)
			{
				open = false;
			}

			// Selection
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
			{
				if (ImGui::GetIO().KeyCtrl)
				{
					if (isSelected)
					{
						ecsDbg.selectedIds.Remove(id);
					}
					else
					{
						ecsDbg.selectedIds.Add(id);
						ecsDbg.lastSelectedId = id;
					}
				}
				else if (ImGui::GetIO().KeyShift && ecsDbg.lastSelectedId != NoId)
				{
					if (ecsDbg.idToDisplayIndex.Contains(ecsDbg.lastSelectedId)
					    && ecsDbg.idToDisplayIndex.Contains(id))
					{
						i32 lastIdx = ecsDbg.idToDisplayIndex[ecsDbg.lastSelectedId];
						i32 currIdx = ecsDbg.idToDisplayIndex[id];
						if (lastIdx > currIdx)
						{
							Swap(lastIdx, currIdx);
						}

						ecsDbg.selectedIds.Clear(false);
						ecsDbg.lastSelectedId = NoId;
						for (auto& pair : ecsDbg.idToDisplayIndex)
						{
							if (pair.second >= lastIdx && pair.second <= currIdx)
							{
								ecsDbg.selectedIds.Add(pair.first);
								ecsDbg.lastSelectedId = pair.first;
							}
						}
					}
				}
				else
				{
					ecsDbg.selectedIds.Clear(false);
					ecsDbg.selectedIds.Add(id);
					ecsDbg.lastSelectedId = id;
				}
			}
			// Context Menu
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
			{
				if (ecsDbg.selectedIds.Contains(id))
				{
					ecsDbg.contextMenuIds = ecsDbg.selectedIds;
				}
				else
				{
					ecsDbg.selectedIds.Clear(false);
					ecsDbg.contextMenuIds.Assign(1, id);
				}
				ImGui::OpenPopup("IdContextMenu");
			}


			ImGui::TableSetColumnIndex(2);    // Previews
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
			return open;
		}

		Id DrawClippedIdAndAdvanceToNext(
		    ImGuiListClipper& clipper, DrawIdAccess& access, DebugECSContext& ecsDbg, Id id)
		{
			const CParent* parent = access.TryGet<const CParent>(id);
			Id firstChildren      = NoId;
			if (parent)
			{
				for (Id childId : parent->children)
				{
					if (access.IsValid(childId))
					{
						firstChildren = childId;
						break;
					}
				}
			}

			bool open = false;
			if (clipper.UserIndex >= clipper.DisplayStart && clipper.UserIndex < clipper.DisplayEnd)
			{
				open = details::DrawIdInTable(access, ecsDbg, id, firstChildren != NoId);
			}
			else
			{
				open = firstChildren != NoId && ImGui::TreeNodeGetOpen((ImGuiID)id.value);
			}

			if (open)
			{
				ImGui::TreePush(reinterpret_cast<void*>(i64(id.value)));
			}
			ecsDbg.idToDisplayIndex[id] = clipper.UserIndex;
			++clipper.UserIndex;

			if (open)
			{
				return firstChildren;
			}

			// Find next sibling, or walk back to parent's sibling
			Id current          = id;
			bool firstIteration = true;
			while (true)
			{
				const CChild* child = access.TryGet<const CChild>(current);
				if (!child || !access.IsValid(child->parent))
				{
					const CParent* rootParent = access.TryGet<const CParent>(current);
					if (rootParent && ImGui::TreeNodeGetOpen((ImGuiID)current.value))
					{
						ImGui::TreePop();
					}
					return NoId;
				}

				Id parentId      = child->parent;
				const CParent* p = access.TryGet<const CParent>(parentId);
				if (!p)
				{
					return NoId;
				}

				i32 childIdx = -1;
				for (i32 i = 0; i < p->children.Size(); ++i)
				{
					if (p->children[i] == current)
					{
						childIdx = i;
						break;
					}
				}
				for (i32 i = childIdx + 1; i < p->children.Size(); ++i)
				{
					if (access.IsValid(p->children[i]))
					{
						return p->children[i];
					}
				}
				current = parentId;
				ImGui::TreePop();
			}
		}

		void DrawIdTableAsList(DrawIdAccess access, DebugECSContext& ecsDbg, TView<const Id> ids)
		{
			static const ImGuiTableFlags tableFlags =
			    ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable
			    | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;
			if (ImGui::BeginTable("entityTable", 3, tableFlags))
			{
				ImGui::TableSetupColumn("Inspect",
				    ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed
				        | ImGuiTableColumnFlags_NoResize);    // Inspect
				ImGui::TableSetupColumn(
				    "Id", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_IndentEnable);
				ImGui::TableSetupColumn("Previews");

				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableHeadersRow();

				ecsDbg.idToDisplayIndex.Clear();
				ImGuiListClipper clipper;
				clipper.Begin(ids.Size());
				while (clipper.Step())
				{
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
					{
						details::DrawIdInTable(access, ecsDbg, ids[i], false);
					}
				}
				ImGui::EndTable();
			}
		}

		void DrawIdTableAsTree(DrawIdAccess access, DebugECSContext& ecsDbg, TView<const Id> ids)
		{
			static const ImGuiTableFlags tableFlags =
			    ImGuiTableFlags_Reorderable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable
			    | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;
			if (ImGui::BeginTable("entityTable", 3, tableFlags))
			{
				ImGui::TableSetupColumn("Inspect",
				    ImGuiTableColumnFlags_IndentDisable | ImGuiTableColumnFlags_WidthFixed
				        | ImGuiTableColumnFlags_NoResize);    // Inspect
				ImGui::TableSetupColumn(
				    "Id", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_IndentEnable);
				ImGui::TableSetupColumn("Previews");

				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableHeadersRow();

				TArray<Id> rootIds;
				GetIdParentRoots(access, ids, rootIds);
				Id id = rootIds[0];    // TODO

				ecsDbg.idToDisplayIndex.Clear();
				ImGuiListClipper clipper;
				clipper.Begin(GetDebugCtx().Size());
				while (clipper.Step())
				{
					while (clipper.UserIndex < clipper.DisplayEnd && id != NoId)
					{
						id = details::DrawClippedIdAndAdvanceToNext(clipper, access, ecsDbg, id);
					}
				}
				// Count remaining for accurate scrollbar
				while (id != NoId)
				{
					id = details::DrawClippedIdAndAdvanceToNext(clipper, access, ecsDbg, id);
				}
				clipper.SeekCursorForItem(clipper.UserIndex);

				ImGui::EndTable();
			}
		}

		void DrawEntityInspector(
		    StringView label, DebugECSInspector& inspector, bool* open, ImGuiWindowFlags flags)
		{
			if (!EnsureInsideDebug)
			{
				return;
			}

			const bool valid      = GetDebugCtx().IsValid(inspector.id);
			const bool removed    = GetDebugCtx().WasRemoved(inspector.id);
			bool clone            = false;
			Id nextId             = inspector.id;
			bool wantAddComponent = false;

			String name;
			Strings::FormatTo(name, "{}: {}{}###inspector{}", label, inspector.id,
			    removed ? " (removed)" : "", inspector.uniqueId);

			if (inspector.pendingFocus)
			{
				ImGui::SetNextWindowFocus();
				inspector.pendingFocus = false;
			}

			ImGui::SetNextWindowSizeConstraints(ImVec2(300.f, 200.f), ImVec2(800, FLT_MAX));
			if (ImGui::Begin(name.c_str(), open, ImGuiWindowFlags_MenuBar))
			{
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

					if (valid)
					{
						if (ImGui::Button(" + "))
						{
							wantAddComponent = true;
						}
						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltip("Add Component");
						}
					}

					const float cloneWidth  = ImGui::CalcTextSize("-->").x
					                        + ImGui::GetStyle().FramePadding.x * 2.f
					                        + ImGui::GetStyle().ItemSpacing.x;
					const float filterWidth = ImGui::GetContentRegionAvail().x - cloneWidth
					                        - ImGui::GetStyle().ItemSpacing.x;
					ImGui::SetNextItemWidth(Max(filterWidth, 60.f));
					ImGui::DrawFilterWithHint(inspector.filter, "##filter", "Search...", 0.f);

					ImGui::SameLine();
					if (ImGui::Button("-->"))
					{
						clone = true;
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Clone inspector");
					}

					ImGui::EndMenuBar();
				}

				if (wantAddComponent)
				{
					ImGui::OpenPopup("AddComponentPopup");
				}

				static ImGuiTextFilter addComponentFilter;
				TypeId selectedType;
				if (ChooseAddComponentPopup("AddComponentPopup", addComponentFilter, GetDebugCtx(),
				        selectedType, inspector.id))
				{
					GetDebugCtx().AddByTypeId(selectedType, inspector.id);
				}

				if (valid && BeginInspection("##Inspector", {}, inspector.flags))
				{
					currentContext->inspection.filter = &inspector.filter;

					String componentLabel;

					static TypeId pendingDeleteType;
					static bool dontShowDeleteConfirm = false;

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

						ImGuiTreeNodeFlags headerFlags = ImGuiTreeNodeFlags_DefaultOpen
						                               | ImGuiTreeNodeFlags_SpanAllColumns
						                               | ImGuiTreeNodeFlags_AllowOverlap;
						void* data = poolInstance.GetPool()->TryGetVoid(inspector.id);
						if (!data)
						{
							headerFlags |= ImGuiTreeNodeFlags_Leaf;
						}
						ImGui::TableNextRow();
						InspectSetKeyColumn();

						const TypeId typeId = poolInstance.componentId;
						const bool openComponent =
						    ImGui::CollapsingHeader(componentLabel.c_str(), headerFlags);
						if (ImGui::IsItemHovered())
						{
							const float buttonWidth =
							    ImGui::CalcTextSize("x").x + ImGui::GetStyle().FramePadding.x * 2;
							const auto table       = ImGui::GetCurrentTable();
							const float rowRight   = table->WorkRect.Max.x;
							const float widthRight = rowRight - ImGui::GetCursorScreenPos().x;
							ImGui::SameLine(
							    widthRight - buttonWidth - ImGui::GetStyle().ItemSpacing.x);
							ImGui::PushStyleCompact();
							if (ImGui::SmallButton("x"))
							{
								pendingDeleteType = typeId;
								if (!dontShowDeleteConfirm)
								{
									ImGui::OpenPopup("DeleteComponentModal");
								}
								else
								{
									GetDebugCtx().RemoveByTypeId(pendingDeleteType, inspector.id);
								}
							}
							ImGui::PopStyleCompact();
						}

						if (openComponent)
						{
							ImGui::Indent();
							InspectProperties(data, typeId);
							ImGui::Unindent();
						}
					}

					// Delete component confirmation modal
					if (ImGui::BeginPopupModal(
					        "DeleteComponentModal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						static String modalTitle;
						modalTitle.clear();
						p::Strings::FormatTo(modalTitle, "Remove {}?",
						    RemoveNamespace(GetTypeName(pendingDeleteType)));
						ImGui::Text(modalTitle);
						ImGui::NewLine();
						if (ImGui::Button("Delete", ImVec2(120, 0)))
						{
							GetDebugCtx().RemoveByTypeId(pendingDeleteType, inspector.id);
							ImGui::CloseCurrentPopup();
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
						}
						ImGui::Separator();
						ImGui::Checkbox("Don't show this again", &dontShowDeleteConfirm);
						ImGui::EndPopup();
					}

					currentContext->inspection.filter = nullptr;
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
				StartInspectingId(ecsDbg, inspector.id, false);
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
			const bool visible = ImGui::Begin(label, open, flags | ImGuiWindowFlags_MenuBar);
			ImGui::PopStyleVar();
			if (visible)
			{
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::BeginMenu("Settings"))
					{
						ImGui::SeparatorText("View");
						if (ImGui::MenuItem("List", nullptr, ecsDbg.drawAsList))
						{
							ecsDbg.drawAsList = true;
						}
						if (ImGui::MenuItem("Tree", nullptr, !ecsDbg.drawAsList))
						{
							ecsDbg.drawAsList = false;
						}
						ImGui::SeparatorText("Layout");
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
			}
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

			{        // Filtering
				{    // Cache pools
					ecsDbg.includePools.Clear(false);
					ecsDbg.excludePools.Clear(false);
					ecsDbg.previewPools.Clear(false);
					GetDebugCtx().GetPools(ecsDbg.includeTypes, ecsDbg.includePools);
					GetDebugCtx().GetPools(ecsDbg.excludeTypes, ecsDbg.excludePools);
					GetDebugCtx().GetPools(ecsDbg.previewTypes, ecsDbg.previewPools);
				}

				ecsDbg.ids.Clear(false);
				if (ecsDbg.includeTypes.IsEmpty())
				{
					ecsDbg.ids.Assign(GetDebugCtx().Size());
					i32 idx = 0;
					GetDebugCtx().Each([&ecsDbg, &idx](Id id)
					{
						ecsDbg.ids[idx] = id;
						++idx;
					});
				}
				else
				{
					FindAllIdsWith(ecsDbg.includePools, ecsDbg.ids);
				}

				for (const IPool* pool : ecsDbg.excludePools)
				{
					ExcludeIdsWithStable(pool, ecsDbg.ids, false);
				}

				if (ecsDbg.filter.IsActive())
				{
					ExcludeIdsIfStable(ecsDbg.ids, [&ecsDbg](const Id id)
					{
						static p::String idText;
						idText.clear();
						ToString(id, idText);
						return !ecsDbg.filter.PassFilter(
						    idText.c_str(), idText.c_str() + idText.size());
					});
				}

				ecsDbg.ids.Shrink();
				// Ids are always sorted
			}

			DrawIdAccess access{GetDebugCtx()};
			if (ecsDbg.drawAsList)
			{
				details::DrawIdTableAsList(access, ecsDbg, ecsDbg.ids);
			}
			else
			{
				details::DrawIdTableAsTree(access, ecsDbg, ecsDbg.ids);
			}

			details::DrawIdContextMenu(ecsDbg);
		}
		ImGui::End();

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

	bool IsInspectingId(const DebugECSContext& ecsDbg, Id id)
	{
		DebugECSInspector searchInspector;
		searchInspector.id = id;
		return ecsDbg.inspectors.Contains(searchInspector);
	}

	void StartInspectingId(DebugECSContext& ecsDbg, Id id, bool useMainInspector)
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

	void StopInspectingId(DebugECSContext& ecsDbg, Id id)
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

	void DrawInspectIdButton(DebugECSContext& ecsDbg, Id id)
	{
		static p::String inspectLabel;
		inspectLabel.clear();
		const bool inspected = IsInspectingId(ecsDbg, id);
		const char* icon     = inspected ? " × " : "-->";
		p::Strings::FormatTo(inspectLabel, "{}##{}", icon, id);
		ImGui::PushTextColor(
		    inspected ? ImGui::GetTextColor() : ImGui::GetTextColor().Translucency(0.3f));
		ImGui::PushStyleCompact();
		if (ImGui::Button(inspectLabel.c_str()))
		{
			if (inspected)
			{
				StopInspectingId(ecsDbg, id);
			}
			else
			{
				StartInspectingId(ecsDbg, id);
			}
		}
		ImGui::PopStyleCompact();
		ImGui::PopTextColor();
	}

	void AddIdContextMenuCallback(DebugECSContext::ContextMenuCallback callback)
	{
		if (EnsureInsideDebug)
		{
			currentContext->ecs.contextMenuCallbacks.Add(Move(callback));
		}
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
			if ((ctx.bFilterWithAny && !HasAnyTypeFlags(type, ctx.typeFlagsFilter))
			    || (!ctx.bFilterWithAny && !HasTypeFlags(type, ctx.typeFlagsFilter)))
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

		if (ImGui::Begin(label, open, flags))
		{
			if (ImGui::BeginPopup("Filter"))
			{
				if (ImGui::RadioButton("Any", reflectDbg.bFilterWithAny))
				{
					reflectDbg.bFilterWithAny = true;
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("All", !reflectDbg.bFilterWithAny))
				{
					reflectDbg.bFilterWithAny = false;
				}

				ImGui::CheckboxFlags(
				    "Native", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Native));
				ImGui::CheckboxFlags("Enum", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Enum));
				ImGui::CheckboxFlags(
				    "Struct", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Struct));
				ImGui::CheckboxFlags(
				    "Object", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Object));
				ImGui::CheckboxFlags(
				    "Container", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Container));

				ImGui::SeparatorText("ECS");
				ImGui::CheckboxFlags("Store Last Modified", (ImU64*)&reflectDbg.typeFlagsFilter,
				    ImU64(TF_ECS_StoreLastModified));
				ImGui::CheckboxFlags("Modify on Edit", (ImU64*)&reflectDbg.typeFlagsFilter,
				    ImU64(TF_ECS_ModifyOnEdit));
				ImGui::CheckboxFlags("Modify on Add", (ImU64*)&reflectDbg.typeFlagsFilter,
				    ImU64(TF_ECS_ModifyOnAdd));
				ImGui::CheckboxFlags("Modify on Get", (ImU64*)&reflectDbg.typeFlagsFilter,
				    ImU64(TF_ECS_ModifyOnGet));
				ImGui::CheckboxFlags("Modify on Remove", (ImU64*)&reflectDbg.typeFlagsFilter,
				    ImU64(TF_ECS_ModifyOnRm));

				ImGui::SeparatorText("Other");
				ImGui::CheckboxFlags(
				    "Not Serialized", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_NotSerialized));
				ImGui::CheckboxFlags(
				    "Abstract", (ImU64*)&reflectDbg.typeFlagsFilter, ImU64(TF_Abstract));
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
		}
		ImGui::End();
	}
	#pragma endregion Reflection

	#pragma region Memory
	namespace details
	{
		// Arena type colors
		static ImU32 GetArenaColor(TypeId typeId)
		{
			// Simple hash-based coloring for arena types
			u32 hash = static_cast<u32>(typeId);
			float h  = static_cast<float>((hash * 2654435761u) % 360) / 360.0f;
			float s  = 0.7f;
			float v  = 0.9f;
			// HSV to RGB
			int i   = static_cast<int>(h * 6.0f);
			float f = h * 6.0f - i;
			float p = v * (1.0f - s);
			float q = v * (1.0f - f * s);
			float t = v * (1.0f - (1.0f - f) * s);
			float r, g, b;
			switch (i % 6)
			{
				case 0:
					r = v;
					g = t;
					b = p;
					break;
				case 1:
					r = q;
					g = v;
					b = p;
					break;
				case 2:
					r = p;
					g = v;
					b = t;
					break;
				case 3:
					r = p;
					g = q;
					b = v;
					break;
				case 4:
					r = t;
					g = p;
					b = v;
					break;
				case 5:
					r = v;
					g = p;
					b = q;
					break;
				default:
					r = v;
					g = v;
					b = v;
					break;
			}
			return IM_COL32(static_cast<int>(r * 255), static_cast<int>(g * 255),
			    static_cast<int>(b * 255), 255);
		}

		// Draw zigzag line for gaps
		static void DrawZigzag(ImDrawList* draw, ImVec2 p0, ImVec2 p1, float h, ImU32 color)
		{
			static constexpr float step = 4.0f;
			draw->PathLineTo(p0);
			for (float x = p0.x; x < p1.x; x += step)
			{
				draw->PathLineTo(ImVec2(x, p0.y));
				draw->PathLineTo(ImVec2(x + step * 0.5f, p0.y + h));
			}
			draw->PathStroke(color, 0, 1.0f);
		}
	}    // namespace details

	void DrawMemory(const char* label, bool* open, ImGuiWindowFlags flags)
	{
		if (!EnsureInsideDebug)
		{
			return;
		}

		auto& memoryDbg = currentContext->memory;

		// Rebuild arena info cache
		memoryDbg.arenaInfos.Clear();
		const auto& arenas = GetAllArenas();
		for (const auto& entry : arenas)
		{
			if (!entry.arena)
			{
				continue;
			}

			DebugMemoryContext::ArenaBlockInfo info;
			info.arena  = entry.arena;
			info.typeId = entry.typeId;

			// Get blocks
			TArray<ArenaBlock> blocks;
			entry.arena->GetBlocks(blocks);
			for (const auto& block : blocks)
			{
				info.blocks.Add(block.data);
				info.blockSizes.Add(block.size);
				if (info.blockBegin == nullptr
				    || (uintptr_t)block.data < (uintptr_t)info.blockBegin)
				{
					info.blockBegin = block.data;
				}
				info.blockSize += block.size;
			}

			// Get stats if available
			const auto* stats = entry.arena->GetStats();
			if (stats)
			{
				info.usedSize = stats->used;
				info.freeSize = info.blockSize > info.usedSize ? info.blockSize - info.usedSize : 0;
			}

			memoryDbg.arenaInfos.Add(info);
		}

		// Sort by block address for consistent rendering
		std::sort(memoryDbg.arenaInfos.begin(), memoryDbg.arenaInfos.end(),
		    [](const auto& a, const auto& b)
		{
			return (uintptr_t)a.blockBegin < (uintptr_t)b.blockBegin;
		});

		// Determine selected arena
		DebugMemoryContext::ArenaBlockInfo* selectedInfo = nullptr;
		if (memoryDbg.selectedArena >= 0 && memoryDbg.selectedArena < memoryDbg.arenaInfos.Size())
		{
			selectedInfo = &memoryDbg.arenaInfos[memoryDbg.selectedArena];
		}

		if (!ImGui::Begin(label, open, flags))
		{
			ImGui::End();
			return;
		}

		// Toolbar
		{
			ImGui::Checkbox("Horizontal", &memoryDbg.horizontal);
			ImGui::SameLine();
			ImGui::Checkbox("Hex", &memoryDbg.showHex);
			ImGui::SameLine();
			ImGui::Checkbox("ASCII", &memoryDbg.showAscii);
			ImGui::SameLine();
			ImGui::Text("| Arenas: %d", static_cast<int>(memoryDbg.arenaInfos.Size()));
		}

		ImGui::Separator();

		// Calculate layout
		const float detailsWidth = memoryDbg.selectedArena >= 0 ? 300.0f : 0.0f;
		const float graphWidth   = ImGui::GetContentRegionAvail().x - detailsWidth;
		const float graphHeight  = ImGui::GetContentRegionAvail().y;

		const float hexLineHeight =
		    memoryDbg.horizontal && (memoryDbg.showHex || memoryDbg.showAscii) ? 20.0f : 0.0f;
		const float hexLineWidth =
		    !memoryDbg.horizontal && (memoryDbg.showHex || memoryDbg.showAscii) ? 40.0f : 0.0f;

		// Draw hex/ASCII lines and graph
		if (memoryDbg.horizontal)
		{
			// Horizontal layout: hex/ASCII at top, graph below

			// Hex/ASCII lines (fixed at top)
			if (hexLineHeight > 0)
			{
				ImGui::BeginChild("##hexascii", ImVec2(graphWidth, hexLineHeight), false);
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				ImVec2 canvasPos     = ImGui::GetCursorScreenPos();
				ImVec2 canvasSize    = ImGui::GetContentRegionAvail();

				// Draw hex/ASCII for selected arena or first arena
				if (selectedInfo || !memoryDbg.arenaInfos.IsEmpty())
				{
					const auto& info = selectedInfo ? *selectedInfo : memoryDbg.arenaInfos[0];
					if (info.blockBegin && info.blockSize > 0)
					{
						const u8* data      = static_cast<const u8*>(info.blockBegin);
						float pixelsPerByte = canvasSize.x / static_cast<float>(info.blockSize);
						sizet maxBytes =
						    static_cast<sizet>(canvasSize.x / 20.0f);
						sizet visibleBytes = info.blockSize < maxBytes ? info.blockSize : maxBytes;

						for (sizet i = 0; i < visibleBytes; ++i)
						{
							float x = canvasPos.x + static_cast<float>(i) * pixelsPerByte;

							if (memoryDbg.showHex)
							{
								char hex[3];
								snprintf(hex, sizeof(hex), "%02X", data[i]);
								drawList->AddText(
								    ImVec2(x, canvasPos.y), IM_COL32(200, 200, 200, 255), hex);
							}

							if (memoryDbg.showAscii)
							{
								char c = (data[i] >= 32 && data[i] < 127)
								           ? static_cast<char>(data[i])
								           : '.';
								drawList->AddText(
								    ImVec2(x, canvasPos.y + (memoryDbg.showHex ? 10.0f : 0.0f)),
								    IM_COL32(150, 255, 150, 255), &c, &c + 1);
							}
						}
					}
				}

				ImGui::EndChild();
				ImGui::Separator();
			}

			// Graph area
			ImGui::BeginChild("##graph", ImVec2(graphWidth, graphHeight - hexLineHeight), false);
		}
		else
		{
			// Vertical layout: graph at right
			ImGui::BeginChild("##graph", ImVec2(graphWidth - hexLineWidth, graphHeight), false);
		}

		// Draw icicle graph — address-relative blocks with ImGui drawlist
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 canvasPos     = ImGui::GetCursorScreenPos();
			ImVec2 canvasSize    = ImGui::GetContentRegionAvail();

			// Find address range across all blocks
			uintptr_t addrMin = 0, addrMax = 0;
			bool hasAddr = false;
			for (const auto& info : memoryDbg.arenaInfos)
			{
				for (int b = 0; b < info.blocks.Size(); ++b)
				{
					uintptr_t addr = reinterpret_cast<uintptr_t>(info.blocks[b]);
					uintptr_t end  = addr + info.blockSizes[b];
					if (!hasAddr)
					{
						addrMin = addr;
						addrMax = end;
						hasAddr = true;
					}
					else
					{
						if (addr < addrMin)
						{
							addrMin = addr;
						}
						if (end > addrMax)
						{
							addrMax = end;
						}
					}
				}
			}

			if (!hasAddr)
			{
				addrMin = 0;
				addrMax = 1;
			}

			double range = static_cast<double>(addrMax - addrMin);
			if (range <= 0)
			{
				range = 1.0;
			}

			// Compute visible address range from zoom/pan
			double viewRange = range / memoryDbg.zoom;
			double viewStart = addrMin + memoryDbg.pan * (range - viewRange);
			double viewEnd   = viewStart + viewRange;
			if (viewStart < addrMin)
			{
				viewStart = addrMin;
				viewEnd   = viewStart + viewRange;
			}
			if (viewEnd > addrMax)
			{
				viewEnd   = addrMax;
				viewStart = viewEnd - viewRange;
			}
			// Clamp pan to match (avoid div-by-zero when zoom == 1.0)
			double panDiv = range - viewRange;
			memoryDbg.pan = (panDiv > 0.0) ? static_cast<float>((viewStart - addrMin) / panDiv) : 0.0f;

			// Address to pixel x
			auto AddrToX = [&](uintptr_t a) {
				double t = (static_cast<double>(a) - viewStart) / viewRange;
				return canvasPos.x + static_cast<float>(t * canvasSize.x);
			};

			constexpr float rulerH = 24.0f;
			constexpr float rowH   = 20.0f;
			float graphY0          = canvasPos.y + rulerH + 2.0f;

			// Ruler bar
			{
				// Dark background
				drawList->AddRectFilled(ImVec2(canvasPos.x, canvasPos.y),
				    ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + rulerH),
				    IM_COL32(20, 20, 30, 255));

				// Compute nice tick spacing
				double pixelsPerByte = canvasSize.x / viewRange;
				double targetPx      = 100.0;
				double rawTick       = targetPx / pixelsPerByte;

				// Nice-round to 1/2/5/10 * 10^N
				auto NiceRound = [](double v) -> double {
					if (v <= 0)
					{
						return 1.0;
					}
					double scale = 1.0;
					while (v > 10.0)
					{
						v /= 10.0;
						scale *= 10.0;
					}
					while (v < 1.0)
					{
						v *= 10.0;
						scale /= 10.0;
					}
					if (v < 1.5)      { return 1.0 * scale; }
					else if (v < 3.5) { return 2.0 * scale; }
					else if (v < 7.5) { return 5.0 * scale; }
					else              { return 10.0 * scale; }
				};

				double majorStep = NiceRound(rawTick);
				if (majorStep < 1.0)
				{
					majorStep = 1.0;
				}
				double qtrStep = majorStep / 4.0;

				// Start at first multiple of majorStep within view
				double tickAddr =
				    std::ceil(viewStart / majorStep) * majorStep;

				for (; tickAddr <= viewEnd; tickAddr += majorStep)
				{
					// Quarter ticks
					for (int q = 0; q < 4; ++q)
					{
						double ta = tickAddr + static_cast<double>(q) * qtrStep;
						if (ta < viewStart || ta > viewEnd)
						{
							continue;
						}
						float tx = AddrToX(static_cast<uintptr_t>(ta));

						if (q == 0)
						{
							// Major — full height, bright
							drawList->AddLine(ImVec2(tx, canvasPos.y + rulerH - 12),
							    ImVec2(tx, canvasPos.y + rulerH),
							    IM_COL32(220, 220, 220, 255), 1.5f);

							char buf[32];
							snprintf(buf, sizeof(buf), "0x%llX",
							    static_cast<unsigned long long>(
							        static_cast<uintptr_t>(ta)));
							drawList->AddText(ImVec2(tx + 3.0f, canvasPos.y + rulerH - 12),
							    IM_COL32(200, 200, 200, 255), buf);
						}
						else if (q == 2)
						{
							// Half — medium height, medium opacity
							drawList->AddLine(ImVec2(tx, canvasPos.y + rulerH - 8),
							    ImVec2(tx, canvasPos.y + rulerH),
							    IM_COL32(180, 180, 200, 160));
						}
						else
						{
							// Quarter — short, dim
							drawList->AddLine(ImVec2(tx, canvasPos.y + rulerH - 5),
							    ImVec2(tx, canvasPos.y + rulerH),
							    IM_COL32(150, 150, 180, 80));
						}
					}
				}
			}

			// Arena rows
			float yPos = 0.0f;
			for (int i = 0; i < memoryDbg.arenaInfos.Size(); ++i)
			{
				const auto& info = memoryDbg.arenaInfos[i];

				ImU32 fill = details::GetArenaColor(info.typeId);
				if (i == memoryDbg.selectedArena)
				{
					fill = IM_COL32(255, 255, 100, 255);
				}

				float rowY = graphY0 + yPos;

				// Row background
				drawList->AddRectFilled(ImVec2(canvasPos.x, rowY),
				    ImVec2(canvasPos.x + canvasSize.x, rowY + rowH),
				    IM_COL32(0, 0, 0, 30));

				// Blocks at address positions
				if (info.blockBegin && info.blockSize > 0)
				{
					for (int b = 0; b < info.blocks.Size(); ++b)
					{
						float x0 = AddrToX(reinterpret_cast<uintptr_t>(info.blocks[b]));
						float x1 = AddrToX(reinterpret_cast<uintptr_t>(info.blocks[b])
						                   + info.blockSizes[b]);

						// Clamp to visible area
						if (x1 < canvasPos.x || x0 > canvasPos.x + canvasSize.x)
						{
							continue;
						}
						x0 = (x0 > canvasPos.x) ? x0 : canvasPos.x;
						x1 = (x1 < canvasPos.x + canvasSize.x) ? x1 : (canvasPos.x + canvasSize.x);

						drawList->AddRectFilled(
						    ImVec2(x0, rowY), ImVec2(x1, rowY + rowH), fill);
						drawList->AddRect(
						    ImVec2(x0, rowY), ImVec2(x1, rowY + rowH),
						    IM_COL32(0, 0, 0, 80));
					}
				}

				// Arena label
				char label[128];
				snprintf(label, sizeof(label), "%s###%d", GetTypeName(info.typeId).data(), i);
				drawList->AddText(ImVec2(canvasPos.x + 4.0f, rowY + 2.0f),
				    IM_COL32(220, 220, 220, 255), label);

				// Selection click
				ImRect rowRect(ImVec2(canvasPos.x, rowY),
				    ImVec2(canvasPos.x + canvasSize.x, rowY + rowH));
				if (rowRect.Contains(ImGui::GetIO().MousePos) && ImGui::IsMouseClicked(0))
				{
					memoryDbg.selectedArena = i;
				}

				// Tooltip on hover
				if (rowRect.Contains(ImGui::GetIO().MousePos))
				{
					ImGui::BeginTooltip();
					ImGui::Text("Arena: %s", GetTypeName(info.typeId).data());
					ImGui::Text("Block: 0x%p", info.blockBegin);
					ImGui::Text("Size: %zu bytes", info.blockSize);
					if (info.usedSize > 0 || info.freeSize > 0)
					{
						ImGui::Text("Used: %zu (%.1f%%)", info.usedSize,
						    100.0f * info.usedSize / info.blockSize);
					}
					ImGui::EndTooltip();
				}

				yPos += rowH + 2.0f;
			}

			// Zoom / pan
			ImRect graphRect(ImVec2(canvasPos.x, graphY0),
			    ImVec2(canvasPos.x + canvasSize.x,
			        graphY0 + static_cast<float>(memoryDbg.arenaInfos.Size()) * (rowH + 2.0f)));

			// Mouse wheel zoom (over graph area)
			if (graphRect.Contains(ImGui::GetIO().MousePos))
			{
				float wheel = ImGui::GetIO().MouseWheel;
				if (wheel != 0.0f)
				{
					// Zoom towards cursor
					float mx = ImGui::GetIO().MousePos.x;
					double addrAtCursor =
					    viewStart + ((mx - canvasPos.x) / canvasSize.x) * viewRange;

					memoryDbg.zoom *= (wheel > 0) ? 1.3f : (1.0f / 1.3f);
					memoryDbg.zoom = (memoryDbg.zoom < 1.0f) ? 1.0f : (memoryDbg.zoom > 1000000.0f ? 1000000.0f : memoryDbg.zoom);

					double newViewRange = range / memoryDbg.zoom;
					double newViewStart = addrAtCursor
					                      - ((mx - canvasPos.x) / canvasSize.x) * newViewRange;
					double pd = range - newViewRange;
					memoryDbg.pan = (pd > 0.0) ? static_cast<float>((newViewStart - addrMin) / pd) : 0.0f;
				}
			}

			// Drag to pan
			if (graphRect.Contains(ImGui::GetIO().MousePos) && ImGui::IsMouseDragging(2))
			{
				if (!memoryDbg.panning)
				{
					memoryDbg.panning   = true;
					memoryDbg.panPos    = ImGui::GetIO().MousePos;
					memoryDbg.panStart  = memoryDbg.pan;
				}
				ImVec2 delta;
				delta.x = ImGui::GetIO().MousePos.x - memoryDbg.panPos.x;
				double addrDelta = -delta.x / canvasSize.x * viewRange;
				double pd = range - viewRange;
				double newPan = (pd > 0.0) ? memoryDbg.panStart + addrDelta / pd : memoryDbg.panStart;
				memoryDbg.pan = (newPan < 0.0f) ? 0.0f : (newPan > 1.0f ? 1.0f : static_cast<float>(newPan));
			}
			else
			{
				memoryDbg.panning = false;
			}
		}

		ImGui::EndChild();

		// Vertical: draw hex/ASCII on left side
		if (!memoryDbg.horizontal && hexLineWidth > 0)
		{
			ImGui::SameLine();
			ImGui::BeginChild("##hexascii", ImVec2(hexLineWidth, graphHeight), false);
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 canvasPos     = ImGui::GetCursorScreenPos();

			if (selectedInfo || !memoryDbg.arenaInfos.IsEmpty())
			{
				const auto& info = selectedInfo ? *selectedInfo : memoryDbg.arenaInfos[0];
				if (info.blockBegin && info.blockSize > 0)
				{
					const u8* data      = static_cast<const u8*>(info.blockBegin);
					float pixelsPerByte = graphHeight / static_cast<float>(info.blockSize);
					sizet maxBytes      = static_cast<sizet>(graphHeight / 20.0f);
					sizet visibleBytes  = info.blockSize < maxBytes ? info.blockSize : maxBytes;

					for (sizet i = 0; i < visibleBytes; ++i)
					{
						float y = canvasPos.y + static_cast<float>(i) * pixelsPerByte;

						if (memoryDbg.showHex)
						{
							char hex[3];
							snprintf(hex, sizeof(hex), "%02X", data[i]);
							drawList->AddText(
							    ImVec2(canvasPos.x, y), IM_COL32(200, 200, 200, 255), hex);
						}

						if (memoryDbg.showAscii)
						{
							char c = (data[i] >= 32 && data[i] < 127)
							           ? static_cast<char>(data[i])
							           : '.';
							drawList->AddText(
							    ImVec2(canvasPos.x + (memoryDbg.showHex ? 20.0f : 0.0f), y),
							    IM_COL32(150, 255, 150, 255), &c, &c + 1);
						}
					}
				}
			}

			ImGui::EndChild();
		}

		// Details panel (dockable window)
		if (selectedInfo)
		{
			ImGui::SameLine();
			ImGui::BeginChild("##details", ImVec2(detailsWidth, graphHeight), true);

			ImGui::Text("Arena Details");
			ImGui::Separator();

			ImGui::Text("Type: %s", GetTypeName(selectedInfo->typeId).data());
			ImGui::Text("Block: 0x%p - 0x%p", selectedInfo->blockBegin,
			    static_cast<u8*>(selectedInfo->blockBegin) + selectedInfo->blockSize);
			ImGui::Text("Size: %zu bytes", selectedInfo->blockSize);

			if (selectedInfo->usedSize > 0 || selectedInfo->freeSize > 0)
			{
				float usedPct = 100.0f * selectedInfo->usedSize / selectedInfo->blockSize;
				ImGui::Text("Used: %zu (%.1f%%)", selectedInfo->usedSize, usedPct);
				ImGui::Text("Free: %zu (%.1f%%)", selectedInfo->freeSize, 100.0f - usedPct);

				// Progress bar
				ImGui::ProgressBar(usedPct / 100.0f);
			}

			ImGui::Separator();
			ImGui::Text("Blocks: %d", static_cast<int>(selectedInfo->blocks.Size()));

			// List blocks
			for (int b = 0; b < selectedInfo->blocks.Size(); ++b)
			{
				char blockLabel[128];
				snprintf(blockLabel, sizeof(blockLabel), "Block %d: 0x%p (%zu bytes)", b,
				    selectedInfo->blocks[b], selectedInfo->blockSizes[b]);
				ImGui::BulletText("%s", blockLabel);
			}

			// Close button
			ImGui::Separator();
			if (ImGui::Button("Deselect"))
			{
				memoryDbg.selectedArena = -1;
			}

			ImGui::EndChild();
		}

		ImGui::End();
	}
	#pragma endregion Memory

	bool BeginDebug(DebugContext& context)
	{
		if (!P_EnsureMsg(!currentContext,
		        "Called BeginDebug() but there was a ECS Debug Context already! "
		        "Forgot to call "
		        "EndECSDebug()?"))
		{
			return false;
		}

		if (!P_EnsureMsg(context.ctx, "Debug Context does not contain a valid IdContext."))
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
