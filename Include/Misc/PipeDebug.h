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
		bool showHex     = true;
		bool showAscii   = true;
		i32 bytesPerLine = 4;

		bool showDetails      = true;
		bool resetLayout      = true;
		ImGuiID graphDockId   = 0;
		ImGuiID detailsDockId = 0;

		// Absolute view state (stable across reallocations of the ruler).
		// viewStart: target left-edge address (set by user input, clamped to data).
		// viewRange: target visible byte range (0 = uninitialized).
		// smoothViewStart / smoothViewRange: actual rendered values (<0 = uninit).
		// On pan the target moves and the smooth position lerps toward it.
		// On wheel zoom the range lerps smoothly (start snaps to keep the
		// anchor under the cursor fixed). On Focus/double-click both snap.
		sizet viewStart        = 0;
		double viewRange       = 0.0;
		double smoothViewStart = -1.0;
		double smoothViewRange = -1.0;

		// "Go to" address input
		char gotoBuffer[32] = "";
		sizet pendingGoto   = 0;

		// Cached graph width (updated each frame, used to position the goto offset)
		float lastGraphW = 1500.0f;

		// Address ranges hidden by the user (compressed to a zigzag). Stored
		// sorted by start; the compressed AddrToX accounts for them.
		struct HiddenRange
		{
			sizet start = 0;
			sizet end   = 0;
		};
		TArray<HiddenRange> hiddenRanges;

		// Horizontal area selection (left-mouse drag in the graph)
		bool hasSelection         = false;
		sizet selectionStart      = 0;
		sizet selectionEnd        = 0;
		i32 selectionArenaIdx     = NO_INDEX;
		i32 selectionBlockIdx     = NO_INDEX;
		bool isSelecting          = false;
		sizet selectionFirstAddr  = 0;
		sizet selectionSecondAddr = 0;

		// When true, the next view-update bypasses the minimum view-range
		// (24px/byte) cap so a "Focus selection" can fully zoom in.
		bool forceZoom = false;

		// Zoom anchor: while isZooming is true, smoothViewStart is coupled to
		// smoothViewRange so the address under the cursor stays fixed during
		// the range lerp (prevents the left/right flicker).
		bool isZooming       = false;
		sizet zoomAnchorAddr = 0;
		float zoomAnchorRelX = 0.0f;

		struct ArenaSnapshot
		{
			const Arena* arena = nullptr;
			// Minimum allocated address
			const u8* begin = nullptr;
			// Maximum allocated address (exclusive)
			const u8* end  = nullptr;
			sizet capacity = 0;
			sizet used     = 0;
			TArray<ArenaBlock> blocks;
			TArray<sizet> blockSizes;
			Tag name;
			TypeId typeId;
			Tag typeName;
			TArray<MemoryStatsEvent> events;
		};
		TArray<ArenaSnapshot> snapshots;
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
					for (i32 i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
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

		// Draw `text` vertically. Uses only public ImGui APIs
		// (ImGui::GetFont, ImFont::GetFontBaked, ImFontBaked::FindGlyph, and
		// ImDrawList::PrimReserve/PrimQuadUV/PrimUnreserve) so the file
		// compiles without imgui_internal.h.

		// Bottom-to-top: pos is the screen position of the BOTTOM-LEFT corner
		// of the first glyph. The text rises upward from there. Each glyph's
		// top face points RIGHT (tilt head right to read).
		static void AddTextVertical(ImDrawList* draw, ImVec2 pos, ImU32 col, const char* text_begin,
		    const char* text_end = nullptr)
		{
			if (!text_end)
			{
				text_end = text_begin + strlen(text_begin);
			}
			if (text_begin == text_end || draw == nullptr)
			{
				return;
			}
			ImFont* imguiFont   = ImGui::GetFont();
			float imguiFontSize = ImGui::GetFontSize();
			if (imguiFont == nullptr)
			{
				return;
			}
			ImFontBaked* font = imguiFont->GetFontBaked(imguiFontSize);
			if (font == nullptr)
			{
				return;
			}
			const float scale       = imguiFontSize / font->Size;
			v2 cursor               = v2{RoundToZero(pos.x), RoundToZero(pos.y)};
			const char* s           = text_begin;
			i32 chars_exp           = (i32)(text_end - s);
			i32 chars_rnd           = 0;
			const i32 vtx_count_max = chars_exp * 4;
			const i32 idx_count_max = chars_exp * 6;
			draw->PrimReserve(idx_count_max, vtx_count_max);
			while (s < text_end)
			{
				const u32 c = (u32)*s;
				s += 1;
				const ImFontGlyph* glyph = font->FindGlyph((ImWchar)c);
				if (glyph == nullptr)
				{
					continue;
				}
				// Bottom-to-top rotation: glyph X axis maps to screen Y
				// (negated), glyph Y axis maps to screen X.
				const v2 p0 = cursor + v2{glyph->Y0, -glyph->X0} * scale;
				const v2 p1 = cursor + v2{glyph->Y0, -glyph->X1} * scale;
				const v2 p2 = cursor + v2{glyph->Y1, -glyph->X1} * scale;
				const v2 p3 = cursor + v2{glyph->Y1, -glyph->X0} * scale;
				draw->PrimQuadUV(ToIM(p0), ToIM(p1), ToIM(p2), ToIM(p3),
				    ToIM(v2{glyph->U0, glyph->V0}), ToIM(v2{glyph->U1, glyph->V0}),
				    ToIM(v2{glyph->U1, glyph->V1}), ToIM(v2{glyph->U0, glyph->V1}), col);
				cursor.y -= glyph->AdvanceX * scale;
				chars_rnd++;
			}
			i32 chars_skp = chars_exp - chars_rnd;
			draw->PrimUnreserve(chars_skp * 6, chars_skp * 4);
		}

		// Top-to-bottom: pos is the TOP-LEFT corner of the first glyph. Text
		// descends downward. Each glyph's top face points RIGHT.
		static void AddTextVerticalTopDown(ImDrawList* draw, ImVec2 pos, ImU32 col,
		    const char* text_begin, const char* text_end = nullptr)
		{
			if (!text_end)
			{
				text_end = text_begin + strlen(text_begin);
			}
			if (text_begin == text_end || draw == nullptr)
			{
				return;
			}
			ImFont* imguiFont   = ImGui::GetFont();
			float imguiFontSize = ImGui::GetFontSize();
			if (imguiFont == nullptr)
			{
				return;
			}
			ImFontBaked* font = imguiFont->GetFontBaked(imguiFontSize);
			if (font == nullptr)
			{
				return;
			}
			const float scale       = imguiFontSize / font->Size;
			v2 cursor               = v2{RoundToZero(pos.x), RoundToZero(pos.y)};
			const char* s           = text_begin;
			i32 chars_exp           = (i32)(text_end - s);
			i32 chars_rnd           = 0;
			const i32 vtx_count_max = chars_exp * 4;
			const i32 idx_count_max = chars_exp * 6;
			draw->PrimReserve(idx_count_max, vtx_count_max);
			while (s < text_end)
			{
				const u32 c = (u32)*s;
				s += 1;
				const ImFontGlyph* glyph = font->FindGlyph((ImWchar)c);
				if (glyph == nullptr)
				{
					continue;
				}
				// Top-to-bottom rotation: glyph X axis maps to screen Y,
				// glyph Y axis maps to screen X (negated).
				const v2 p0 = cursor + v2{-glyph->Y0, glyph->X0} * scale;
				const v2 p1 = cursor + v2{-glyph->Y0, glyph->X1} * scale;
				const v2 p2 = cursor + v2{-glyph->Y1, glyph->X1} * scale;
				const v2 p3 = cursor + v2{-glyph->Y1, glyph->X0} * scale;
				draw->PrimQuadUV(ToIM(p0), ToIM(p1), ToIM(p2), ToIM(p3),
				    ToIM(v2{glyph->U0, glyph->V0}), ToIM(v2{glyph->U1, glyph->V0}),
				    ToIM(v2{glyph->U1, glyph->V1}), ToIM(v2{glyph->U0, glyph->V1}), col);
				cursor.y += glyph->AdvanceX * scale;
				chars_rnd++;
			}
			i32 chars_skp = chars_exp - chars_rnd;
			draw->PrimUnreserve(chars_skp * 6, chars_skp * 4);
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
		memoryDbg.snapshots.Clear();
		TArray<const Arena*> arenas;
		GetAllArenas(arenas);
		for (const auto* arena : arenas)
		{
			if (!arena)
			{
				continue;
			}

			DebugMemoryContext::ArenaSnapshot snapshot;
			snapshot.arena    = arena;
			snapshot.typeId   = arena->GetTypeId();
			snapshot.typeName = GetTypeName(snapshot.typeId);

			// Get blocks
			arena->GetBlocks(snapshot.blocks);
			for (const auto& block : snapshot.blocks)
			{
				if (!snapshot.begin || block.data < snapshot.begin)
				{
					snapshot.begin = (u8*)block.data;
				}
				const u8* blockEnd = (u8*)block.data + block.size;
				if (!snapshot.end || blockEnd > snapshot.end)
				{
					snapshot.end = blockEnd;
				}
				snapshot.capacity += block.size;
			}

			// Get stats if available
			const auto* stats = arena->GetStats();
			if (stats)
			{
				stats->CollectStats();
				snapshot.name = Tag(stats->name);
				snapshot.used = stats->used;
				// snapshot.events = stats->events;
			}

			memoryDbg.snapshots.Add(snapshot);
		}

		// Sort by block address for consistent rendering
		std::sort(memoryDbg.snapshots.begin(), memoryDbg.snapshots.end(),
		    [](const auto& a, const auto& b)
		{
			return a.begin < b.begin;
		});

		// Determine selected arena
		DebugMemoryContext::ArenaSnapshot* selectedArena = nullptr;
		if (memoryDbg.snapshots.IsValidIndex(memoryDbg.selectionArenaIdx))
		{
			selectedArena = &memoryDbg.snapshots[memoryDbg.selectionArenaIdx];
		}

		if (!ImGui::Begin(label, open, flags | ImGuiWindowFlags_MenuBar))
		{
			ImGui::End();
			return;
		}

		// Menu bar — selection inputs, focus button, arena count, help, settings.
		if (ImGui::BeginMenuBar())
		{
			// Selection range (read-only inputs)
			char startBuf[32] = "";
			char endBuf[32]   = "";
			if (memoryDbg.hasSelection)
			{
				snprintf(startBuf, sizeof(startBuf), "0x%llX", memoryDbg.selectionStart);
				snprintf(endBuf, sizeof(endBuf), "0x%llX", memoryDbg.selectionEnd);
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
				if (memoryDbg.hasSelection && memoryDbg.selectionEnd >= memoryDbg.selectionStart)
				{
					const sizet s     = memoryDbg.selectionStart;
					const sizet e     = memoryDbg.selectionEnd;
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
					memoryDbg.smoothViewStart = double(memoryDbg.viewStart);
					memoryDbg.smoothViewRange = memoryDbg.viewRange;
					memoryDbg.forceZoom       = true;
				}
			}
			ImGui::SameLine();
			ImGui::TextDisabled("Arenas: %d", i32(memoryDbg.snapshots.Size()));
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
				    "Middle mouse drag to pan\n"
				    "Mouse wheel to pan\n"
				    "Alt + Mouse wheel to zoom\n"
				    "Click a column to select\n"
				    "Double-click a block to focus it");
			}

			// Right-align Settings submenu
			const char* settingsLabel = "Settings";
			const float settingsW =
			    ImGui::CalcTextSize(settingsLabel).x + ImGui::GetStyle().FramePadding.x * 2.0f;
			ImGui::SetCursorPosX(
			    ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - settingsW);
			if (ImGui::BeginMenu(settingsLabel))
			{
				ImGui::SeparatorText("Graph");
				ImGui::MenuItem("Hex", nullptr, &memoryDbg.showHex);
				ImGui::MenuItem("ASCII", nullptr, &memoryDbg.showAscii);
				// Bytes/line combo: powers of 2 up to 32
				const i32 allowed[]  = {1, 2, 4, 8, 16, 32};
				const char* labels[] = {"1", "2", "4", "8", "16", "32"};
				i32 current          = 0;
				for (i32 i = 0; i < 6; ++i)
				{
					if (allowed[i] == memoryDbg.bytesPerLine)
					{
						current = i;
						break;
					}
				}
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Line Size");
				ImGui::SameLine();
				if (ImGui::Combo("##LineSize", &current, labels, 6))
				{
					memoryDbg.bytesPerLine = allowed[current];
				}
				ImGui::SeparatorText("View");
				ImGui::MenuItem("Details", nullptr, &memoryDbg.showDetails);
				ImGui::SeparatorText("Layout");
				if (ImGui::MenuItem("Reset"))
				{
					memoryDbg.resetLayout = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// DockSpace — splits 0.5 (right) for Details, the rest for the View graph.
		const ImGuiID dockspaceId = ImGui::GetID("MemoryDebuggerDockspace");
		if (memoryDbg.resetLayout || ImGui::DockBuilderGetNode(dockspaceId) == nullptr)
		{
			memoryDbg.resetLayout = false;
			ImGui::DockBuilderRemoveNode(dockspaceId);
			ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_KeepAliveOnly);
			ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.5f, &memoryDbg.detailsDockId,
			    &memoryDbg.graphDockId);
			ImGui::DockBuilderGetNode(memoryDbg.graphDockId)->LocalFlags |=
			    ImGuiDockNodeFlags_AutoHideTabBar;
			// Re-dock windows (SetNextWindowDockID FirstUseEver is one-shot)
			ImGui::DockBuilderDockWindow("View", memoryDbg.graphDockId);
			if (memoryDbg.showDetails)
			{
				ImGui::DockBuilderDockWindow("Details", memoryDbg.detailsDockId);
			}
			ImGui::DockBuilderFinish(dockspaceId);
		}
		ImGui::DockSpace(dockspaceId, ImVec2{0.0f, 0.0f});

		// ----- Layout vars -----
		const i32 bytesPerLine    = Max(memoryDbg.bytesPerLine, 1);
		const ImVec2 charTextSize = ImGui::CalcTextSize("0");
		const float stripPad      = 4.0f;
		const float leftGap       = 4.0f;
		const float colGap        = 4.0f;
		constexpr float colMaxW   = 32.0f;
		const float rulerStripW   = 32.0f;
		const float hexStripW =
		    memoryDbg.showHex ? (charTextSize.x * 2.0f * bytesPerLine + stripPad * 2.0f) : 0.0f;
		const float stringStripW =
		    memoryDbg.showAscii ? (charTextSize.x * 1.0f * bytesPerLine + stripPad * 2.0f) : 0.0f;

		// Compute desired graph width so the View window first-use size fits the
		// strips + all arena columns without horizontal scrolling.
		{
			const i32 arenaCount      = memoryDbg.snapshots.Size();
			const float desiredGraphW = rulerStripW + leftGap + hexStripW
			                          + (hexStripW > 0 ? leftGap : 0.0f) + stringStripW
			                          + (stringStripW > 0 ? leftGap : 0.0f)
			                          + (colMaxW + colGap) * arenaCount + colGap + 32.0f;
			ImGui::SetNextWindowSize(ImVec2(desiredGraphW, 400.0f), ImGuiCond_FirstUseEver);
		}
		if (ImGui::Begin("View", nullptr))
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			const ImVec2 canvasPos  = ImGui::GetCursorScreenPos();
			const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
			const ImRect graphRect  = ImRect(canvasPos, canvasPos + canvasSize);

			// Capture input over the whole View so the parent never starts a
			// window-drag from a click inside the graph. canvasPos is captured
			// BEFORE the button (the button advances the cursor).
			ImGui::InvisibleButton("##graph_input", canvasSize);
			ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));

			// Full interactable area of the View (declared early for hit-tests)

			// ----- Layout vars -----
			const float colAreaX0 = canvasPos.x + rulerStripW + leftGap + hexStripW
			                      + (hexStripW > 0 ? leftGap : 0.0f) + stringStripW
			                      + (stringStripW > 0 ? leftGap : 0.0f);
			const float colAreaW  = (canvasPos.x + canvasSize.x) - colAreaX0;
			const i32 arenaCount  = memoryDbg.snapshots.Size();
			const float colW =
			    (arenaCount > 0 && colAreaW > 0.0f)
			        ? p::Min((colAreaW - colGap * (arenaCount + 1)) / (float)arenaCount, colMaxW)
			        : 0.0f;
			const float addressY0 = canvasPos.y;
			const float addressY1 = canvasPos.y + canvasSize.y;
			const float addressH  = (addressY1 > addressY0) ? (addressY1 - addressY0) : 1.0f;
			const float hexX0     = canvasPos.x + rulerStripW + leftGap;
			const float stringX0  = hexX0 + hexStripW + (hexStripW > 0 ? leftGap : 0.0f);
			const float colX0     = stringX0 + stringStripW + (stringStripW > 0 ? leftGap : 0.0f);
			const float colTotalW = colW + colGap;
			const float graphX0   = canvasPos.x;
			const float graphW    = canvasSize.x;
			const float rulerW    = rulerStripW;

			// ----- Address range -----
			sizet addrMin = 0, addrMax = 0;
			for (const auto& snapshot : memoryDbg.snapshots)
			{
				if (!addrMin || sizet(snapshot.begin) < addrMin)
				{
					addrMin = sizet(snapshot.begin);
				}
				if (!addrMax || sizet(snapshot.end) > addrMax)
				{
					addrMax = sizet(snapshot.end);
				}
			}
			if (!addrMax || !addrMin)
			{
				addrMin = 0;
				addrMax = 1;
			}
			double range = static_cast<double>(addrMax - addrMin);
			if (range <= 0)
			{
				range = 1.0;
			}
			if (memoryDbg.viewRange <= 0.0)
			{
				memoryDbg.viewRange = range;
				memoryDbg.viewStart = addrMin;
			}
			double viewStart = static_cast<double>(memoryDbg.viewStart);
			double viewRange = memoryDbg.viewRange;
			double viewEnd   = viewStart + viewRange;

			// ----- Smooth pan / zoom -----
			if (memoryDbg.smoothViewRange < 0.0)
			{
				memoryDbg.smoothViewRange = viewRange;
				memoryDbg.smoothViewStart = viewStart;
			}
			if (memoryDbg.forceZoom)
			{
				memoryDbg.smoothViewStart = viewStart;
				memoryDbg.smoothViewRange = viewRange;
				memoryDbg.forceZoom       = false;
			}
			const double targetStart = double(memoryDbg.viewStart);
			const double targetRange = memoryDbg.viewRange;
			const double dt          = ImGui::GetIO().DeltaTime;
			const double alpha       = 1.0 - std::exp(-25.0 * dt);
			if (memoryDbg.isZooming)
			{
				// Couple start to range so anchor stays fixed during zoom lerp
				memoryDbg.smoothViewRange = p::Lerp(memoryDbg.smoothViewRange, targetRange, alpha);
				memoryDbg.smoothViewStart =
				    double(memoryDbg.zoomAnchorAddr)
				    - double(memoryDbg.zoomAnchorRelX) * memoryDbg.smoothViewRange;
				if (p::Abs(memoryDbg.smoothViewRange - targetRange) < 0.5)
				{
					memoryDbg.isZooming = false;
				}
			}
			else
			{
				memoryDbg.smoothViewRange = p::Lerp(memoryDbg.smoothViewRange, targetRange, alpha);
				memoryDbg.smoothViewStart = p::Lerp(memoryDbg.smoothViewStart, targetStart, alpha);
			}
			// Use smooth values for rendering
			viewStart = memoryDbg.smoothViewStart;
			viewRange = memoryDbg.smoothViewRange;
			viewEnd   = viewStart + viewRange;

			// ----- Hidden range helpers -----
			auto HiddenBytesBefore = [&](sizet a) -> double
			{
				double hidden = 0.0;
				for (const auto& h : memoryDbg.hiddenRanges)
				{
					if (h.end <= a)
					{
						hidden += static_cast<double>(h.end - h.start);
					}
					else if (h.start < a)
					{
						hidden += static_cast<double>(a - h.start);
					}
					else
					{
						break;
					}
				}
				return hidden;
			};
			double hiddenAtEnd = 0.0;
			for (const auto& h : memoryDbg.hiddenRanges)
			{
				hiddenAtEnd += static_cast<double>(h.end - h.start);
			}
			double safeEffectiveViewRange = viewRange - hiddenAtEnd;
			if (safeEffectiveViewRange <= 0.0)
			{
				safeEffectiveViewRange = 1.0;
			}

			// ----- Mapping helpers (address <-> vertical pixel) -----
			auto AddrToY = [&](sizet a) -> float
			{
				sizet aa = a;
				if (aa < static_cast<sizet>(viewStart))
				{
					aa = static_cast<sizet>(viewStart);
				}
				if (aa > static_cast<sizet>(viewStart + viewRange))
				{
					aa = static_cast<sizet>(viewStart + viewRange);
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
				return addressY0
				     + static_cast<float>(visibleOffset / safeEffectiveViewRange * addressH);
			};
			auto ScreenYToAddr = [&](float y) -> sizet
			{
				const double t = (static_cast<double>(y) - addressY0) / addressH;
				return static_cast<sizet>(viewStart + t * viewRange);
			};
			auto ArenaColumnX = [&](i32 colIndex) -> float
			{
				return colX0 + colIndex * colTotalW;
			};
			auto SnapToScale = [](double minBytes) -> double
			{
				if (minBytes <= 16.0)
				{
					return 16.0;
				}
				const double e = std::ceil(std::log2(minBytes));
				double snapped = std::pow(2.0, e);
				if (snapped < 64.0)
				{
					snapped = 64.0;
				}
				return snapped;
			};


	#pragma region Compute
			// ----- Compute -----
			const double pixelsPerByte = addressH / viewRange;
			const double majorStep     = SnapToScale(100.0 / pixelsPerByte);
			const double halfStep      = majorStep * 0.5;
			const double quarterStep   = majorStep * 0.25;
			TArray<float> majorTickYs;
			TArray<sizet> majorTickAddrs;

			// Collect all major-tick addresses first so we can find the
			// common hex prefix and gray it out
			const double firstMajor = std::ceil(viewStart / majorStep) * majorStep;
			for (double a = firstMajor; a <= viewEnd; a += majorStep)
			{
				const float ty = AddrToY(static_cast<sizet>(a));
				if (ty >= addressY0 - 1.0f && ty <= addressY1 + 1.0f)
				{
					majorTickAddrs.Add(static_cast<sizet>(a));
					majorTickYs.Add(ty);
				}
			}

			// Find the common prefix of the PRINTED hex strings across all
			// visible major ticks (not the normalized 64-bit form)
			char firstLabel[32] = "";
			if (majorTickAddrs.Size() > 0)
			{
				snprintf(firstLabel, sizeof(firstLabel), "0x%llX",
				    static_cast<unsigned long long>(majorTickAddrs[0]));
			}
			size_t commonHexChars = 0;
			if (majorTickAddrs.Size() >= 2)
			{
				const size_t firstLen = strlen(firstLabel);
				for (size_t c = 2; c < firstLen; ++c)    // skip "0x"
				{
					bool allMatch = true;
					for (i32 i = 1; i < majorTickAddrs.Size(); ++i)
					{
						char ib[32];
						snprintf(ib, sizeof(ib), "0x%llX",
						    static_cast<unsigned long long>(majorTickAddrs[i]));
						if (ib[c] != firstLabel[c])
						{
							allMatch = false;
							break;
						}
					}
					if (allMatch)
					{
						++commonHexChars;
					}
					else
					{
						break;
					}
				}
			}
			// Total length of the gray (common) portion of the label.
			const size_t commonStrLen = 2 + commonHexChars;
	#pragma endregion Compute

	#pragma region Draw
	#pragma region DrawBgs
			// ----- Backgrounds -----
			const ImU32 frameBgCol   = ImGui::GetColorU32(ImGuiCol_FrameBg);
			const ImU32 headerBgCol  = ImGui::GetColorU32(ImGuiCol_TableHeaderBg);
			const ImU32 borderStrCol = ImGui::GetColorU32(ImGuiCol_TableBorderStrong);
			const ImU32 borderLgtCol = ImGui::GetColorU32(ImGuiCol_TableBorderLight);

			drawList->AddRectFilled(canvasPos, canvasPos + canvasSize, frameBgCol);

			// Ruler
			drawList->AddRectFilled(ImVec2(canvasPos.x, canvasPos.y),
			    ImVec2(canvasPos.x + rulerW, canvasPos.y + canvasSize.y), headerBgCol);
			drawList->AddLine(ImVec2(canvasPos.x + rulerW, canvasPos.y),
			    ImVec2(canvasPos.x + rulerW, canvasPos.y + canvasSize.y), borderLgtCol, 1.0f);

			// HEX/String bars
			if (hexStripW > 0.0f)
			{
				// Left border
				drawList->AddLine(ImVec2(hexX0 + hexStripW, canvasPos.y),
				    ImVec2(hexX0 + hexStripW, canvasPos.y + canvasSize.y), borderLgtCol, 1.0f);
			}
			if (stringStripW > 0.0f)
			{
				// Right border
				drawList->AddLine(ImVec2(stringX0 + stringStripW, canvasPos.y),
				    ImVec2(stringX0 + stringStripW, canvasPos.y + canvasSize.y), borderLgtCol,
				    1.0f);
			}
	#pragma endregion DrawBgs

	#pragma region DrawValues
			{    // ---- Values ----
				// HEX and String values
				if ((hexStripW > 0.0f || stringStripW > 0.0f) && !memoryDbg.snapshots.IsEmpty())
				{
					const sizet viewLo        = static_cast<sizet>(viewStart);
					const sizet viewHi        = static_cast<sizet>(viewStart + viewRange);
					const float pixelsPerByte = static_cast<float>(addressH / viewRange);
					if (pixelsPerByte * static_cast<float>(bytesPerLine) >= 13.0f)
					{
						for (i32 a = 0; a < memoryDbg.snapshots.Size(); ++a)
						{
							const auto& snapshot = memoryDbg.snapshots[a];
							if (!snapshot.begin || snapshot.capacity == 0)
							{
								continue;
							}
							for (const auto& block : snapshot.blocks)
							{
								const sizet bs = reinterpret_cast<sizet>(block.data);
								const sizet be = bs + block.size;
								if (be <= viewLo || bs >= viewHi)
								{
									continue;
								}
								const sizet firstByte = (bs > viewLo) ? bs : viewLo;
								const sizet lastByte  = (be < viewHi) ? be : viewHi;
								if (lastByte <= firstByte)
								{
									continue;
								}
								const u8* data  = static_cast<const u8*>(block.data);
								const sizet bpl = static_cast<sizet>(bytesPerLine);
								// Global row grid anchored at viewLo
								const sizet gridOff = (firstByte - viewLo) % bpl;
								for (sizet a2 = firstByte - gridOff; a2 < lastByte; a2 += bpl)
								{
									if (a2 + bpl <= bs)
									{
										continue;
									}
									const sizet rowBegin = (a2 < bs) ? bs : a2;
									const sizet rowEnd =
									    (a2 + bpl < lastByte) ? (a2 + bpl) : lastByte;
									const float y = AddrToY(a2);
									for (sizet b2 = rowBegin; b2 < rowEnd; ++b2)
									{
										const u8 byte           = data[b2 - bs];
										const sizet lineByteIdx = b2 - a2;
										if (hexStripW > 0.0f)
										{
											char hex[3];
											snprintf(hex, sizeof(hex), "%02X", byte);
											const ImU32 hexCol =
											    (byte == 0)
											        ? p::Color{90, 90, 90, 255}.DWColor()
											        : p::Color{220, 220, 220, 255}.DWColor();
											const float xOff = static_cast<float>(lineByteIdx)
											                 * (charTextSize.x * 2.0f);
											drawList->AddText(
											    ImVec2(hexX0 + stripPad + xOff, y), hexCol, hex);
										}
										if (stringStripW > 0.0f)
										{
											const bool printable = (byte >= 32 && byte < 127);
											const char c =
											    printable ? static_cast<char>(byte) : '.';
											const ImU32 asciiCol =
											    printable ? p::Color{220, 220, 220, 255}.DWColor()
											              : p::Color{90, 90, 90, 255}.DWColor();
											const float xOff =
											    static_cast<float>(lineByteIdx) * charTextSize.x;
											drawList->AddText(ImVec2(stringX0 + stripPad + xOff, y),
											    asciiCol, &c, &c + 1);
										}
									}
								}
							}
						}
					}
				}

				// Ruler Values
				auto DrawMinorTick = [&](double a, float len, ImU32 col)
				{
					const float ty = AddrToY(static_cast<sizet>(a));
					if (ty < addressY0 - 1.0f || ty > addressY1 + 1.0f)
					{
						return;
					}
					for (i32 t = 0; t < majorTickYs.Size(); ++t)
					{
						if (p::Abs(majorTickYs[t] - ty) < 4.0f)
						{
							return;
						}
					}
					drawList->AddLine(ImVec2(canvasPos.x + rulerW, ty),
					    ImVec2(canvasPos.x + rulerW - len, ty), col, 1.5f);
				};
				const ImU32 halfCol    = p::Color{200, 200, 200, 230}.DWColor();
				const ImU32 quarterCol = p::Color{170, 170, 170, 200}.DWColor();
				for (double a = std::ceil(viewStart / halfStep) * halfStep; a <= viewEnd;
				    a += halfStep)
				{
					if (std::fmod(a, majorStep) == 0.0)
					{
						continue;
					}
					DrawMinorTick(a, 7.0f, halfCol);
				}
				for (double a = std::ceil(viewStart / quarterStep) * quarterStep; a <= viewEnd;
				    a += quarterStep)
				{
					if (std::fmod(a, halfStep) == 0.0)
					{
						continue;
					}
					DrawMinorTick(a, 4.0f, quarterCol);
				}

				// Major ticks + labels (gray common prefix, white changing suffix)
				for (i32 t = 0; t < majorTickAddrs.Size(); ++t)
				{
					const sizet addr = majorTickAddrs[t];
					const float ty   = majorTickYs[t];
					drawList->AddLine(ImVec2(canvasPos.x + rulerW, ty),
					    ImVec2(canvasPos.x + rulerW - 10.0f, ty), p::Color{220, 220, 220}.DWColor(),
					    1.5f);

					char fullBuf[32];
					snprintf(
					    fullBuf, sizeof(fullBuf), "0x%llX", static_cast<unsigned long long>(addr));
					const float fullWidth = ImGui::CalcTextSize(fullBuf).x;

					// Split label into common (gray) and changing (white).
					const size_t fullLen = strlen(fullBuf);
					const size_t splitAt = p::Min(commonStrLen, fullLen);
					char commonBuf[32]   = {};
					char changingBuf[32] = {};
					if (splitAt > 0)
					{
						memcpy(commonBuf, fullBuf, splitAt);
					}
					if (splitAt < fullLen)
					{
						memcpy(changingBuf, fullBuf + splitAt, fullLen - splitAt + 1);
					}

					// Bottom-to-top text: the FIRST char sits at the BOTTOM.
					// Common prefix at the bottom (read first), changing
					// suffix on top. Total vertical extent = fullWidth.
					const float baseY = ty + fullWidth;
					if (commonBuf[0] != '\0')
					{
						const float commonW = ImGui::CalcTextSize(commonBuf).x;
						details::AddTextVertical(drawList, ImVec2(canvasPos.x + 2.0f, baseY),
						    p::Color{110, 110, 110, 255}.DWColor(), commonBuf);
						details::AddTextVertical(drawList,
						    ImVec2(canvasPos.x + 2.0f, baseY - commonW),
						    p::Color{230, 230, 230, 255}.DWColor(), changingBuf);
					}
					else
					{
						details::AddTextVertical(drawList, ImVec2(canvasPos.x + 2.0f, baseY),
						    p::Color{230, 230, 230, 255}.DWColor(), fullBuf);
					}
				}
			}
	#pragma endregion DrawValues

	#pragma region DrawLabels
			{    // ---- Labels ----
				const ImVec2 padding = ImGui::GetStyle().FramePadding;
				// Label background
				const ImU32 bgCol = ImGui::GetColorU32(ImGuiCol_TableHeaderBg, 0.9f);
				drawList->AddRectFilled(canvasPos,
				    ImVec2(colX0, canvasPos.y + charTextSize.y + (padding.y * 2.f)), bgCol);

				// Ruler label (scale)
				const String scaleStr  = Strings::ParseMemorySize(static_cast<sizet>(majorStep));
				const ImVec2 rulerSize = ImGui::CalcTextSize(scaleStr.c_str());
				drawList->AddText(
				    ImVec2(canvasPos.x + (rulerW - rulerSize.x) * 0.5f, canvasPos.y + padding.y),
				    p::Color{220, 220, 220}.DWColor(), scaleStr.data());

				// HEX and String labels
				if (hexStripW > 0.0f || stringStripW > 0.0f)
				{
					// Strip headers (horizontal, centered) drawn at the top of the canvas
					if (hexStripW > 0.0f)
					{
						const char* hexLabel = "HEX";
						const ImVec2 hexSize = ImGui::CalcTextSize(hexLabel);
						drawList->AddText(
						    ImVec2(hexX0 + (hexStripW - hexSize.x) * 0.5f, addressY0 + padding.y),
						    p::Color{180, 180, 180}.DWColor(), hexLabel);
					}
					if (stringStripW > 0.0f)
					{
						const char* strLabel = "ASCII";
						const ImVec2 strSize = ImGui::CalcTextSize(strLabel);
						drawList->AddText(ImVec2(stringX0 + (stringStripW - strSize.x) * 0.5f,
						                      addressY0 + padding.y),
						    p::Color{180, 180, 180}.DWColor(), strLabel);
					}
				}
			}
	#pragma endregion DrawLabels
	#pragma endregion Draw

			// ----- Arena columns loop (blocks, markers, click, tooltip) -----
			for (i32 i = 0; i < memoryDbg.snapshots.Size(); ++i)
			{
				const auto& snapshot    = memoryDbg.snapshots[i];
				const p::Color baseFill = details::GetArenaColor(snapshot.typeId);
				const bool isSelected   = (i == memoryDbg.selectionArenaIdx);
				const p::Color fill     = isSelected ? (baseFill * 0.65f) : baseFill;
				const p::Color outline  = isSelected ? p::Color::Orange() : p::Color{0, 0, 0, 80};
				const ImU32 fillU32     = fill.DWColor();
				const ImU32 outlineU32  = outline.DWColor();
				const float colX        = ArenaColumnX(i);
				const float colRight    = colX + colW;

				// Column background (subtle arena tint so each column is
				// visually distinct from the window bg, similar to the
				// alternating row backgrounds in the ImGui Tree view demo).
				{
					const p::Color colBg{baseFill.r, baseFill.g, baseFill.b, 30};
					drawList->AddRectFilled(
					    ImVec2(colX, addressY0), ImVec2(colRight, addressY1), colBg.DWColor());
				}

				// Block draw + double-click focus
				if (snapshot.begin && snapshot.capacity > 0)
				{
					for (const auto& block : snapshot.blocks)
					{
						const sizet blockStart = reinterpret_cast<sizet>(block.data);
						const float y0Raw      = AddrToY(blockStart);
						const float y1Raw      = AddrToY(blockStart + block.size);
						if (y1Raw < addressY0 || y0Raw > addressY1)
						{
							continue;
						}
						float y0 = y0Raw;
						float y1 = y1Raw;
						if (y1 - y0 < 2.0f)
						{
							const float mid = (y0 + y1) * 0.5f;
							y0              = mid - 1.0f;
							y1              = mid + 1.0f;
						}
						y0 = (y0 > addressY0) ? y0 : addressY0;
						y1 = (y1 < addressY1) ? y1 : addressY1;
						drawList->AddRectFilled(ImVec2(colX, y0), ImVec2(colRight, y1), fillU32);
						drawList->AddRect(ImVec2(colX, y0), ImVec2(colRight, y1), outlineU32);
						// Double-click a block to focus it
						if (ImGui::IsMouseDoubleClicked(0))
						{
							const ImRect blockRect(ImVec2(colX, y0), ImVec2(colRight, y1));
							if (blockRect.Contains(ImGui::GetIO().MousePos))
							{
								const double blkSize = static_cast<double>(block.size);
								if (blkSize > 0.0)
								{
									const double minViewRange = p::Max(1.0,
									    addressH
									        / (24.0 * double(bytesPerLine > 0 ? bytesPerLine : 1)));
									double newViewRange(block.size);
									newViewRange        = p::Max(newViewRange, minViewRange);
									newViewRange        = p::Min(newViewRange, range);
									const double center = double(blockStart) + block.size * 0.5;
									const double newViewStart = center - newViewRange * 0.5;
									memoryDbg.viewStart       = static_cast<sizet>(newViewStart);
									memoryDbg.viewRange       = newViewRange;
									memoryDbg.smoothViewStart = newViewStart;
									memoryDbg.smoothViewRange = newViewRange;
								}
							}
						}
					}
				}

				{
					TSet<u8*> freedAllocs;

					for (auto& ev : snapshot.events)
					{
						if (ev.IsFree())
						{
							// const sizet addr = reinterpret_cast<sizet>(ev.GetPtr());
							// const float ty   = AddrToY(addr);
							// const float ty2  = AddrToY(addr + ev.GetSize());
							// if (ty < addressY0 || ty2 > addressY1)
							//{
							//	continue;
							// }

							// drawList->AddRectFilled(ImVec2(colX, ty - 0.5f),
							//     ImVec2(colRight, ty2 + 0.5f), p::Color::Red().DWColor());

							freedAllocs.Insert(ev.GetPtr());
						}
					}

					for (auto& ev : snapshot.events)
					{
						if (!ev.IsFree()
						    && !freedAllocs.Contains(ev.GetPtr()))    // Alive allocations
						{
							const sizet addr = reinterpret_cast<sizet>(ev.GetPtr());
							const float ty   = AddrToY(addr);
							const float ty2  = AddrToY(addr + ev.GetSize());
							if (ty < addressY0 || ty2 > addressY1)
							{
								continue;
							}

							// drawList->AddRectFilled(ImVec2(colX, ty - 0.5f),
							//     ImVec2(colRight, ty2 + 0.5f), p::Color::Green().DWColor());
						}
					}
				}

				// Column click selects arena
				const ImRect colRect(ImVec2(colX, addressY0), ImVec2(colRight, addressY1));
				if (colRect.Contains(ImGui::GetIO().MousePos) && ImGui::IsMouseClicked(0))
				{
					memoryDbg.hasSelection      = true;
					memoryDbg.selectionArenaIdx = i;
				}

				// Column tooltip
				if (colRect.Contains(ImGui::GetIO().MousePos))
				{
					ImGui::BeginTooltip();
					ImGui::SeparatorText("Arena");
					ImGui::Text("Name: %s (%s)", snapshot.name.Data(), snapshot.typeName.Data());
					ImGui::Text("Range: 0x%llX - 0x%llX", reinterpret_cast<sizet>(snapshot.begin),
					    reinterpret_cast<sizet>(snapshot.begin) + snapshot.capacity);
					static String sizeStr;
					if (snapshot.capacity > 0)
					{
						sizeStr = Strings::ParseMemorySize(snapshot.capacity);
						ImGui::Text("Capacity: %s (%zuB)", sizeStr.data(), snapshot.capacity);
					}
					if (snapshot.used > 0)
					{
						sizeStr = Strings::ParseMemorySize(snapshot.used);
						const float cof =
						    (snapshot.capacity > 0) ? snapshot.used / snapshot.capacity : 0;
						ImGui::Text("Used: %s (%.1f%%  %zuB)", sizeStr.c_str(), 100.0f * cof,
						    snapshot.used);
					}
					ImGui::EndTooltip();
				}

				// Column header (vertical, drawn LAST so rects/markers don't cover it)
				const char* name = snapshot.name.Data();
				bool nameIsType  = false;
				if (!name)
				{
					name       = snapshot.typeName.Data();
					nameIsType = true;
				}
				if (name)
				{
					p::Color color{220, 220, 220};
					const float nameExtent = ImGui::CalcTextSize(name).x;
					const float fontHeight = ImGui::GetTextLineHeight();
					// Center horizontally: strip extends right by ~fontHeight from pos.x
					const float stripX = colX + (colW - fontHeight) * 0.5f;
					details::AddTextVertical(drawList,
					    ImVec2(stripX, addressY0 + 4.0f + nameExtent),
					    (nameIsType ? color : color.Tint(0.5f)).DWColor(), name);
				}
			}

			// ----- Grid lines through column area (from majorTickYs) -----
			if (!majorTickYs.IsEmpty())
			{
				const float gridX0  = colX0;
				const float gridX1  = canvasPos.x + canvasSize.x;
				const ImU32 gridCol = p::Color{255, 255, 255, 20}.DWColor();
				for (i32 t = 0; t < majorTickYs.Size(); ++t)
				{
					drawList->AddLine(ImVec2(gridX0, majorTickYs[t]),
					    ImVec2(gridX1, majorTickYs[t]), gridCol, 1.0f);
				}
			}

			// ----- Hidden range zigzags (V-shape) + click to remove -----
			if (!memoryDbg.hiddenRanges.IsEmpty())
			{
				const float zigX0  = colX0;
				const float zigX1  = canvasPos.x + canvasSize.x;
				const ImU32 zigCol = p::Color{200, 120, 80, 220}.DWColor();
				for (i32 h = 0; h < memoryDbg.hiddenRanges.Size(); ++h)
				{
					const auto& hr = memoryDbg.hiddenRanges[h];
					float yStart   = AddrToY(hr.start);
					float yEnd     = AddrToY(hr.end);
					if (yEnd < addressY0 || yStart > addressY1)
					{
						continue;
					}
					yStart           = (yStart > addressY0) ? yStart : addressY0;
					yEnd             = (yEnd < addressY1) ? yEnd : addressY1;
					const float midY = (yStart + yEnd) * 0.5f;
					drawList->AddLine(ImVec2(zigX0, yStart), ImVec2(zigX1, midY), zigCol, 1.0f);
					drawList->AddLine(ImVec2(zigX0, midY), ImVec2(zigX1, yEnd), zigCol, 1.0f);
					// Click ±6px to remove
					if (ImGui::IsMouseClicked(0))
					{
						const ImVec2 mp = ImGui::GetIO().MousePos;
						if (mp.x >= zigX0 - 6.0f && mp.x <= zigX1 + 6.0f && mp.y >= yStart - 6.0f
						    && mp.y <= yEnd + 6.0f)
						{
							memoryDbg.hiddenRanges.RemoveAt(h);
							--h;
						}
					}
				}
			}

			// ----- Graph-wide tooltip (address always, block info if hit) -----
			{
				const ImVec2 mp = ImGui::GetIO().MousePos;
				if (graphRect.Contains(mp))
				{
					const sizet hoverAddr                        = ScreenYToAddr(mp.y);
					const DebugMemoryContext::ArenaSnapshot* hit = nullptr;
					for (i32 i = 0; i < memoryDbg.snapshots.Size() && !hit; ++i)
					{
						const auto& snapshot = memoryDbg.snapshots[i];
						if (!snapshot.begin || snapshot.capacity == 0)
						{
							continue;
						}
						const float cx = ArenaColumnX(i);
						if (mp.x < cx || mp.x >= cx + colW)
						{
							continue;
						}
						const sizet aStart = reinterpret_cast<sizet>(snapshot.begin);
						const sizet aEnd   = aStart + snapshot.capacity;
						if (hoverAddr >= aStart && hoverAddr < aEnd)
						{
							hit = &snapshot;
						}
					}
					ImGui::BeginTooltip();
					ImGui::Text("Address: 0x%llX", static_cast<unsigned long long>(hoverAddr));
					if (hit)
					{
						ImGui::Text("Arena: %s", GetTypeName(hit->typeId).data());
						const sizet offset = hoverAddr - reinterpret_cast<sizet>(hit->begin);
						ImGui::Text("Offset: 0x%llX (%zuB)",
						    static_cast<unsigned long long>(offset), static_cast<size_t>(offset));
						if (hit->used > 0)
						{
							static String sizeStr;
							sizeStr         = Strings::ParseMemorySize(hit->used);
							const float cof = (hit->capacity > 0) ? hit->used / hit->capacity : 0;
							ImGui::Text("Used: %s (%.1f%%  %zuB)", sizeStr.c_str(), 100.0f * cof,
							    hit->used);
						}
					}
					ImGui::EndTooltip();
				}
			}

			// ----- Selection (left-click any column, block-click sets block range, drag)
			const ImVec2 mousePos = ImGui::GetIO().MousePos;
			const bool inGraph    = graphRect.Contains(mousePos);
			{    // Selection Logic
				// Start selection on left-click
				if (ImGui::IsMouseClicked(0) && inGraph && !memoryDbg.isSelecting)
				{
					i32 arenaIdx = NO_INDEX;
					i32 blockIdx = NO_INDEX;
					for (i32 i = 0; i < memoryDbg.snapshots.Size(); ++i)
					{
						const auto& snapshot = memoryDbg.snapshots[i];
						if (!snapshot.begin || snapshot.capacity == 0)
						{
							continue;
						}
						const float cx = ArenaColumnX(i);
						if (mousePos.x < cx || mousePos.x >= cx + colW)
						{
							continue;
						}
						for (i32 e = 0; e < snapshot.blocks.Size(); ++e)
						{
							const auto& block  = snapshot.blocks[e];
							const sizet bStart = reinterpret_cast<sizet>(block.data);
							const sizet bEnd   = bStart + block.size;
							if (bEnd <= bStart)
							{
								continue;
							}
							const float y0 = AddrToY(bStart);
							const float y1 = AddrToY(bEnd);
							if (mousePos.y >= y0 && mousePos.y <= y1)
							{
								arenaIdx = i;
								blockIdx = e;
								break;
							}
						}

						if (blockIdx != NO_INDEX)
						{
							break;
						}
					}

					if (blockIdx != NO_INDEX)
					{
						memoryDbg.isSelecting    = false;
						const auto& block        = memoryDbg.snapshots[arenaIdx].blocks[blockIdx];
						memoryDbg.hasSelection   = true;
						memoryDbg.selectionStart = sizet(block.data);
						memoryDbg.selectionEnd   = memoryDbg.selectionStart + block.size;
						memoryDbg.selectionArenaIdx = arenaIdx;
						memoryDbg.selectionBlockIdx = blockIdx;
					}
					else
					{
						memoryDbg.isSelecting         = true;
						memoryDbg.selectionFirstAddr  = ScreenYToAddr(mousePos.y);
						memoryDbg.selectionSecondAddr = memoryDbg.selectionFirstAddr;
					}
				}

				const bool draggingSelection = memoryDbg.isSelecting && ImGui::IsMouseDragging(0);
				const bool finishSelection   = memoryDbg.isSelecting && ImGui::IsMouseReleased(0);
				if (draggingSelection || finishSelection)
				{
					memoryDbg.selectionSecondAddr = ScreenYToAddr(mousePos.y);
				}
				if (finishSelection)    // Dragging or single click
				{
					memoryDbg.isSelecting  = false;
					memoryDbg.hasSelection = true;
					memoryDbg.selectionStart =
					    Min(memoryDbg.selectionFirstAddr, memoryDbg.selectionSecondAddr);
					memoryDbg.selectionEnd =
					    Max(memoryDbg.selectionFirstAddr, memoryDbg.selectionSecondAddr);
					memoryDbg.selectionArenaIdx = NO_INDEX;
					memoryDbg.selectionBlockIdx = NO_INDEX;
				}
			}

			// Draw selection
			constexpr Color selectionCol(255, 200, 80);
			if (memoryDbg.isSelecting)
			{
				const float sy0 = AddrToY(memoryDbg.selectionFirstAddr);
				const float sy1 = AddrToY(memoryDbg.selectionSecondAddr);
				// Selection box
				drawList->AddLine(ImVec2(canvasPos.x, sy0), ImVec2(canvasPos.x + canvasSize.x, sy0),
				    selectionCol.Translucency(220).DWColor());
				drawList->AddLine(ImVec2(canvasPos.x, sy1), ImVec2(canvasPos.x + canvasSize.x, sy1),
				    selectionCol.Translucency(220).DWColor());
				drawList->AddRectFilled(ImVec2(canvasPos.x + rulerW, sy0),
				    ImVec2(canvasPos.x + canvasSize.x, sy1),
				    selectionCol.Translucency(90).DWColor());
			}
			if (memoryDbg.hasSelection)
			{
				if (memoryDbg.selectionEnd > memoryDbg.selectionStart)    // Is Range selection
				{
					const float sy0 = AddrToY(memoryDbg.selectionStart);
					const float sy1 = AddrToY(memoryDbg.selectionEnd);
					drawList->AddLine(ImVec2(canvasPos.x, sy0),
					    ImVec2(canvasPos.x + canvasSize.x, sy0), selectionCol.DWColor());
					drawList->AddLine(ImVec2(canvasPos.x, sy1),
					    ImVec2(canvasPos.x + canvasSize.x, sy1), selectionCol.DWColor());
					drawList->AddRectFilled(ImVec2(canvasPos.x + rulerW, sy0),
					    ImVec2(canvasPos.x + canvasSize.x, sy1),
					    selectionCol.Translucency(45).DWColor());
					if (memoryDbg.selectionArenaIdx != NO_INDEX
					    && memoryDbg.selectionBlockIdx != NO_INDEX)    // Block selection box
					{
						const float cx = ArenaColumnX(memoryDbg.selectionArenaIdx);
						drawList->AddRectFilled(ImVec2(cx, sy0), ImVec2(cx + colW, sy1),
						    selectionCol.Translucency(90).DWColor());
					}
				}
				else
				{
					const float sy0 = AddrToY(memoryDbg.selectionStart);
					// Single click: draw a bright line across the entire
					// address area so the user always sees where they
					// clicked, regardless of which strip it was in.
					drawList->AddLine(ImVec2(canvasPos.x, sy0),
					    ImVec2(canvasPos.x + canvasSize.x, sy0),
					    p::Color{255, 200, 80, 240}.DWColor(), 2.0f);
					drawList->AddRectFilled(ImVec2(canvasPos.x, sy0 - 1.5f),
					    ImVec2(canvasPos.x + canvasSize.x, sy0 + 2.5f),
					    p::Color{255, 200, 80, 70}.DWColor());
				}
			}

			// Right-click context menu
			if (inGraph && memoryDbg.hasSelection && ImGui::IsMouseClicked(1))
			{
				i32 selectedColumn = -1;
				for (i32 c = 0; c < memoryDbg.snapshots.Size(); ++c)
				{
					const float cx = ArenaColumnX(c);
					if (mousePos.x >= cx && mousePos.x < cx + colW)
					{
						selectedColumn = c;
						break;
					}
				}
				if (selectedColumn >= 0)
				{
					ImGui::OpenPopup("MemorySelectionMenu");
				}
			}


			if (ImGui::BeginPopup("MemorySelectionMenu"))
			{
				if (memoryDbg.hasSelection)
				{
					ImGui::Text("0x%llX - 0x%llX",
					    static_cast<unsigned long long>(memoryDbg.selectionStart),
					    static_cast<unsigned long long>(memoryDbg.selectionEnd));
					ImGui::Separator();
				}
				if (ImGui::MenuItem("Focus selection"))
				{
					if (memoryDbg.hasSelection
					    && memoryDbg.selectionEnd >= memoryDbg.selectionStart)
					{
						const sizet s     = memoryDbg.selectionStart;
						const sizet e     = memoryDbg.selectionEnd;
						const double size = double(e - s);
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
						memoryDbg.smoothViewStart = double(memoryDbg.viewStart);
						memoryDbg.smoothViewRange = memoryDbg.viewRange;
						memoryDbg.forceZoom       = true;
					}
				}
				if (ImGui::MenuItem("Hide range"))
				{
					if (memoryDbg.hasSelection && memoryDbg.selectionEnd > memoryDbg.selectionStart)
					{
						DebugMemoryContext::HiddenRange hr;
						hr.start = memoryDbg.selectionStart;
						hr.end   = memoryDbg.selectionEnd;
						memoryDbg.hiddenRanges.Add(hr);
						std::sort(memoryDbg.hiddenRanges.begin(), memoryDbg.hiddenRanges.end(),
						    [](const DebugMemoryContext::HiddenRange& a,
						        const DebugMemoryContext::HiddenRange& b)
						{
							return a.start < b.start;
						});
					}
				}
				if (ImGui::MenuItem("Clear selection"))
				{
					memoryDbg.hasSelection = false;
				}
				ImGui::EndPopup();
			}

			// ----- Wheel: Zoom(ctrl) + Pan -----
			{
				const ImRect wGraphRect(canvasPos, canvasPos + canvasSize);
				const double wAddressY0 = addressY0;
				const double wAddressH  = addressH;
				const double wViewStart = memoryDbg.smoothViewStart;
				const double wViewRange = memoryDbg.smoothViewRange;
				const i32 wBytesPerLine = (memoryDbg.bytesPerLine > 0) ? memoryDbg.bytesPerLine : 1;
				if (wGraphRect.Contains(ImGui::GetIO().MousePos))
				{
					const float wheel = ImGui::GetIO().MouseWheel;
					if (wheel != 0.0f)
					{
						if (ImGui::GetIO().KeyCtrl)    // Zoom
						{
							const float my = ImGui::GetIO().MousePos.y;
							double addrAtCursor;
							if (my <= wAddressY0)
							{
								addrAtCursor = wViewStart;
							}
							else if (my >= wAddressY0 + wAddressH)
							{
								addrAtCursor = wViewStart + wViewRange;
							}
							else
							{
								addrAtCursor = wViewStart
								             + ((static_cast<double>(my) - wAddressY0) / wAddressH)
								                   * wViewRange;
							}
							const double minViewRange = p::Max(
							    1.0, wAddressH / (24.0 * static_cast<double>(wBytesPerLine)));
							double newViewRange =
							    (wheel > 0) ? (wViewRange / 1.15) : (wViewRange * 1.15);
							if (newViewRange < minViewRange)
							{
								newViewRange = minViewRange;
							}
							if (newViewRange > range)
							{
								newViewRange = range;
							}
							const double zoomEpsilon = 0.5;
							const bool atMinZoom     = (newViewRange <= minViewRange + zoomEpsilon);
							const bool atMaxZoom     = (newViewRange >= range - zoomEpsilon);
							if (atMinZoom || atMaxZoom)
							{
								memoryDbg.viewRange       = atMinZoom ? minViewRange : range;
								memoryDbg.isZooming       = false;
								memoryDbg.smoothViewRange = memoryDbg.viewRange;
								memoryDbg.smoothViewStart =
								    static_cast<double>(memoryDbg.viewStart);
							}
							else
							{
								// Zoom coupling: anchor stays at cursor
								const double relX =
								    (wAddressH > 0.0)
								        ? ((static_cast<double>(my) - wAddressY0) / wAddressH)
								        : 0.0;
								const double newStart     = addrAtCursor - relX * newViewRange;
								memoryDbg.viewStart       = static_cast<sizet>(newStart);
								memoryDbg.viewRange       = newViewRange;
								memoryDbg.smoothViewStart = newStart;
								memoryDbg.smoothViewRange = newViewRange;
								memoryDbg.zoomAnchorAddr  = static_cast<sizet>(addrAtCursor);
								memoryDbg.zoomAnchorRelX  = static_cast<float>(relX);
								memoryDbg.isZooming       = true;
							}
						}
						else    // Pan
						{
							// Pan: wheel up → higher addresses
							const double addrDelta =
							    -static_cast<double>(wheel) * wViewRange * 0.15;
							memoryDbg.viewStart = static_cast<sizet>(wViewStart + addrDelta);
						}
					}
				}
			}

			// ----- Middle-mouse pan (inverted/grab model) -----
			{
				const ImRect dGraphRect(canvasPos, canvasPos + canvasSize);
				if (dGraphRect.Contains(ImGui::GetIO().MousePos) && ImGui::IsMouseDragging(2))
				{
					memoryDbg.isZooming = false;
					const float dy      = ImGui::GetIO().MouseDelta.y;
					if (dy != 0.0f)
					{
						const double addrDelta =
						    -static_cast<double>(dy) / addressH * memoryDbg.smoothViewRange;
						memoryDbg.viewStart =
						    static_cast<sizet>(memoryDbg.smoothViewStart + addrDelta);
						memoryDbg.smoothViewStart = static_cast<double>(memoryDbg.viewStart);
					}
				}
			}
		}
		ImGui::End();    // View

		// Details window (docked to the right of the View)
		if (memoryDbg.showDetails)
		{
			if (memoryDbg.detailsDockId != 0)
			{
				ImGui::SetNextWindowDockID(memoryDbg.detailsDockId, ImGuiCond_FirstUseEver);
			}
			if (ImGui::Begin("Details"))
			{
				static String detailsLabel;
				static String sizeStr;
				if (selectedArena)
				{
					detailsLabel = GetTypeName(selectedArena->typeId);
					ImGui::Text("Type: %s", detailsLabel.c_str());
					ImGui::Text("Range: 0x%llX - 0x%llX",
					    reinterpret_cast<sizet>(selectedArena->begin),
					    reinterpret_cast<sizet>(selectedArena->begin) + selectedArena->capacity);
					ImGui::SeparatorText("Usage");
					if (selectedArena->capacity > 0)
					{
						sizeStr = Strings::ParseMemorySize(selectedArena->capacity);
						ImGui::Text("Capacity: %s (%zuB)", sizeStr.data(), selectedArena->capacity);
					}
					if (selectedArena->used > 0)
					{
						const float usedPct = (selectedArena->capacity > 0)
						                        ? selectedArena->used / selectedArena->capacity
						                        : 0;
						sizeStr             = Strings::ParseMemorySize(selectedArena->used);
						ImGui::Text("Used: %s (%.1f%%)", sizeStr.c_str(), 100.f * usedPct);
						ImGui::ProgressBar(usedPct / 100.0f);
					}
					detailsLabel.clear();
					Strings::FormatTo(detailsLabel, "{} blocks", selectedArena->blocks.Size());
					ImGui::SeparatorText(detailsLabel.data());
					for (i32 i = 0; i < selectedArena->blocks.Size(); ++i)
					{
						const auto& block = selectedArena->blocks[i];
						char blockLabel[128];
						sizeStr = Strings::ParseMemorySize(block.size);
						snprintf(blockLabel, sizeof(blockLabel), "Block %i: 0x%llX | %s (%zuB)", i,
						    reinterpret_cast<sizet>(block.data), sizeStr.data(), block.size);
						ImGui::BulletText("%s", blockLabel);
					}
					ImGui::Separator();
					if (ImGui::Button("Deselect"))
					{
						memoryDbg.hasSelection      = false;
						memoryDbg.selectionArenaIdx = NO_INDEX;
						memoryDbg.selectionBlockIdx = NO_INDEX;
					}
				}
				else
				{
					ImGui::TextDisabled("No arena selected.");
					ImGui::TextDisabled("Click a column or a block in the graph.");
				}
				ImGui::End();
			}
		}

		ImGui::End();    // Parent window (closes the ImGuiWindowFlags_MenuBar window)
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
