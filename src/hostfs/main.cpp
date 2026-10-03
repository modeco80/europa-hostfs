#include <utils/log.hpp>
#include <utils/hook/fnhook.hpp>
#include <utils/hook/classhook.hpp>
#include "europa/lecal.h"
#include "filemanager.hpp"
#include <europa/stdstring.h>
#include <ml/string.h>

extern "C" {
	extern bool ps2CdCatalogLoaded;
	void ps2LoadCdCatalog();

	bool ps2FileExists(const europaStdString& pathName);

}

bool cdStreamInitHooks();
bool consoleInitHooks();

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


CLASS_HOOK(u64, LECAudio, RPCCall, u32 cmd, void* param, void* param2, void* param3, void* param4, void* param5, void* param6, void* param7, void* param8) {
	if((cmd & 0x8350) == 0x8350) {
		const u32 ulChannel = (cmd & ~0x8350);
		// If this command is a valid OPEN_STREAM_DIRECT_FAST command,
		// then replace the OPEN_STREAM_DIRECT_FAST command with OPEN_STREAM_FAST.
		if(ulChannel < 2) {
			const u32 newCommand = ulChannel | 0x8340;
			utilLogf(LogInfo, "DEBUG! old command was 0x%04x, new command is 0x%04x", cmd, newCommand);

			// The arguments are in the correct order for OPEN_STREAM_DIRECT_FAST
			// Arguments for OPEN_STREAM_FAST
			// param is filename (thanks to fileexists/query hooks)
			// param2 is ulStreamPlayId
			// param3 is ulCombinedVolume,
			// param4 is ulLoopCount
			// param5 is ulStartit
			return hook_LECAudio_RPCCall.original(klass, newCommand, param, param3, param4, param5, param6, vnil, vnil, vnil);
		}
	}

	// Otherwise pass through and call the original function
	return hook_LECAudio_RPCCall.original(klass, cmd, param, param2, param3, param4, param5, param6, param7, param8);
}

extern "C" void StreamMgr_addLecalStreamPath(const char* psz) {
	// Translate the path to a hostfs path using the FileMan API to do so.
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

	if(!consoleInitHooks()) {
		utilLogf(LogErr, "Failed to hook console functions.");
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


	if(!hook_LECAudio_RPCCall.hook()) {
		utilLogf(LogErr, "Failed to hook LECAudio::RPCCall");
		return 1;
	}

	return 0;
}
