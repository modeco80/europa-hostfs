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
	// TODO: Instead of performing an open()/close(), we should be able to,
	// if sceStat() is inside of the binary, simply call that, which should be
	// a bit less awful. It's not too bad, because file objects *are* pre-allocated,
	// but bleh.
	const char* pszFileName = pathName.cStr();
	File* pfile = FileMan_openFile(pszFileName);
	if(pfile == nil(File*)) {
		return false;
	}
	FileMan_closeFile(pfile);
	return true;
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
