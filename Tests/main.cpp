// Copyright 2015-2026 Piperift. All Rights Reserved.

// NOTE: PipeNewDelete is deliberately not included here. PipeTests provides the
// replacement operator new/delete (P_OVERRIDE_NEWDELETE) in its own translation unit;
// including it here too would cause duplicate-definition linker errors.

#include <PipeTests.h>
#include <Pipe.h>
#include <PipeMemoryArenas.h>

#include <cstdlib>


// namespace backward
//{
//	backward::SignalHandling sh;
// }    // namespace backward


// Forward declarations
void RegisterTimeTests();
void RegisterCoreFunctionTests();
void RegisterCoreOwnPtrTests();
void RegisterCorePageBufferTests();
void RegisterCorePlatformProcessTests();
void RegisterCoreSetTests();
void RegisterCoreSpinLockTests();
void RegisterCoreStringTests();
void RegisterCoreStringViewTests();
void RegisterCoreTagTests();
void RegisterContainersArraysTests();
void RegisterECSComponentsTests();
void RegisterECSECSsmTests();
void RegisterECSFilteringTests();
void RegisterECSHierarchyTests();
void RegisterECSIdRegistryTests();
void RegisterECSIdScopesTests();
void RegisterECSStaticsTests();
void RegisterFilesPathsTests();
void RegisterMathColorTests();
void RegisterMathMathTests();
void RegisterMathVectorTests();
void RegisterMemoryBestFitArenaTests();
void RegisterMemoryBigBestFitArenaTests();
void RegisterMemoryMemoryTests();
void RegisterMemoryMemoryStatsTests();
void RegisterMemoryMonoLinearArenaTests();
void RegisterReflectionMacroReflectionTests();
void RegisterReflectionObjectTests();
void RegisterReflectionTraitsTests();
void RegisterReflectionTypeIdTests();
void RegisterReflectionTypeNameTests();
void RegisterSerializationBinaryTests();
void RegisterSerializationJsonTests();
void RegisterSerializationSerializationTests();


int main(int argc, char* argv[])
{
	p::Initialize();
	// Suppress leak messages from the global HeapArena (used internally by
	// many subsystems; not all of them free every allocation during tests).
	p::GetHeapArena().GetStats()->detectLeaks = false;

	RegisterTimeTests();
	RegisterCoreFunctionTests();
	RegisterCoreOwnPtrTests();
	RegisterCorePageBufferTests();
	RegisterCorePlatformProcessTests();
	RegisterCoreSetTests();
	RegisterCoreSpinLockTests();
	RegisterCoreStringTests();
	RegisterCoreStringViewTests();
	RegisterCoreTagTests();
	RegisterContainersArraysTests();
	RegisterECSComponentsTests();
	RegisterECSECSsmTests();
	RegisterECSFilteringTests();
	RegisterECSHierarchyTests();
	RegisterECSIdRegistryTests();
	RegisterECSIdScopesTests();
	RegisterECSStaticsTests();
	RegisterFilesPathsTests();
	RegisterMathColorTests();
	RegisterMathMathTests();
	RegisterMathVectorTests();
	RegisterMemoryBestFitArenaTests();
	RegisterMemoryBigBestFitArenaTests();
	RegisterMemoryMemoryTests();
	RegisterMemoryMemoryStatsTests();
	RegisterMemoryMonoLinearArenaTests();
	RegisterReflectionMacroReflectionTests();
	RegisterReflectionObjectTests();
	RegisterReflectionTraitsTests();
	RegisterReflectionTypeIdTests();
	RegisterReflectionTypeNameTests();
	RegisterSerializationBinaryTests();
	RegisterSerializationJsonTests();
	RegisterSerializationSerializationTests();

	int result = p::RunTests(argc, argv);
	p::Shutdown();
	return result;
}
