#include <utils/log.hpp>
#include <utils/hook/fnhook.hpp>
#include "europa/lecal.h"
#include "filemanager.hpp"
#include <europa/stdstring.h>
#include <ml/string.h>

extern "C" {
	extern bool ps2CdCatalogLoaded;
	void ps2LoadCdCatalog();

	bool ps2FileExists(const europaStdString& pathName);


	void LECOutputConsoleImpl(int level, const char* psz, __builtin_va_list val);
	void OutputConsoleAlso(const europaStdString& str, int level);
}

bool cdStreamInitHooks();

FUNC_REPLACE0(ps2LoadCdCatalog, void) {
	utilLogf(LogInfo, "ps2LoadCdCatalog called?");
	ps2CdCatalogLoaded = true;

	utilLogf(LogInfo, "ps2LoadCdCatalog returning");
	return;
}

FUNC_REPLACE(ps2FileExists, bool, const europaStdString& pathName) {
	utilLogf(LogInfo, "ps2FileExists(%s)", pathName.cStr());
	File* pfile = FileMan_openFile(pathName.cStr());
	if(pfile != nil(File*)) {
		FileMan_closeFile(pfile);
		return true;
	}
	return false;
}

// Utility hook; this allows getting output from LECOutputConsole().
FUNC_HOOK(LECOutputConsoleImpl, void, int level, const char* psz, __builtin_va_list val) {
	char buf[1024];
	vsprintf(&buf[0], psz, val);
	utilLogf(LogInfo, "[Europa console] %s", buf);
	return hook_LECOutputConsoleImpl.original(level, psz, val);
}

FUNC_HOOK(OutputConsoleAlso, void, const europaStdString& str, int level) {
	utilLogf(LogInfo, "[Europa console] %s", str.cStr());
	return hook_OutputConsoleAlso.original(str,level);
}


extern "C" void StreamMgr_addLecalStreamPath(const char* psz) {
	char translatedPath[512];
	FileMan_translateFileName(&translatedPath[0], psz);
	strcat(&translatedPath[0], "/"); // hack

	utilLogf(LogInfo, "Adding LECAUDIO.IRX stream path %s", translatedPath);
	g_lecAudio.RPCCall(0x80d0, (void*)&translatedPath[0], vnil, vnil, vnil, vnil, vnil, vnil, vnil);
}


extern "C" int modMain() {
	utilLogf(LogInfo, "Europa HostFS patch (Game: %s). 2026 modeco80",
#ifdef GAME_starfighter
	"Starfighter"
#endif
	);

	// Hook debug console stuff so we can see it better

	if(!hook_LECOutputConsoleImpl.hook()) {
		utilLogf(LogErr, "Failed to hook LECOutputConsoleImpl");
		return 1;
	}

	if(!hook_OutputConsoleAlso.hook()) {
		utilLogf(LogErr, "Failed to hook OutputConsoleAlso");
		return 1;
	}

	if(!cdStreamInitHooks()) {
		utilLogf(LogErr, "Failed to hook Cd stream stuff");
		return 1;
	}

	if(!hook_ps2LoadCdCatalog.hook()) {
		utilLogf(LogErr, "Failed to hook ps2LoadCdCatalog");
		return 1;
	}


	if(!hook_ps2FileExists.hook()) {
		utilLogf(LogErr, "Failed to hook ps2FileExists");
		return 1;
	}

	return 0;
}
