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
#include "Pipe/Core/String.h"
#include "Pipe/Memory/MemoryStats.h"
#include "PipeColor.h"
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
		bool showHex      = true;
		bool showAscii    = true;

		// Absolute view state (stable across reallocations of the timeline).
		// viewStart: target left-edge address (set by user input, clamped to data).
		// viewRange: target visible byte range (0 = uninitialized).
		// smoothViewStart / smoothViewRange: actual rendered values (<0 = uninit).
		// On pan the target moves and the smooth position lerps toward it.
		// On wheel zoom the range lerps smoothly (start snaps to keep the
		// anchor under the cursor fixed). On Focus/double-click both snap.
		uintptr_t viewStart    = 0;
		double viewRange       = 0.0;
		double smoothViewStart = -1.0;
		double smoothViewRange = -1.0;

		// "Go to" address input
		char gotoBuffer[32]   = "";
		uintptr_t pendingGoto = 0;

		// Cached graph width (updated each frame, used to position the goto offset)
		float lastGraphW = 1500.0f;

		// Address ranges hidden by the user (compressed to a zigzag). Stored
		// sorted by start; the compressed AddrToX accounts for them.
		struct HiddenRange
		{
			uintptr_t start = 0;
			uintptr_t end   = 0;
		};
		TArray<HiddenRange> hiddenRanges;

		// Horizontal area selection (left-mouse drag in the graph)
		uintptr_t selectionStart = 0;
		uintptr_t selectionEnd   = 0;
		bool selectionValid      = false;
		bool isSelecting         = false;
		uintptr_t selectAnchor   = 0;

		// When true, the next view-update bypasses the minimum view-range
		// (24px/byte) cap so a "Focus selection" can fully zoom in.
		bool forceZoom = false;

		// Zoom anchor: while isZooming is true, smoothViewStart is coupled to
		// smoothViewRange so the address under the cursor stays fixed during
		// the range lerp (prevents the left/right flicker).
		bool isZooming           = false;
		uintptr_t zoomAnchorAddr = 0;
		float zoomAnchorRelX     = 0.0f;

		// Cached arena blocks for rendering
		struct ArenaBlockInfo
		{
			const Arena* arena = nullptr;
			TypeId typeId;
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
		// Arena colors — hash a unique identifier (arena pointer) to produce a
		// distinct color per arena instance. Note: Arena::typeId is TypeId<Arena>
		// for all registered arenas (cached in base ctor), so it cannot be used.
		static p::Color GetArenaColor(TypeId id)
		{
			const u32 hash = static_cast<u32>(id.GetId());
			const float h  = static_cast<float>((hash * 2654435761u) % 360);
			const float s  = 0.65f;
			const float v  = 0.55f;
			// Build the color in HSV space and let p::Color convert to RGBA.
			return p::Color{
			    p::HSVColor{h, s, v, 1.0f}
            };
		}

		// Draw a zigzag line from p0 to p1 (horizontal), alternating y between
		// p0.y and p0.y + h. Used as a scale-break indicator on huge blocks.
		static void DrawZigzag(ImDrawList* draw, ImVec2 p0, ImVec2 p1, float h, ImU32 color)
		{
			constexpr float step = 6.0f;
			draw->PathLineTo(p0);
			bool down = true;
			for (float x = p0.x + step; x < p1.x; x += step)
			{
				draw->PathLineTo(ImVec2(x, down ? (p0.y + h) : p0.y));
				down = !down;
			}
			draw->PathLineTo(p1);
			draw->PathStroke(color, 0, 1.5f);
		}

		// Draw a vertical zigzag from (x, y0) to (x, y1), alternating x
		// by ±w. Used as a scale-break indicator on hidden (compressed) ranges.
		static void DrawZigzagVertical(
		    ImDrawList* draw, float x, float y0, float y1, float w, ImU32 color)
		{
			constexpr float step = 3.0f;
			draw->PathLineTo(ImVec2(x, y0));
			bool right = true;
			for (float y = y0 + step; y < y1; y += step)
			{
				draw->PathLineTo(ImVec2(right ? (x + w) : (x - w), y));
				right = !right;
			}
			draw->PathLineTo(ImVec2(x, y1));
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
		TArray<const Arena*> arenas;
		GetAllArenas(arenas);
		for (const auto* arena : arenas)
		{
			if (!arena)
			{
				continue;
			}

			DebugMemoryContext::ArenaBlockInfo info;
			info.arena  = arena;
			info.typeId = arena->GetTypeId();

			// Get blocks
			TArray<ArenaBlock> blocks;
			arena->GetBlocks(blocks);
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
			const auto* stats = arena->GetStats();
			if (stats)
			{
				// Read summary counters under a brief shared lock.
				// We do NOT copy the allocations/freedAllocations TArrays here —
				// their copy-assignment would allocate from the current arena, which
				// (when the arena being debugged IS the current arena) recurses into
				// stats.Add() and deadlocks on the same mutex.
				std::shared_lock<std::shared_mutex> statsLock(stats->mutex);
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

		if (!ImGui::Begin(label, open, flags | ImGuiWindowFlags_MenuBar))
		{
			ImGui::End();
			return;
		}
		// Menu bar
		if (ImGui::BeginMenuBar())
		{
			ImGui::MenuItem("Hex", nullptr, &memoryDbg.showHex);
			ImGui::MenuItem("ASCII", nullptr, &memoryDbg.showAscii);
			// Selection range display (replaces the old "Go to" input).
			// Two read-only text fields (no visible label — ## prefix hides it)
			// showing the start and end of the current selection, followed by a
			// "Focus selection" button. Single click on the graph = single byte
			// (start == end); drag = range.
			{
				char startBuf[32];
				char endBuf[32];
				startBuf[0] = '\0';
				endBuf[0]   = '\0';
				if (memoryDbg.selectionValid)
				{
					snprintf(startBuf, sizeof(startBuf), "0x%llX",
					    static_cast<unsigned long long>(memoryDbg.selectionStart));
					snprintf(endBuf, sizeof(endBuf), "0x%llX",
					    static_cast<unsigned long long>(memoryDbg.selectionEnd));
				}
				ImGui::SetNextItemWidth(110.0f);
				ImGui::InputText("##selStart", startBuf, sizeof(startBuf),
				    ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(110.0f);
				ImGui::InputText("##selEnd", endBuf, sizeof(endBuf),
				    ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
				ImGui::SameLine();
				if (ImGui::Button("Focus selection"))
				{
					if (memoryDbg.selectionValid
					    && memoryDbg.selectionEnd >= memoryDbg.selectionStart)
					{
						const uintptr_t s = memoryDbg.selectionStart;
						const uintptr_t e = memoryDbg.selectionEnd;
						const double size = static_cast<double>(e - s);
						if (size <= 0.0)
						{
							memoryDbg.viewRange = 64.0;
							memoryDbg.viewStart = (s > 32) ? (s - 32) : 0;
						}
						else
						{
							memoryDbg.viewRange = size;
							memoryDbg.viewStart = s;
						}
						memoryDbg.smoothViewStart = static_cast<double>(memoryDbg.viewStart);
						memoryDbg.smoothViewRange = memoryDbg.viewRange;
						memoryDbg.forceZoom       = true;
					}
				}
			}
			ImGui::SameLine();

			ImGui::TextDisabled("Arenas: %d", static_cast<int>(memoryDbg.arenaInfos.Size()));
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
				    "Middle mouse drag to pan\n"
				    "Alt + Mouse wheel to pan\n"
				    "Mouse wheel to zoom\n"
				    "Click a row to select\n"
				    "Double-click a block to focus it");
			}
			ImGui::EndMenuBar();
		}

		// Calculate layout
		const float detailsWidth = memoryDbg.selectedArena >= 0 ? 300.0f : 0.0f;
		const float graphWidth   = ImGui::GetContentRegionAvail().x - detailsWidth;
		const float graphHeight  = ImGui::GetContentRegionAvail().y;

		// Horizontal: two distinct bands (HEX then ASCII) stacked below the ruler.
		// Horizontal layout: two distinct bands (HEX then ASCII) stacked below
		// the ruler. A 2px gap is added between adjacent bands so they are
		// clearly delimited.
		const float hexBandH   = memoryDbg.showHex ? 20.0f : 0.0f;
		const float asciiBandH = memoryDbg.showAscii ? 20.0f : 0.0f;

		const float bandGap       = (hexBandH > 0 && asciiBandH > 0) ? 2.0f : 0.0f;
		const float hexLineHeight = hexBandH + bandGap + asciiBandH;

		// Open the graph child — full interactable area (ruler + hex/ASCII + rows).
		// Hex/ASCII and timeline are drawn inside this child so zoom/pan/select
		// work everywhere within it.
		ImGui::BeginChild("##graph", ImVec2(graphWidth, graphHeight), false);
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 canvasPos     = ImGui::GetCursorScreenPos();

			// Capture input over the whole graph so the parent window never
			// starts a window-drag from a click here. canvasPos is captured
			// BEFORE the button (the button advances the cursor).
			ImGui::InvisibleButton("##graph_input", ImVec2(graphWidth, graphHeight));
			ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
			ImVec2 canvasSize = ImGui::GetContentRegionAvail();

			// Graph area spans the full child width (hex/ASCII bands are
			// stacked below the ruler, not beside it).
			const float graphX0 = canvasPos.x;
			const float graphW  = canvasSize.x;

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

			// Initialize the absolute view on the first frame, or fit it to the
			// current data range when it is uninitialized. After that, the view
			// is anchored to absolute addresses and does NOT move when the
			// global address range changes (e.g. new allocations extending it).
			if (memoryDbg.viewRange <= 0.0)
			{
				memoryDbg.viewRange = range;
				memoryDbg.viewStart = addrMin;
			}

			double viewStart = static_cast<double>(memoryDbg.viewStart);
			double viewRange = memoryDbg.viewRange;
			double viewEnd   = viewStart + viewRange;

			// Block click detection: set during the block-draw loop when
			// IsMouseClicked(0) lands on a block rect. Consumed by the
			// selection handler to snap the selection to the block range.
			uintptr_t clickedBlockStart = 0;
			uintptr_t clickedBlockEnd   = 0;
			bool clickedBlock           = false;

			// Major timeline tick x-positions, collected during the ruler
			// pass and drawn as darker grid lines through the graph area
			// AFTER the row loop (so block fills don't cover them).
			TArray<float> majorTickXs;

			// Pan is fully unclamped — the view can be anywhere.
			// Zoom-out is still capped to the content size so the user cannot
			// zoom out into terabytes of empty space.
			if (viewRange > range)
			{
				viewRange = range;
			}
			// Enforce a minimum view range (24px/byte readability cap), unless
			// the user explicitly focused a selection (allow full zoom in).
			if (!memoryDbg.forceZoom)
			{
				viewRange = p::Max(viewRange, p::Max(1.0, graphW / 24.0));
			}
			memoryDbg.forceZoom = false;

			memoryDbg.viewStart = (uintptr_t)viewStart;
			memoryDbg.viewRange = viewRange;

			// Cache graph width for the "Go to" 128px offset
			memoryDbg.lastGraphW = graphW;

			// Apply a pending "Go to" navigation: place the address ~128px
			// from the left edge of the graph so it isn't flush against it.
			// Snap the smooth view (no lerp) for instant navigation.
			if (memoryDbg.pendingGoto != 0)
			{
				const float w  = (memoryDbg.lastGraphW > 0.0f) ? memoryDbg.lastGraphW : 1500.0f;
				const double t = 128.0 / static_cast<double>(w);
				const double newViewStart =
				    static_cast<double>(memoryDbg.pendingGoto) - t * viewRange;
				memoryDbg.viewStart       = (uintptr_t)newViewStart;
				memoryDbg.smoothViewStart = newViewStart;
				memoryDbg.pendingGoto     = 0;
			}

			// Smooth pan + zoom: lerp the rendered viewStart and viewRange toward
			// the targets. Frame-rate independent exponential lerp (rate=25 ≈ fast).
			// During a zoom the start is coupled to the range via the anchor so
			// the address under the cursor doesn't drift while the range lerps.
			{
				const double targetStart = static_cast<double>(memoryDbg.viewStart);
				const double targetRange = memoryDbg.viewRange;
				const double dt           = ImGui::GetIO().DeltaTime;
				const double alpha        = 1.0 - std::exp(-25.0 * dt);

				if (memoryDbg.smoothViewRange < 0.0)
				{
					memoryDbg.smoothViewRange = targetRange;
				}
				else
				{
					memoryDbg.smoothViewRange =
					    p::Lerp(memoryDbg.smoothViewRange, targetRange, alpha);
				}
				if (memoryDbg.isZooming)
				{
					// Keep the anchor address fixed under the cursor while the
					// range smoothly lerps to its target.
					memoryDbg.smoothViewStart =
					    static_cast<double>(memoryDbg.zoomAnchorAddr)
					    - static_cast<double>(memoryDbg.zoomAnchorRelX) * memoryDbg.smoothViewRange;
					if (p::Abs(memoryDbg.smoothViewRange - targetRange) < 0.5)
					{
						memoryDbg.isZooming = false;
					}
				}
				else if (memoryDbg.smoothViewStart < 0.0)
				{
					memoryDbg.smoothViewStart = targetStart;
				}
				else
				{
					memoryDbg.smoothViewStart =
					    p::Lerp(memoryDbg.smoothViewStart, targetStart, alpha);
				}
			}

			// Use the smooth values for rendering (AddrToX, ruler, clamps)
			viewStart = memoryDbg.smoothViewStart;
			viewRange = memoryDbg.smoothViewRange;
			viewEnd   = viewStart + viewRange;

			// Width (in pixels) of each hidden range when compressed to a zigzag.
			constexpr float COMPRESSED_W = 20.0f;

			// Total bytes of hidden ranges whose END is at or before `a`.
			// Hidden ranges split the visible range into segments; each hidden
			// range is collapsed to COMPRESSED_W pixels in the mapping below.
			auto HiddenBytesBefore = [&](uintptr_t a) -> double
			{
				double total = 0.0;
				for (const auto& h : memoryDbg.hiddenRanges)
				{
					if (h.end <= a)
					{
						total += static_cast<double>(h.end - h.start);
					}
				}
				return total;
			};
			// Number of hidden ranges fully inside the view (contribute COMPRESSED_W each)
			const double hiddenBytesTotal =
			    HiddenBytesBefore(static_cast<uintptr_t>(viewStart + viewRange));
			const int hiddenCountInView = [&]() -> int
			{
				int n = 0;
				for (const auto& h : memoryDbg.hiddenRanges)
				{
					if (h.end > static_cast<uintptr_t>(viewStart)
					    && h.start < static_cast<uintptr_t>(viewStart + viewRange))
					{
						++n;
					}
				}
				return n;
			}();
			const double effectiveViewRange =
			    viewRange - hiddenBytesTotal + hiddenCountInView * COMPRESSED_W;
			const double safeEffectiveViewRange =
			    effectiveViewRange > 1.0 ? effectiveViewRange : 1.0;

			// Compressed address → X. Addresses inside a hidden range are
			// clamped to the range's end (so they map to the zigzag boundary).
			auto AddrToX = [&](uintptr_t a) -> float
			{
				uintptr_t aa = a;
				if (aa < static_cast<uintptr_t>(viewStart))
				{
					aa = static_cast<uintptr_t>(viewStart);
				}
				if (aa > static_cast<uintptr_t>(viewStart + viewRange))
				{
					aa = static_cast<uintptr_t>(viewStart + viewRange);
				}
				double hidden = HiddenBytesBefore(aa);
				for (const auto& h : memoryDbg.hiddenRanges)
				{
					if (aa >= h.start && aa < h.end)
					{
						hidden += static_cast<double>(aa - h.start);
						aa = h.end;
					}
				}
				const double visibleOffset = static_cast<double>(aa - viewStart) - hidden;
				return graphX0
				     + static_cast<float>(visibleOffset / safeEffectiveViewRange * graphW);
			};

			// Inverse: screen X → true address (uncompressed, for input).
			auto ScreenXToAddr = [&](float x) -> uintptr_t
			{
				const double t = (static_cast<double>(x) - graphX0) / graphW;
				return static_cast<uintptr_t>(viewStart + t * viewRange);
			};

			constexpr float rulerH       = 24.0f;
			constexpr float rowH         = 20.0f;
			constexpr float allocHeaderH = 14.0f;

			// Ruler bar (at top of graph area) — uses popup/submenu background color
			{
				drawList->AddRectFilled(ImVec2(graphX0, canvasPos.y),
				    ImVec2(graphX0 + graphW, canvasPos.y + rulerH),
				    ImGui::GetColorU32(ImGuiCol_PopupBg));

				double pixelsPerByte = graphW / viewRange;

				// Snap a minimum byte count up to the nearest valid scale:
				// {16, 64, 256, 1024, 4096, 16384, 65536, ...}
				// = 2^4, 2^6, 2^8, 2^10, 2^12, ... (multiples of 4 starting at 16)
				auto SnapToScale = [](double minBytes) -> double
				{
					if (minBytes <= 16.0)
					{
						return 16.0;
					}
					double e = std::ceil(std::log2(minBytes));
					e += std::fmod(e, 2.0);    // round up to even
					return std::pow(2.0, e);
				};

				// First snap: target ~100px between major ticks
				double majorStep = SnapToScale(100.0 / pixelsPerByte);

				// Re-snap if the first tick's label would clip the next tick
				{
					double firstTick = std::ceil(viewStart / majorStep) * majorStep;
					if (firstTick < viewStart)
					{
						firstTick += majorStep;
					}
					char sample[32];
					const int textLen = snprintf(sample, sizeof(sample), "0x%llX",
					    static_cast<unsigned long long>(static_cast<uintptr_t>(firstTick)));
					if (textLen > 0)
					{
						const float textWidth = static_cast<float>(textLen) * 7.0f + 20.0f;
						if (textWidth > static_cast<float>(majorStep * pixelsPerByte))
						{
							majorStep = SnapToScale(static_cast<double>(textWidth) / pixelsPerByte);
						}
					}
				}

				double qtrStep = majorStep / 4.0;

				// Current scale label (top-left)
				{
					const String scaleStr = Strings::ParseMemorySize(static_cast<sizet>(majorStep));
					drawList->AddText(ImVec2(canvasPos.x + 4.0f, canvasPos.y + 2.0f),
					    p::Color{180, 180, 180}.ToPackedABGR(), scaleStr.data());
				}

				// Start at first multiple of majorStep within view
				double tickAddr = std::ceil(viewStart / majorStep) * majorStep;

				// Draw the partial first section: quarter/half ticks between
				// viewStart and the first major tick (the main loop only draws
				// q=1,2,3 relative to each major tick, so the range before
				// the first major tick would otherwise be empty).
				if (tickAddr > viewStart)
				{
					const double prevMajor = tickAddr - majorStep;
					double ta              = std::ceil(viewStart / qtrStep) * qtrStep;
					if (ta < prevMajor + qtrStep)
					{
						ta = prevMajor + qtrStep;
					}
					for (; ta < tickAddr && ta <= viewEnd; ta += qtrStep)
					{
						const int q    = static_cast<int>((ta - prevMajor) / qtrStep + 0.5);
						const float tx = AddrToX(static_cast<uintptr_t>(ta));
						if (q == 2)
						{
							drawList->AddLine(ImVec2(tx, canvasPos.y + rulerH - 10),
							    ImVec2(tx, canvasPos.y + rulerH),
							    p::Color{150, 150, 150, 200}.ToPackedABGR(), 1.2f);
						}
						else
						{
							drawList->AddLine(ImVec2(tx, canvasPos.y + rulerH - 6),
							    ImVec2(tx, canvasPos.y + rulerH),
							    p::Color{120, 120, 120, 150}.ToPackedABGR(), 1.0f);
						}
					}
				}

				for (; tickAddr <= viewEnd; tickAddr += majorStep)
				{
					// Quarter ticks (4 subdivisions per major: 0, 1/4, 1/2, 3/4)
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
							// Major — bright tick in the timeline with the
							// address label.
							drawList->AddLine(ImVec2(tx, canvasPos.y + rulerH - 14),
							    ImVec2(tx, canvasPos.y + rulerH),
							    p::Color{220, 220, 220}.ToPackedABGR(), 1.5f);

							// Address label split into two colors: gray for the "0x" prefix
							// and leading digits, white for the trailing digits that
							// match the current scale (the part that changes each tick).
							char buf[32];
							snprintf(buf, sizeof(buf), "0x%llX",
							    static_cast<unsigned long long>(static_cast<uintptr_t>(ta)));

							// Number of hex digits in the scale step
							int scaleDigits = 1;
							{
								u64 ss = static_cast<u64>(majorStep);
								while (ss >= 16)
								{
									ss /= 16;
									++scaleDigits;
								}
								const int totalHex = (int)strlen(buf) - 2;
								const int leading  = totalHex - scaleDigits;
								if (leading > 0)
								{
									char grayPart[32];
									const int grayLen = 2 + leading;
									memcpy(grayPart, buf, grayLen);
									grayPart[grayLen] = '\0';
									drawList->AddText(ImVec2(tx + 3.0f, canvasPos.y + rulerH - 14),
									    p::Color{140, 140, 140}.ToPackedABGR(), grayPart);
									const float grayW = ImGui::CalcTextSize(grayPart).x;
									drawList->AddText(
									    ImVec2(tx + 3.0f + grayW, canvasPos.y + rulerH - 14),
									    p::Color{230, 230, 230}.ToPackedABGR(), buf + grayLen);
								}
								else
								{
									drawList->AddText(ImVec2(tx + 3.0f, canvasPos.y + rulerH - 14),
									    p::Color{230, 230, 230}.ToPackedABGR(), buf);
								}
							}

							// Remember this major tick so we can draw a darker grid
							// line through the graph area AFTER the rows (on top of
							// block fills so it isn't covered).
							majorTickXs.Add(tx);
						}
						else if (q == 2)
						{
							// Half — medium gray bar
							drawList->AddLine(ImVec2(tx, canvasPos.y + rulerH - 10),
							    ImVec2(tx, canvasPos.y + rulerH),
							    p::Color{150, 150, 150, 200}.ToPackedABGR(), 1.2f);
						}
						else
						{
							// Quarter — dimmer, shorter gray bar
							drawList->AddLine(ImVec2(tx, canvasPos.y + rulerH - 6),
							    ImVec2(tx, canvasPos.y + rulerH),
							    p::Color{120, 120, 120, 150}.ToPackedABGR(), 1.0f);
						}
					}
				}
			}

			// Content area background — matches the menu bar color so the arenas,
			// hex and string regions all sit on a uniform panel beneath the timeline.
			{
				const ImU32 panelBg = ImGui::GetColorU32(ImGuiCol_MenuBarBg);
				drawList->AddRectFilled(ImVec2(canvasPos.x, canvasPos.y + rulerH),
				    ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), panelBg);
			}

			// Hex/ASCII drawn inside the graph child so it shares the interactable area.
			// Each band/column is a distinct, delimited region. Map each visible byte
			// to its screen position using the zoomed view range so single bytes
			// get a readable amount of space when zoomed in.
			if (hexBandH > 0 || asciiBandH > 0)
			{
				const float hexY0   = canvasPos.y + rulerH + 2.0f;
				const float asciiY0 = hexY0 + hexBandH + bandGap;

				// Section headers on the left
				if (hexBandH > 0)
				{
					drawList->AddText(ImVec2(graphX0 + 4.0f, hexY0 + 3.0f),
					    p::Color{180, 180, 180}.ToPackedABGR(), "HEX");
				}
				if (asciiBandH > 0)
				{
					drawList->AddText(ImVec2(graphX0 + 4.0f, asciiY0 + 3.0f),
					    p::Color{180, 180, 180}.ToPackedABGR(), "String");
				}

				if (selectedInfo || !memoryDbg.arenaInfos.IsEmpty())
				{
					const auto& info = selectedInfo ? *selectedInfo : memoryDbg.arenaInfos[0];
					if (info.blockBegin && info.blockSize > 0)
					{
						// Compute the visible address range and render only bytes
						// inside it. Iterate ALL blocks of the arena so later
						// (higher-address) blocks also render when in view, and
						// each block reads from its own data pointer (non-contiguous).
						const uintptr_t viewLo    = (uintptr_t)viewStart;
						const uintptr_t viewHi    = (uintptr_t)(viewStart + viewRange);
						const float pixelsPerByte = static_cast<float>(graphW / viewRange);

						if (pixelsPerByte >= 6.0f)
						{
							for (int b = 0; b < info.blocks.Size(); ++b)
							{
								const uintptr_t bs = reinterpret_cast<uintptr_t>(info.blocks[b]);
								const uintptr_t be = bs + info.blockSizes[b];
								if (be <= viewLo || bs >= viewHi)
								{
									continue;
								}

								const uintptr_t firstByte = (bs > viewLo) ? bs : viewLo;
								const uintptr_t lastByte  = (be < viewHi) ? be : viewHi;
								if (lastByte <= firstByte)
								{
									continue;
								}

								const u8* data = static_cast<const u8*>(info.blocks[b]);

								for (uintptr_t a = firstByte; a < lastByte; ++a)
								{
									// Use the compressed AddrToX so hidden ranges
									// are reflected and bytes align with the blocks.
									const float x = AddrToX(a);
									const u8 byte = data[a - bs];

									if (hexBandH > 0)
									{
										char hex[3];
										snprintf(hex, sizeof(hex), "%02X", byte);
										const ImU32 hexCol =
										    (byte == 0)
										        ? p::Color{90, 90, 90, 255}.ToPackedABGR()
										        : p::Color{220, 220, 220, 255}.ToPackedABGR();
										drawList->AddText(ImVec2(x, hexY0), hexCol, hex);
									}
									if (asciiBandH > 0)
									{
										const bool printable = (byte >= 32 && byte < 127);
										const char c = printable ? static_cast<char>(byte) : '.';
										const ImU32 asciiCol =
										    printable ? p::Color{220, 220, 220, 255}.ToPackedABGR()
										              : p::Color{90, 90, 90, 255}.ToPackedABGR();
										drawList->AddText(ImVec2(x, asciiY0), asciiCol, &c, &c + 1);
									}
								}
							}
						}
					}
				}
			}
			// Arena rows
			float rowsY0 = canvasPos.y + rulerH + hexLineHeight + 6.0f + allocHeaderH;

			// "Alloc" header above the arena rows
			drawList->AddText(ImVec2(graphX0 + 4.0f, rowsY0 - allocHeaderH + 2.0f),
			    p::Color{180, 180, 180}.ToPackedABGR(), "Alloc");

			float yPos = 0.0f;
			for (int i = 0; i < memoryDbg.arenaInfos.Size(); ++i)
			{
				const auto& info = memoryDbg.arenaInfos[i];

				const p::Color baseFill = details::GetArenaColor(info.typeId);
				const bool isSelected   = (i == memoryDbg.selectedArena);
				// Selected rows keep their arena color, slightly darkened, with
				// an orange outline so they stand out without losing identity.
				const p::Color fill    = isSelected ? (baseFill * 0.65f) : baseFill;
				const p::Color outline = isSelected ? p::Color::Orange() : p::Color{0, 0, 0, 80};

				const ImU32 fillU32    = fill.ToPackedABGR();
				const ImU32 outlineU32 = outline.ToPackedABGR();

				float rowY = rowsY0 + yPos;

				// Blocks at address positions
				if (info.blockBegin && info.blockSize > 0)
				{
					for (int b = 0; b < info.blocks.Size(); ++b)
					{
						const float x0Raw = AddrToX(reinterpret_cast<uintptr_t>(info.blocks[b]));
						const float x1Raw = AddrToX(
						    reinterpret_cast<uintptr_t>(info.blocks[b]) + info.blockSizes[b]);

						// Skip blocks entirely outside the visible graph area
						if (x1Raw < graphX0 || x0Raw > graphX0 + graphW)
						{
							continue;
						}

						float x0 = x0Raw;
						float x1 = x1Raw;
						// Enforce a minimum visible width of 2px so tiny blocks
						// (small block size or far-zoomed-out) stay perceivable.
						// The rect is centered on the block's address midpoint.
						if (x1 - x0 < 2.0f)
						{
							const float mid = (x0 + x1) * 0.5f;
							x0              = mid - 1.0f;
							x1              = mid + 1.0f;
						}
						// Clamp to graph area
						x0 = (x0 > graphX0) ? x0 : graphX0;
						x1 = (x1 < graphX0 + graphW) ? x1 : (graphX0 + graphW);

						drawList->AddRectFilled(ImVec2(x0, rowY), ImVec2(x1, rowY + rowH), fillU32);
						drawList->AddRect(ImVec2(x0, rowY), ImVec2(x1, rowY + rowH), outlineU32);

						// Double-click to focus: set the view to fit and center this block
						if (ImGui::IsMouseDoubleClicked(0))
						{
							const ImRect blockRect(ImVec2(x0, rowY), ImVec2(x1, rowY + rowH));
							if (blockRect.Contains(ImGui::GetIO().MousePos))
							{
								const uintptr_t blkStart =
								    reinterpret_cast<uintptr_t>(info.blocks[b]);
								const double blkSize = static_cast<double>(info.blockSizes[b]);
								if (blkSize > 0.0)
								{
									// Cap the view range: min so each byte ≤ 24px,
									// max so it does not exceed the content size.
									const double minViewRange =
									    (graphW / 24.0 > 1.0) ? (graphW / 24.0) : 1.0;
									double newViewRange = blkSize;
									if (newViewRange < minViewRange)
									{
										newViewRange = minViewRange;
									}
									if (newViewRange > range)
									{
										newViewRange = range;
									}
									// Center the block in the visible range
									const double center =
									    static_cast<double>(blkStart) + blkSize * 0.5;
									const double newViewStart = center - newViewRange * 0.5;
									memoryDbg.viewStart = static_cast<uintptr_t>(newViewStart);
									memoryDbg.viewRange = newViewRange;
									memoryDbg.smoothViewStart = newViewStart;    // snap on focus
									memoryDbg.smoothViewRange = newViewRange;
								}
							}
						}
					}
				}

				// Per-allocation markers (debug invalid/unfreed allocations).
				// Live = green, freed-but-tracked = red/orange. Drawn as a thin
				// colored line at the top of the row at the allocation's address.
				// We read stats->allocations directly under a shared_lock — no copy
				// into a TArray (which would recurse into Alloc and deadlock).
				{
					const uintptr_t arenaStart = reinterpret_cast<uintptr_t>(info.blockBegin);
					const uintptr_t arenaEnd = info.blockBegin ? (arenaStart + info.blockSize) : 0;
					const float tickH        = 3.0f;

					auto DrawTick = [&](uintptr_t addr, ImU32 color)
					{
						if (addr < arenaStart || addr >= arenaEnd)
						{
							return;
						}
						float tx = AddrToX(addr);
						if (tx < graphX0 || tx > graphX0 + graphW)
						{
							return;
						}
						drawList->AddRectFilled(
						    ImVec2(tx - 0.5f, rowY), ImVec2(tx + 0.5f, rowY + tickH), color);
					};

					const MemoryStats* rowStats = info.arena ? info.arena->GetStats() : nullptr;
					if (rowStats)
					{
						std::shared_lock<std::shared_mutex> statsLock(rowStats->mutex);
						for (const auto& a : rowStats->allocations)
						{
							DrawTick(reinterpret_cast<uintptr_t>(a.ptr),
							    p::Color::Emerald().ToPackedABGR());
						}
						for (const auto& a : rowStats->freedAllocations)
						{
							DrawTick(reinterpret_cast<uintptr_t>(a.ptr),
							    p::Color{230, 90, 60, 220}.ToPackedABGR());
						}
					}
				}

				// Arena label
				static String arenaTypeName;
				arenaTypeName = GetTypeName(info.typeId);
				drawList->AddText(ImVec2(graphX0 + 4.0f, rowY + 2.0f),
				    p::Color{220, 220, 220}.ToPackedABGR(), arenaTypeName.c_str());

				// Selection click
				ImRect rowRect(ImVec2(graphX0, rowY), ImVec2(graphX0 + graphW, rowY + rowH));
				if (rowRect.Contains(ImGui::GetIO().MousePos) && ImGui::IsMouseClicked(0))
				{
					memoryDbg.selectedArena = i;
				}

				// Tooltip on hover
				if (rowRect.Contains(ImGui::GetIO().MousePos))
				{
					ImGui::BeginTooltip();
					ImGui::Text("Arena: %s", arenaTypeName.data());
					ImGui::Text("Block: 0x%p", info.blockBegin);
					const String sizeStr = Strings::ParseMemorySize(info.blockSize);
					ImGui::Text("Size: %s (%zuB)", sizeStr.data(), info.blockSize);
					if (info.usedSize > 0 || info.freeSize > 0)
					{
						ImGui::Text("Used: %zu (%.1f%%)", info.usedSize,
						    100.0f * info.usedSize / info.blockSize);
					}
					ImGui::EndTooltip();
				}

				yPos += rowH + 2.0f;
			}

			// Full interactable area of the graph child (for hit-testing inputs)
			const ImRect graphRect(canvasPos, canvasPos + canvasSize);

			// Hidden ranges: draw a vertical zigzag for each, spanning the content
			// area. Click a zigzag to expand (remove) that hidden range.
			{
				const float zigY0    = canvasPos.y + rulerH;
				const float zigY1    = rowsY0;
				const ImU32 zigColor = p::Color::Silver().ToPackedABGR();
				for (int hi = 0; hi < memoryDbg.hiddenRanges.Size(); ++hi)
				{
					const auto& h = memoryDbg.hiddenRanges[hi];
					if (h.end <= static_cast<uintptr_t>(viewStart)
					    || h.start >= static_cast<uintptr_t>(viewStart + viewRange))
					{
						continue;
					}
					const float x = AddrToX(h.end);
					details::DrawZigzagVertical(drawList, x, zigY0, zigY1, 3.0f, zigColor);

					// Click to expand. Detect a click within a small x margin
					// around the zigzag line, anywhere in the content area.
					if (ImGui::IsMouseClicked(0) && !ImGui::IsMouseDragging(0))
					{
						const float mx = ImGui::GetIO().MousePos.x;
						if (mx >= x - 6.0f && mx <= x + 6.0f && ImGui::GetIO().MousePos.y >= zigY0
						    && ImGui::GetIO().MousePos.y <= zigY1)
						{
							memoryDbg.hiddenRanges.RemoveAt(hi);
							--hi;
						}
					}
				}
			}

			// Horizontal area selection: left-mouse drag in the graph creates a
			// range selection. Right-click on the selection opens a context menu.
			{
				const ImVec2 mousePos = ImGui::GetIO().MousePos;
				const bool inGraph    = graphRect.Contains(mousePos);

				// Start a selection on left-mouse-down
				if (ImGui::IsMouseClicked(0) && inGraph && !memoryDbg.isSelecting)
				{
					memoryDbg.isSelecting    = true;
					memoryDbg.selectAnchor   = ScreenXToAddr(mousePos.x);
					memoryDbg.selectionStart = memoryDbg.selectAnchor;
					memoryDbg.selectionEnd   = memoryDbg.selectAnchor;
					memoryDbg.selectionValid = true;
				}
				// Update selection while dragging
				if (memoryDbg.isSelecting && ImGui::IsMouseDragging(0))
				{
					const uintptr_t a        = memoryDbg.selectAnchor;
					const uintptr_t b        = ScreenXToAddr(mousePos.x);
					memoryDbg.selectionStart = (a < b) ? a : b;
					memoryDbg.selectionEnd   = (a < b) ? b : a;
				}
				// Finalize on mouse-up
				if (memoryDbg.isSelecting && ImGui::IsMouseReleased(0))
				{
					memoryDbg.isSelecting = false;
					// Alt+Left release: focus the selection immediately
					if (memoryDbg.selectionValid
					    && memoryDbg.selectionEnd > memoryDbg.selectionStart
					    && ImGui::GetIO().KeyAlt)
					{
						const uintptr_t s = memoryDbg.selectionStart;
						const uintptr_t e = memoryDbg.selectionEnd;
						const double size = static_cast<double>(e - s);
						if (size > 0.0)
						{
							memoryDbg.viewRange       = size;
							memoryDbg.viewStart       = s;
							memoryDbg.smoothViewStart = static_cast<double>(s);
							memoryDbg.forceZoom       = true;
						}
					}
				}

				// Draw the selection rectangle
				if (memoryDbg.selectionValid && memoryDbg.selectionEnd > memoryDbg.selectionStart)
				{
					const float sx0 = AddrToX(memoryDbg.selectionStart);
					const float sx1 = AddrToX(memoryDbg.selectionEnd);
					drawList->AddRectFilled(ImVec2(sx0, canvasPos.y + rulerH),
					    ImVec2(sx1, canvasPos.y + canvasSize.y),
					    p::Color{255, 200, 80, 50}.ToPackedABGR());
					drawList->AddRect(ImVec2(sx0, canvasPos.y + rulerH),
					    ImVec2(sx1, canvasPos.y + canvasSize.y),
					    p::Color{255, 200, 80, 220}.ToPackedABGR());
				}

				// Right-click within the selection opens the context menu
				// (skip when over a block — blocks may get their own interactions)
				if (ImGui::IsMouseClicked(1) && inGraph && memoryDbg.selectionValid
				    && memoryDbg.selectionEnd > memoryDbg.selectionStart)
				{
					const float sx0 = AddrToX(memoryDbg.selectionStart);
					const float sx1 = AddrToX(memoryDbg.selectionEnd);
					if (mousePos.x >= sx0 && mousePos.x <= sx1)
					{
						ImGui::OpenPopup("MemorySelectionMenu");
					}
				}

				if (ImGui::BeginPopup("MemorySelectionMenu"))
				{
					if (ImGui::MenuItem("Focus selection"))
					{
						const uintptr_t s = memoryDbg.selectionStart;
						const uintptr_t e = memoryDbg.selectionEnd;
						const double size = static_cast<double>(e - s);
						if (size > 0.0)
						{
							memoryDbg.viewRange       = size;
							memoryDbg.viewStart       = s;
							memoryDbg.smoothViewStart = static_cast<double>(s);
							memoryDbg.forceZoom       = true;
						}
					}
					if (ImGui::MenuItem("Hide (compress to zigzag)"))
					{
						memoryDbg.hiddenRanges.Add(
						    {memoryDbg.selectionStart, memoryDbg.selectionEnd});
						// Sort by start for the compressed mapping
						memoryDbg.hiddenRanges.Sort([](const DebugMemoryContext::HiddenRange& a,
						                                const DebugMemoryContext::HiddenRange& b)
						{
							return a.start < b.start;
						});
						memoryDbg.selectionValid = false;
					}
					ImGui::EndPopup();
				}
			}

			// Draw the major-tick grid lines through the graph area ON TOP of
			// the rows (darker than the bright timeline tick, but still visible).
			{
				const ImU32 gridCol = p::Color{120, 120, 120}.ToPackedABGR();
				for (int t = 0; t < majorTickXs.Size(); ++t)
				{
					drawList->AddLine(ImVec2(majorTickXs[t], canvasPos.y + rulerH),
					    ImVec2(majorTickXs[t], canvasPos.y + canvasSize.y), gridCol, 1.0f);
				}
			}

			// Zoom / pan over the full child window (ruler, hex/ascii, rows, empty)
			// (graphRect is defined before the selection/hidden-ranges block below)

			// Mouse wheel: Alt+wheel pans horizontally, wheel alone zooms
			if (graphRect.Contains(ImGui::GetIO().MousePos))
			{
				float wheel = ImGui::GetIO().MouseWheel;
				if (wheel != 0.0f)
				{
					if (ImGui::GetIO().KeyAlt)
					{
						// Alt+wheel: pan horizontally (one notch ≈ 5% of visible range)
						// Wheel up/right → view advances to higher addresses.
						const double addrDelta = static_cast<double>(wheel) * viewRange * 0.05;
						memoryDbg.viewStart    = static_cast<uintptr_t>(viewStart + addrDelta);
					}
					else
					{
						float mx = ImGui::GetIO().MousePos.x;
						double addrAtCursor;
						if (mx <= graphX0)
						{
							// Cursor in hex column or before graph area — anchor at view start
							addrAtCursor = viewStart;
						}
						else
						{
							double relX  = static_cast<double>(mx - graphX0) / graphW;
							addrAtCursor = viewStart + relX * viewRange;
						}

						// Zoom in/out: shrink/grow the visible byte range.
						// Cap zoom-in so each byte is at most 24px wide.
						// Cap zoom-out to the content size so the view never
						// escapes the memory range into terabytes of empty space.
						const double minViewRange = (graphW / 24.0 > 1.0) ? (graphW / 24.0) : 1.0;
						double newViewRange = (wheel > 0) ? (viewRange / 1.15) : (viewRange * 1.15);
						if (newViewRange < minViewRange)
						{
							newViewRange = minViewRange;
						}
						if (newViewRange > range)
						{
							newViewRange = range;
						}

						// Anchor the address under the cursor. Store the anchor and
						// its relative x; the top-of-frame couples smoothViewStart
						// to smoothViewRange so the anchor stays fixed while the
						// range lerps (no left/right flicker).
						double relX = 0.0;
						if (mx > graphX0)
						{
							relX = static_cast<double>(mx - graphX0) / graphW;
						}
						const uintptr_t anchorAddr = (uintptr_t)(viewStart + relX * viewRange);
						const double newViewStart  = (double)anchorAddr - relX * newViewRange;

						memoryDbg.viewStart      = static_cast<uintptr_t>(newViewStart);
						memoryDbg.viewRange      = newViewRange;
						memoryDbg.isZooming      = true;
						memoryDbg.zoomAnchorAddr = anchorAddr;
						memoryDbg.zoomAnchorRelX = static_cast<float>(relX);
					}
				}
			}

			// Drag to pan — middle mouse button only.
			// Left-drag is reserved for area selection.
			// Drag right → view moves to LOWER addresses (grab/inverted model:
			// the content follows the mouse, like dragging a map).
			if (graphRect.Contains(ImGui::GetIO().MousePos) && ImGui::IsMouseDragging(2))
			{
				// Cancel any in-progress zoom coupling so pan works normally.
				memoryDbg.isZooming = false;
				const float dx      = ImGui::GetIO().MouseDelta.x;
				if (dx != 0.0f)
				{
					const double addrDelta = -static_cast<double>(dx) / graphW * viewRange;
					memoryDbg.viewStart    = static_cast<uintptr_t>(viewStart + addrDelta);
				}
			}
		}
		ImGui::EndChild();

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
			{
				const String sizeStr = Strings::ParseMemorySize(selectedInfo->blockSize);
				ImGui::Text("Size: %s (%zuB)", sizeStr.data(), selectedInfo->blockSize);
			}

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
				snprintf(blockLabel, sizeof(blockLabel), "Block %d: 0x%p (%zuB)", b,
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
