#include "filemanager.hpp"
#include "filemanager_config.hpp"
#include "file.hpp"

#include <ml/abort.h>
#include <ml/mem.h>
#include <sce/fio.h>

#include <ml/cxx/freelist.hpp>
#include <new>

#include <utils/log.hpp>

namespace {
	ml::FreeList<File, FILEMAN_MAX_FILES> openFileList;
} // namespace

void FileMan_translateFileName(char* pszOut, const char* pszInPath) {
	// ?? what are you expecting?
	if(pszInPath == nil(char*) || *pszInPath == '\0')
		return;

	mlStaticStrCpy(pszOut, "host0:");
	strcat(pszOut, pszInPath);

	const u32 len = strlen(pszOut);
	for(u32 i = mlStaticStrLen("host0:"); i < len; ++i) {
		// Clean up the case of file paths so it is entirely lowercase.
		if(pszOut[i] >= 'A' && pszOut[i] <= 'Z')
			pszOut[i] |= 0x20;

		// Fix up the objectively incorrect directory seperator to the correct one.
		if(pszOut[i] == '\\')
			pszOut[i] = '/';
	}

	// Make doubly sure the output string is null-terminated, even if
	// strcat() was supposed to do that for us. Defensive programming is
	// better than buggy code, after all :)
	pszOut[len] = '\0';
}

File* FileMan_openFile(const char* path) {
	File* pFile = openFileList.allocate();
	if(pFile == nil(File*)) {
		utilLog(LogErr, "[FileMan] We somehow exhausted all file handles??? Increase FILEMAN_MAX_FILES");
		return nil(File*);
	}

	char translatedPath[512];
	FileMan_translateFileName(&translatedPath[0], path);

	i32 fd = sceOpen(translatedPath, SCE_RDONLY);
	if(fd < 0) {
#if defined(FILEMAN_DEBUG) && defined(FILEMAN_DEBUG_OPEN)
		utilLogf(LogErr, "[FileMan] HostFS Open FAIL %s", translatedPath);
#endif
		openFileList.free(pFile);
		return nil(File*);
	}

#if defined(FILEMAN_DEBUG) && defined(FILEMAN_DEBUG_OPEN)
	utilLogf(LogInfo, "[FileMan] HostFS Open %s -> File* %08x", translatedPath, pFile);
#endif

	// ml freelists don't new objects, so we have to do it ourselves.
	return new(pFile) File(fd);
}

void FileMan_closeFile(File* pFile) {
#if defined(FILEMAN_DEBUG) && defined(FILEMAN_DEBUG_OPEN)
	utilLogf(LogInfo, "[FileMan] HostFS Close File* %08x", pFile);
#endif
	pFile->~File();
	openFileList.free(pFile);
}
