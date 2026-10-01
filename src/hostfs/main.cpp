#include <utils/log.hpp>
#include <utils/hook/fnhook.hpp>
#include "filemanager.hpp"
#include <europa/stdstring.h>

extern "C" {
	extern bool ps2CdCatalogLoaded;
	void ps2LoadCdCatalog();

	bool ps2FileExists(const europaStdString& pathName);
}

FUNC_REPLACE0(ps2LoadCdCatalog, void) {
	utilLogf(LogInfo, "ps2LoadCdCatalog called?");
	ps2CdCatalogLoaded = true;

	utilLogf(LogInfo, "ps2LoadCdCatalog returning");
	return;
}

FUNC_REPLACE(ps2FileExists, bool, const europaStdString& pathName) {
	File* pfile = FileMan_openFile(pathName.cStr());
	if(pfile != nil(File*)) {
		FileMan_closeFile(pfile);
		return true;
	}
	return false;
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


	if(!hook_ps2FileExists.hook()) {
		utilLogf(LogErr, "Failed to hook ps2FileExists");
	}
	return 0;
}
