
#include "Pipe/Export.h"
#include "Pipe/Files/STDFileSystem.h"


namespace p
{
	PIPE_API void Initialize(Path logPath = {});
	PIPE_API void Shutdown();
};    // namespace p
