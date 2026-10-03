#include <utils/hook/fnhook.hpp>
#include <europa/stdstring.h>
#include <utils/log.hpp>
#include <ml/string.h>

extern "C" {
	void LECOutputConsoleImpl(int level, const char* psz, __builtin_va_list val);
	void OutputConsoleAlso(const europaStdString& str, int level);
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

bool consoleInitHooks() {
#define DO_HOOK_FUNC(fnName)                             \
	if(!hook_##fnName.hook()) {                          \
		utilLogf(LogErr, "Failed to hook %s.", #fnName); \
		return false;                                    \
	}

	DO_HOOK_FUNC(LECOutputConsoleImpl)
	DO_HOOK_FUNC(OutputConsoleAlso)
	return true;
}
