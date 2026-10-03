#include <ml/mem.h>
#include <ml/string.h>
#include <sce/cdvd.h>

#include <europa/stdstring.h>

#include <utils/hook/fnhook.hpp>
#include <utils/log.hpp>

#include "file.hpp"
#include "filemanager.hpp"

// These hooks define a "fake" version of the sceCdSt* API functions,
// which are fully compatible in place with the original APIs, but
// use our FileMan_*/File APIs, which are hostfs.

//#define CDST_DEBUG

// Fake version of the sceCdlFILE struct we return.
struct sceCdFile {
	const char* pszName;
	u32 size;

	char name[16];
	u8 date[8];
	u32 flag;
};

char gszCurrentCdStreamFileName[64];
File* gpCurrentCdStreamFile = nil(File*);


extern "C" int ps2QueryCdCatalog(const europaStdString& path, u32* lba, u32* size);

// evil. Does NOT need to be fast.
bool stringStartsWith(const char* __restrict haystack, const char* __restrict needle) {
	const u32 haystackLength = strlen(haystack);
	const u32 needleLength = strlen(needle);

	// can't completely store the needle, so don't bother
	if(haystackLength < needleLength)
		return false;

	// Though we could unroll these to word at a time, I suppose.
	for(u32 i = 0; i < needleLength; ++i)
		if(haystack[i] != needle[i])
			return false;

	return true;
}

void stringCopyLower(char* __restrict dst, const char* __restrict src) {
	const u32 len = strlen(src);
	for(u32 i = 0; i < len; ++i) {
		if(src[i] >= 'A' && src[i] <= 'Z') {
			dst[i] = src[i] | 0x20;
		} else {
			dst[i] = src[i];
		}
	}
	dst[len] = '\0';
}

FUNC_REPLACE(ps2QueryCdCatalog, int, const europaStdString& path, const char** filename, u32* size) {
	utilLogf(LogInfo, "ps2QueryCdCatalog called path: %s", path.cStr());
#if 1
	File* pHostFile = FileMan_openFile(path.cStr());
	if(pHostFile == nil(File*))
		return 1;

	const char* pszPath = path.cStr();

	// This is a bit of a hack. Basically, the LECAL IOP module expects
	// just the final file name, and it will try and open all it's known stream
	// directories. So, we need to strip the path out.
	if(!stringStartsWith(pszPath, "STREAMS\\VIDEO")) {
		utilLogf(LogInfo, "Path is a LECAL stream path. Engaging evil hack");

		// Strip the path out.
		const char* pszEnd = pszPath + strlen(pszPath);
		while(*pszEnd != '\\')
			pszEnd--;
		pszPath = pszEnd + 1;
		stringCopyLower(&gszCurrentCdStreamFileName[0], pszPath);
	} else {
		// bleh, it works.
		memcpy(&gszCurrentCdStreamFileName[0], pszPath, strlen(pszPath) + 1);
	}

	// return fake values
	*filename = &gszCurrentCdStreamFileName[0];
	*size = pHostFile->getSize();
	FileMan_closeFile(pHostFile);
	return 0;
#endif
}

FUNC_REPLACE(sceCdStInit, i32, u32 maxBuffers, u32 maxBanks, u32 iopBuffer) {
#ifdef CDST_DEBUG
	utilLog(LogInfo, "sceCdStInit() called");
#endif
	return 1;
}

FUNC_REPLACE(sceCdStStart, i32, const char* pszFileName, void* rmode) {
#ifdef CDST_DEBUG
	utilLogf(LogInfo, "sceCdStStart() active, filename is %s", pszFileName);
#endif

	// Open the CD stream file.
	gpCurrentCdStreamFile = FileMan_openFile(pszFileName);
	// This shouldn't fail, but ToCToU safety is a good thing.
	if(gpCurrentCdStreamFile == nil(File*))
		return 0;

	return 1;
}

FUNC_REPLACE(sceCdStRead, i32, u32 sectorCount, u32* buf, u32 mode, u32* err) {
	if(gpCurrentCdStreamFile == nil(File*)) {
		// no stream active
#ifdef CDST_DEBUG
		utilLogf(LogInfo, "sceCdStRead(): No active stream");
#endif
		return -1;
	}

	if(gpCurrentCdStreamFile->eof()) {
#ifdef CDST_DEBUG
		utilLogf(LogInfo, "sceCdStRead(): Stream has ended");
#endif
		return -1;
	}

	i32 nRead = gpCurrentCdStreamFile->read(reinterpret_cast<u8*>(&buf[0]), sectorCount * 0x800);
	// Return the amount of sectors actually read
	return nRead / 0x800;
}

FUNC_REPLACE0(sceCdStStop, i32) {
#ifdef CDST_DEBUG
	utilLogf(LogInfo, "sceCdStStop()");
#endif
	if(gpCurrentCdStreamFile) {
		FileMan_closeFile(gpCurrentCdStreamFile);
		gpCurrentCdStreamFile = nil(File*);
	}
	return 1;
}

bool cdStreamInitHooks() {
#define DO_HOOK_FUNC(fnName)                             \
	if(!hook_##fnName.hook()) {                          \
		utilLogf(LogErr, "Failed to hook %s.", #fnName); \
		return false;                                    \
	}

	//DO_HOOK_FUNC(sceCdSearchFile);

	DO_HOOK_FUNC(ps2QueryCdCatalog);

	DO_HOOK_FUNC(sceCdStInit);
	DO_HOOK_FUNC(sceCdStStart);
	DO_HOOK_FUNC(sceCdStRead);
	DO_HOOK_FUNC(sceCdStStop);
	return true;
}
