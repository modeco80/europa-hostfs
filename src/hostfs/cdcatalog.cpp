#include <utils/log.hpp>
#include <utils/hook/fnhook.hpp>
#include <utils/hook/classhook.hpp>
#include "filemanager.hpp"
#include <europa/stdstring.h>
#include <ml/string.h>

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
	//utilLogf(LogInfo, "ps2FileExists(%s)", pathName.cStr());
	File* pfile = FileMan_openFile(pathName.cStr());
	if(pfile != nil(File*)) {
		FileMan_closeFile(pfile);
		return true;
	}
	return false;
}

bool cdCatalogInitHooks() {
#define DO_HOOK_FUNC(fnName)                             \
	if(!hook_##fnName.hook()) {                          \
		utilLogf(LogErr, "Failed to hook %s.", #fnName); \
		return false;                                    \
	}

	DO_HOOK_FUNC(ps2LoadCdCatalog);
	DO_HOOK_FUNC(ps2FileExists);
	return true;
}
