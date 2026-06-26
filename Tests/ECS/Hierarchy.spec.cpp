// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "bandit/grammar.h"

#include <bandit/bandit.h>
#include <PipeECS.h>


using namespace snowhouse;
using namespace bandit;
using namespace p;

namespace snowhouse
{
	template<>
	struct Stringizer<Id>
	{
		static std::string ToString(Id id)
		{
			std::stringstream stream;
			stream << "Id(" << id.value << ")";
			return stream.str();
		}
	};
}    // namespace snowhouse


go_bandit([]()
{
	describe("ECS.Hierarchy", []()
	{
		IdContext ctx;
		Id root;
		Id child1;
		Id child2;
		Id child3;
		Id grandchild;

		before_each([&]()
		{
			ctx        = {};
			root       = AddId(ctx);
			child1     = AddId(ctx);
			child2     = AddId(ctx);
			child3     = AddId(ctx);
			grandchild = AddId(ctx);
		});

		describe("AttachId", [&]()
		{
			it("Creates bidirectional parent-child link for single child", [&]()
			{
				AttachId({ctx}, root, child1);

				AssertThat(ctx.Has<CParent>(root), Is().True());
				AssertThat(ctx.Has<CChild>(child1), Is().True());
				AssertThat(ctx.Get<CParent>(root).children.Size(), Equals(1));
				AssertThat(ctx.Get<CParent>(root).children[0], Equals(child1));
				AssertThat(ctx.Get<CChild>(child1).parent, Equals(root));
			});

			it("Appends multiple children to same parent", [&]()
			{
				AttachId({ctx}, root, {child1, child2, child3});

				AssertThat(ctx.Has<CParent>(root), Is().True());
				AssertThat(ctx.Has<CChild>(child1), Is().True());
				AssertThat(ctx.Has<CChild>(child2), Is().True());
				AssertThat(ctx.Has<CChild>(child3), Is().True());
				AssertThat(ctx.Get<CParent>(root).children.Size(), Equals(3));
				AssertThat(ctx.Get<CParent>(root).children[0], Equals(child1));
				AssertThat(ctx.Get<CParent>(root).children[1], Equals(child2));
				AssertThat(ctx.Get<CParent>(root).children[2], Equals(child3));
				AssertThat(ctx.Get<CChild>(child1).parent, Equals(root));
				AssertThat(ctx.Get<CChild>(child2).parent, Equals(root));
				AssertThat(ctx.Get<CChild>(child3).parent, Equals(root));
			});
		});

		describe("AttachIdAfter", [&]()
		{
			it("Inserts child after specified sibling preserving order", [&]()
			{
				AttachId({ctx}, root, {child1, child3});
				AttachIdAfter({ctx}, root, child2, child1);

				AssertThat(ctx.Get<CParent>(root).children.Size(), Equals(3));
				AssertThat(ctx.Get<CParent>(root).children.FindIndex(child2), Equals(1));
			});
		});

		describe("TransferIdChildren", [&]()
		{
			it("Moves children from old parent to new parent", [&]()
			{
				Id newRoot = AddId(ctx);
				AttachId({ctx}, root, {child1, child2});
				TransferIdChildren({ctx}, {child1, child2}, newRoot);

				AssertThat(ctx.Get<CParent>(root).children.IsEmpty(), Is().True());
				AssertThat(ctx.Has<CParent>(newRoot), Is().True());
				AssertThat(ctx.Get<CParent>(newRoot).children.Size(), Equals(2));
				AssertThat(ctx.Get<CChild>(child1).parent, Equals(newRoot));
				AssertThat(ctx.Get<CChild>(child2).parent, Equals(newRoot));
			});
		});

		describe("DetachIdParent", [&]()
		{
			before_each([&]()
			{
				AttachId({ctx}, root, {child1, child2});
			});

			it("Retains CChild component when keepComponents is true", [&]()
			{
				DetachIdParent({ctx}, child1, true);

				AssertThat(ctx.Has<CChild>(child1), Is().True());
				AssertThat(ctx.Get<CChild>(child1).parent, Equals(NoId));
				AssertThat(ctx.Get<CParent>(root).children.Size(), Equals(1));
			});

			it("Removes CChild from detached child and removes from parent list", [&]()
			{
				DetachIdParent({ctx}, child1, false);

				AssertThat(ctx.Has<CChild>(child1), Is().False());
				AssertThat(ctx.Get<CParent>(root).children.Contains(child1), Is().False());
			});

			it("Removes empty CParent when all children are detached", [&]()
			{
				DetachIdParent({ctx}, {child1, child2}, false);

				AssertThat(ctx.Has<CChild>(child1), Is().False());
				AssertThat(ctx.Has<CChild>(child2), Is().False());
				AssertThat(ctx.Has<CParent>(root), Is().False());
			});
		});

		describe("DetachIdChildren", [&]()
		{
			before_each([&]()
			{
				AttachId({ctx}, root, {child1, child2});
			});

			it("Severes all children but retains CChild when keepComponents is true", [&]()
			{
				DetachIdChildren({ctx}, root, true);

				AssertThat(ctx.Has<CChild>(child1), Is().True());
				AssertThat(ctx.Has<CChild>(child2), Is().True());
				AssertThat(ctx.Get<CChild>(child1).parent, Equals(NoId));
				AssertThat(ctx.Get<CChild>(child2).parent, Equals(NoId));
				AssertThat(ctx.Get<CParent>(root).children.IsEmpty(), Is().True());
			});

			it("Removes CChild and CParent when keepComponents is false", [&]()
			{
				DetachIdChildren({ctx}, root, false);

				AssertThat(ctx.Has<CChild>(child1), Is().False());
				AssertThat(ctx.Has<CChild>(child2), Is().False());
				AssertThat(ctx.Has<CParent>(root), Is().False());
			});
		});

		describe("GetIdChildren", [&]()
		{
			before_each([&]()
			{
				AttachId({ctx}, root, {child1, child2});
				AttachId({ctx}, child1, grandchild);
			});

			it("Returns child list for parent entities", [&]()
			{
				const auto* children = GetIdChildren({ctx}, root);
				AssertThat(children, !Equals(nullptr));
				AssertThat(children->Size(), Equals(2));
				AssertThat(children->Contains(child1), Is().True());
				AssertThat(children->Contains(child2), Is().True());
			});

			it("Combines children from multiple parents into one list", [&]()
			{
				TArray<Id> outChildren;
				GetIdChildren({ctx}, {root, child1}, outChildren);
				AssertThat(outChildren.Size(), Equals(3));
				AssertThat(outChildren.Contains(grandchild), Is().True());
			});

			it("Returns null for entities without CParent component", [&]()
			{
				AssertThat(GetIdChildren({ctx}, child2), Equals(nullptr));
			});
		});

		describe("GetAllIdChildren", [&]()
		{
			before_each([&]()
			{
				AttachId({ctx}, root, child1);
				AttachId({ctx}, child1, grandchild);
			});

			it("Recurses full tree depth to collect all descendents", [&]()
			{
				TArray<Id> outChildren;
				GetAllIdChildren({ctx}, root, outChildren, 10);
				AssertThat(outChildren.Size(), Equals(2));
				AssertThat(outChildren.Contains(grandchild), Is().True());
			});

			it("Respects depth limit to return only immediate children", [&]()
			{
				TArray<Id> outChildren;
				GetAllIdChildren({ctx}, root, outChildren, 1);
				AssertThat(outChildren.Size(), Equals(1));
				AssertThat(outChildren.Contains(grandchild), Is().False());
			});
		});

		describe("GetIdParent", [&]()
		{
			before_each([&]()
			{
				AttachId({ctx}, root, child1);
				AttachId({ctx}, child1, grandchild);
			});

			it("Returns parent Id for child entities", [&]()
			{
				AssertThat(GetIdParent({ctx}, child1), Equals(root));
				AssertThat(GetIdParent({ctx}, grandchild), Equals(child1));
			});

			it("Returns unique parents for multiple children", [&]()
			{
				TArray<Id> outParents;
				GetIdParent({ctx}, {child1, grandchild}, outParents);
				AssertThat(outParents.Size(), Equals(2));
				AssertThat(outParents.Contains(root), Is().True());
				AssertThat(outParents.Contains(child1), Is().True());
			});

			it("Returns NoId for root entities without parent", [&]()
			{
				AssertThat(GetIdParent({ctx}, root), Equals(NoId));
			});

			it("Returns NoId for entities without CChild component", [&]()
			{
				AssertThat(GetIdParent({ctx}, child2), Equals(NoId));
			});
		});

		describe("GetAllIdParents", [&]()
		{
			before_each([&]()
			{
				AttachId({ctx}, root, child1);
				AttachId({ctx}, child1, grandchild);
			});

			it("Traverses full ancestry chain from leaf to root", [&]()
			{
				TArray<Id> outParents;
				GetAllIdParents({ctx}, grandchild, outParents);
				AssertThat(outParents.Size(), Equals(2));
				AssertThat(outParents[0], Equals(child1));
				AssertThat(outParents[1], Equals(root));
			});

			it("Returns empty when entity has no CChild component", [&]()
			{
				TArray<Id> outParents;
				GetAllIdParents({ctx}, child2, outParents);
				AssertThat(outParents.IsEmpty(), Is().True());
			});
		});

		describe("FindIdParent", [&]()
		{
			before_each([&]()
			{
				AttachId({ctx}, root, child1);
				AttachId({ctx}, child1, grandchild);
			});

			it("Finds ancestor two levels up matching predicate", [&]()
			{
				AssertThat(FindIdParent({ctx}, grandchild,
				               [&](Id id)
				{
					return id == root;
				}),
				    Equals(root));
			});

			it("Finds immediate parent matching predicate", [&]()
			{
				AssertThat(FindIdParent({ctx}, grandchild,
				               [&](Id id)
				{
					return id == child1;
				}),
				    Equals(child1));
			});

			it("Returns NoId when no ancestor matches predicate", [&]()
			{
				AssertThat(IsNone(FindIdParent({ctx}, grandchild,
				               [](Id)
				{
					return false;
				})),
				    Is().True());
			});
		});

		describe("FindIdParents", [&]()
		{
			it("Finds nearest matching ancestor for deep entity", [&]()
			{
				Id intermediate = AddId(ctx);
				AttachId({ctx}, root, child1);
				AttachId({ctx}, child1, intermediate);
				AttachId({ctx}, intermediate, grandchild);

				TArray<Id> outParents;
				FindIdParents({ctx}, grandchild, outParents, [](Id)
				{
					return true;
				});
				AssertThat(outParents.Size(), Equals(1));
				AssertThat(outParents.Contains(intermediate), Is().True());
			});

			it("Returns empty when no ancestor matches predicate", [&]()
			{
				TArray<Id> outParents;
				FindIdParents({ctx}, child1, outParents, [](Id)
				{
					return false;
				});
				AssertThat(outParents.IsEmpty(), Is().True());
			});
		});

		describe("GetIdRoots", [&]()
		{
			it("Returns empty when no hierarchy exists", [&]()
			{
				TArray<Id> roots;
				GetIdRoots({ctx}, roots);
				AssertThat(roots.IsEmpty(), Is().True());
			});

			it("Finds root of single-parent hierarchy", [&]()
			{
				AttachId({ctx}, root, {child1, child2});

				TArray<Id> roots;
				GetIdRoots({ctx}, roots);
				AssertThat(roots.Size(), Equals(1));
				AssertThat(roots.Contains(root), Is().True());
			});

			it("Returns multiple roots from independent trees", [&]()
			{
				Id root2 = AddId(ctx);
				AttachId({ctx}, root, {child1, child2});
				AttachId({ctx}, root2, child3);

				TArray<Id> roots;
				GetIdRoots({ctx}, roots);
				AssertThat(roots.Size(), Equals(2));
				AssertThat(roots.Contains(root), Is().True());
				AssertThat(roots.Contains(root2), Is().True());
			});

			it("Excludes entities that are both parent and child of someone", [&]()
			{
				AttachId({ctx}, root, child1);
				AttachId({ctx}, child1, grandchild);

				TArray<Id> roots;
				GetIdRoots({ctx}, roots);
				AssertThat(roots.Size(), Equals(1));
				AssertThat(roots.Contains(root), Is().True());
				AssertThat(roots.Contains(child1), Is().False());
			});
		});

		describe("GetIdParentRoots", [&]()
		{
			before_each([&]()
			{
				AttachId({ctx}, root, child1);
				AttachId({ctx}, child1, grandchild);
			});

			it("Walks child chain up to root ancestor", [&]()
			{
				TArray<Id> roots;
				GetIdParentRoots({ctx}, grandchild, roots, false);
				AssertThat(roots.Size(), Equals(1));
				AssertThat(roots.Contains(root), Is().True());
			});

			it("Handles children from different trees", [&]()
			{
				Id root2    = AddId(ctx);
				Id childOf2 = AddId(ctx);
				AttachId({ctx}, root2, childOf2);

				TArray<Id> roots;
				GetIdParentRoots({ctx}, {grandchild, childOf2}, roots, false);
				AssertThat(roots.Size(), Equals(2));
				AssertThat(roots.Contains(root), Is().True());
				AssertThat(roots.Contains(root2), Is().True());
			});

			it("Considers input entities as roots when considerChildren flag is set", [&]()
			{
				TArray<Id> roots;
				GetIdParentRoots({ctx}, {root, grandchild}, roots, true);
				AssertThat(roots.Size(), Equals(1));
				AssertThat(roots.Contains(root), Is().True());
			});

			it("Returns empty for empty input", [&]()
			{
				TArray<Id> roots;
				GetIdParentRoots({ctx}, {}, roots, false);
				AssertThat(roots.IsEmpty(), Is().True());
			});

			it("Returns empty for entities with no parent", [&]()
			{
				TArray<Id> roots;
				GetIdParentRoots({ctx}, child2, roots, false);
				AssertThat(roots.IsEmpty(), Is().True());
			});
		});

		describe("FixParentIdLinks", [&]()
		{
			before_each([&]()
			{
				AttachId({ctx}, root, child1);
			});

			it("Returns false when parent-child links are already correct", [&]()
			{
				AssertThat(FixParentIdLinks({ctx}, root), Is().False());
			});

			it("Fixes child->parent reference when it does not match parent's list", [&]()
			{
				ctx.Get<CChild>(child1).parent = NoId;

				AssertThat(FixParentIdLinks({ctx}, root), Is().True());
				AssertThat(ctx.Get<CChild>(child1).parent, Equals(root));
			});

			it("Adds missing CChild component to orphan children", [&]()
			{
				ctx.Remove<CChild>(child1);
				AssertThat(ctx.Has<CChild>(child1), Is().False());

				AssertThat(FixParentIdLinks({ctx}, root), Is().True());
				AssertThat(ctx.Has<CChild>(child1), Is().True());
				AssertThat(ctx.Get<CChild>(child1).parent, Equals(root));
			});
		});

		describe("ValidateParentIdLinks", [&]()
		{
			before_each([&]()
			{
				AttachId({ctx}, root, child1);
			});

			it("Returns true when all parent-child links are consistent", [&]()
			{
				AssertThat(ValidateParentIdLinks({ctx}, root), Is().True());
			});

			it("Returns false when child->parent reference is mismatched", [&]()
			{
				ctx.Get<CChild>(child1).parent = NoId;

				AssertThat(ValidateParentIdLinks({ctx}, root), Is().False());
			});

			it("Returns false when CChild component is missing from child", [&]()
			{
				ctx.Remove<CChild>(child1);

				AssertThat(ValidateParentIdLinks({ctx}, root), Is().False());
			});
		});
	});
});
