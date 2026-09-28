#include <utils/log.hpp>

extern "C" int modMain() {
	utilLogf(LogInfo, "Europa HostFS patch (Game: %s). 2026 modeco80",
#ifdef GAME_starfighter
	"Starfighter"
#endif
	);
	return 0;
}
