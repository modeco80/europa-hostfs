#include <utils/log.hpp>
#include <utils/hook/fnhook.hpp>
#include <utils/hook/classhook.hpp>
#include "europa/lecal.h"
#include "filemanager.hpp"
#include <europa/stdstring.h>
#include <ml/string.h>

#define LECAL_DEBUG

#ifdef LECAL_DEBUG
	#define lecalDebugLog(level, msg) utilLog(level, msg)
	#define lecalDebugLogf(level, fmt, ...) utilLogf(level, fmt, __VA_ARGS__)
#else
	#define lecalDebugLog(level, msg)
	#define lecalDebugLogf(level, fmt, ...)
#endif

CLASS_HOOK(u64, LECAudio, RPCCall, u32 cmd, void* param, void* param2, void* param3, void* param4, void* param5, void* param6, void* param7, void* param8) {
	if((cmd & LECAL_OPEN_STREAM_DIRECT_FAST) == LECAL_OPEN_STREAM_DIRECT_FAST) {
		const u32 ulChannel = (cmd & ~LECAL_OPEN_STREAM_DIRECT_FAST);
		// If this command is a valid OPEN_STREAM_DIRECT_FAST command,
		// then replace the OPEN_STREAM_DIRECT_FAST command with OPEN_STREAM_FAST.
		if(ulChannel < 2) {
			const u32 newCommand = ulChannel | LECAL_OPEN_STREAM_FAST;
			lecalDebugLogf(LogInfo, "[LECAL] LECAL_OPEN_STREAM_DIRECT_FAST command replacement: Old command was 0x%04x, new command is 0x%04x", cmd, newCommand);

			// The arguments are in the correct order for OPEN_STREAM_DIRECT_FAST,
			// but we need to re-order them for OPEN_STREAM_FAST.
			// Arguments for OPEN_STREAM_FAST
			// param is filename (thanks to fileexists/query hooks)
			// param2 is ulStreamPlayId
			// param3 is ulCombinedVolume,
			// param4 is ulLoopCount
			// param5 is ulStartit
			return hook_LECAudio_RPCCall.original(klass, newCommand, param, param3, param4, param5, param6, vnil, vnil, vnil);
		}
	}

	// Otherwise pass through the original RPC call.
	return hook_LECAudio_RPCCall.original(klass, cmd, param, param2, param3, param4, param5, param6, param7, param8);
}

extern "C" void StreamMgr_addLecalStreamPath(const char* psz) {
	// Translate the path to a hostfs path using the FileMan API to do so.
	char translatedPath[512];

	// Skip $ in europacd.reg paths.
	if(*psz == '$')
		psz++;

	FileMan_translateFileName(&translatedPath[0], psz);
	strcat(&translatedPath[0], "/"); // hack

	utilLogf(LogInfo, "[LECAL] Adding stream path %s", translatedPath);
	g_lecAudio.AddStreamPath(translatedPath);
}

bool lecalInitHooks() {
	if(!hook_LECAudio_RPCCall.hook()) {
		utilLogf(LogErr, "[LECAL] Failed to hook LECAudio::RPCCall.");
		return false;
	}

	return true;
}
