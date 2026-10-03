#ifndef EUROPA_LECAL_H
#define EUROPA_LECAL_H

#include <ml/types.h>

class LECAudio {
public:
	void RPCCall(u32 cmd, void* param, void* param2, void* param3, void* param4, void* param5, void* param6, void* param7, void* param8);

	inline void AddStreamPath(const char* pszPath) {
		RPCCall(0x80d0, (void*)pszPath, vnil, vnil, vnil, vnil, vnil, vnil, vnil);
	}
};

extern LECAudio g_lecAudio;

#endif
