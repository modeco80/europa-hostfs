#include <utils/log.hpp>
#include <utils/hook/fnhook.hpp>

extern "C" {
	extern bool ps2CdCatalogLoaded;
	void ps2LoadCdCatalog();
}

FUNC_REPLACE0(ps2LoadCdCatalog, void) {
	utilLogf(LogInfo, "ps2LoadCdCatalog called?");
	ps2CdCatalogLoaded = false;

	utilLogf(LogInfo, "ps2LoadCdCatalog returning");
	return;
}

extern "C" int modMain() {
	utilLogf(LogInfo, "Europa HostFS patch (Game: %s). 2026 modeco80",
#ifdef GAME_starfighter
	"Starfighter"
#endif
	);


	if(!hook_ps2LoadCdCatalog.hook()) {
		utilLogf(LogErr, "Failed to hook ps2LoadCdCatalog");
	}
	return 0;
}
