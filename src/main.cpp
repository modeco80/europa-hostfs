#include <utils/log.hpp>

bool consoleInitHooks();
bool cdStreamInitHooks();
bool cdCatalogInitHooks();
bool lecalInitHooks();

extern "C" int modMain() {
	utilLogf(LogInfo, "Europa HostFS patch (Game: %s). 2026 modeco80",
#ifdef GAME_starfighter
	"Starfighter"
#endif
	);

	if(!consoleInitHooks()) {
		utilLogf(LogErr, "Failed to hook console functions.");
		return 1;
	}

	if(!cdStreamInitHooks()) {
		utilLogf(LogErr, "Failed to hook Cd stream stuff");
		return 1;
	}

	if(!cdCatalogInitHooks()) {
		utilLogf(LogErr, "Failed to hook CD catalog functions.");
		return 1;
	}

	if(!lecalInitHooks()) {
		utilLogf(LogErr, "Failed to hook LECAL functions.");
		return 1;
	}

	return 0;
}
