#include <europa/storage.h>
#include <europa/stdstring.h>

#include "file.hpp"
#include "filemanager.hpp"
#include <utils/log.hpp>
#include <ml/cxx/string.hpp>
#include <new>

// This file contains a reimplementation of the CStorage interface which
// uses the FileMan* APIs to do file I/O (which means hostfs/path cleanup, et cetera).
//
// This implementation strategy was chosen for a couple reasons:
// - CPS2DiskStore makes a very large allocation per instance of over 0x20000 bytes
// 	which is probably not great. This readahead/window buffer sizing is fine for optical media,
// 	but for faster storage mediums it is rather dumb.
// - Centralizing I/O throughout using FileMan/File APIs. Paths are cleaned up too,
//	which should hopefully help with casing issues.

//#define DISKSTORE_DEBUG

#ifdef DISKSTORE_DEBUG
	#define diskstoreDebugLog(level, msg) utilLog(level, msg)
	#define diskstoreDebugLogf(level, fmt, ...) utilLogf(level, fmt, __VA_ARGS__)
#else
	#define diskstoreDebugLog(level, msg)
	#define diskstoreDebugLogf(level, fmt, ...)
#endif

struct CFileManDiskStore : CStorage {
	i32 flags;
	ml::String filePath;
	File* pFile;
};

// CFileManDiskStore can't be above 48 bytes in size.
ML_STATIC_ASSERT(sizeof(CFileManDiskStore) <= 0x30);

extern "C" {

void CFileManDiskStore_Destruct(CFileManDiskStore* self);
i32 CFileManDiskStore_Open(CFileManDiskStore* self);
void CFileManDiskStore_Close(CFileManDiskStore* self);
i32 CFileManDiskStore_SeekToBeginning(CFileManDiskStore* self);
i32 CFileManDiskStore_Seek(CFileManDiskStore* self, i32 pos);
i32 CFileManDiskStore_SeekToEnd(CFileManDiskStore* self);
i32 CFileManDiskStore_Read(CFileManDiskStore* self, void* pvBuf, const u32& count);

i32 CFileManDiskStore_ReadString(CFileManDiskStore* self, europaStdString* pString);
i32 CFileManDiskStore_ReadLine(CFileManDiskStore* self, europaStdString* pLine, char lineFeed, bool clear);

i32 CFileManDiskStore_Resize(CFileManDiskStore* self, i32 newSize);
i32 CFileManDiskStore_Write(CFileManDiskStore* self, u32 count, void* pvBuf);

void* CFileManDiskStore_GetShortcutBuffer(CFileManDiskStore* self);

i32 CFileManDiskStore_GetStorageHints(CFileManDiskStore* self);

i32 CFileManDiskStore_GetSize(CFileManDiskStore* self);
i32 CFileManDiskStore_GetCurrPos(CFileManDiskStore* self);
bool CFileManDiskStore_IsOpen(CFileManDiskStore* self);
bool CFileManDiskStore_IsAtEnd(CFileManDiskStore* self);

const static CStorage_vtable CFileManDiskStore_vtable = {
	&CPS2DiskStore_RTTI,
	0,

	// Virtual function pointers
	reinterpret_cast<void(*)(CStorage*)>(CFileManDiskStore_Destruct),
	reinterpret_cast<i32(*)(CStorage*)>(CFileManDiskStore_Open),
	reinterpret_cast<void(*)(CStorage*)>(CFileManDiskStore_Close),
	reinterpret_cast<i32(*)(CStorage*)>(CFileManDiskStore_SeekToBeginning),
	reinterpret_cast<i32(*)(CStorage*, i32)>(CFileManDiskStore_Seek),
	reinterpret_cast<i32(*)(CStorage*)>(CFileManDiskStore_SeekToEnd),

	reinterpret_cast<i32(*)(CStorage*, void*, const u32&)>(CFileManDiskStore_Read),
	CStorage_ReadByte,
	CStorage_ReadShort,
	CStorage_ReadInt,

	reinterpret_cast<i32(*)(CStorage*, europaStdString*)>(CFileManDiskStore_ReadString),
	reinterpret_cast<i32(*)(CStorage*, europaStdString*, char, bool)>(CFileManDiskStore_ReadLine),

	reinterpret_cast<i32(*)(CStorage*, i32)>(CFileManDiskStore_Resize),
	reinterpret_cast<i32(*)(CStorage*, u32, const void*)>(CFileManDiskStore_Write),

	CStorage_Copy,
	CStorage_CopyAll,

	reinterpret_cast<void(*)(CStorage*)>(CStorage_WriteValue),
	reinterpret_cast<void(*)(CStorage*)>(CStorage_WriteLine),
	reinterpret_cast<void(*)(CStorage*)>(CStorage_WriteFormattedLine),
	reinterpret_cast<void(*)(CStorage*)>(CStorage_WriteFormattedString),
	reinterpret_cast<void(*)(CStorage*)>(CStorage_WriteFormattedText),

	reinterpret_cast<void*(*)(CStorage*)>(CFileManDiskStore_GetShortcutBuffer),
	reinterpret_cast<i32(*)(CStorage*)>(CFileManDiskStore_GetStorageHints),
	CStorage_GetChecksum,

	reinterpret_cast<i32(*)(CStorage*)>(CFileManDiskStore_GetSize),
	reinterpret_cast<i32(*)(CStorage*)>(CFileManDiskStore_GetCurrPos),
	reinterpret_cast<bool(*)(CStorage*)>(CFileManDiskStore_IsOpen),
	reinterpret_cast<bool(*)(CStorage*)>(CFileManDiskStore_IsAtEnd)
};


void CFileManDiskStore_Construct(CFileManDiskStore* self, europaStdString* pString, i32 flags) {
	diskstoreDebugLogf(LogInfo, "CFileManDiskStore constructor(0x%08x) called.", self);

	// Bless life of our object
	new (self) CFileManDiskStore;

	self->vtable = &CFileManDiskStore_vtable;
	self->flags = flags;
	self->filePath.assign(pString->cStr());
	self->pFile = nil(File*);
}

void CFileManDiskStore_Destruct(CFileManDiskStore* self) {
	diskstoreDebugLogf(LogInfo, "CFileManDiskStore destructor(%08x) called.", self);

	// Close the file if the user didn't.
	if(CFileManDiskStore_IsOpen(self)) {
		CFileManDiskStore_Close(self);
	}

	self->~CFileManDiskStore();
}

i32 CFileManDiskStore_Open(CFileManDiskStore* self) {
	diskstoreDebugLog(LogInfo, "CFileManDiskStore::Open() called.");
	self->pFile = FileMan_openFile(self->filePath.cStr());
	if(self->pFile == nil(File*))
		return 0x66; // File couldn't be opened
	return 0;
}

void CFileManDiskStore_Close(CFileManDiskStore* self) {
	diskstoreDebugLog(LogInfo, "CFileManDiskStore::Close() called.");

	// Close the file.
	if(self->pFile != nil(File*)) {
		FileMan_closeFile(self->pFile);
		self->pFile = nil(File*);
	}
}

i32 CFileManDiskStore_SeekToBeginning(CFileManDiskStore* self) {
	if(!CFileManDiskStore_IsOpen(self))
		return 1001;
	self->pFile->seek(0, File::SeekStart);
	return 0;
}

i32 CFileManDiskStore_Seek(CFileManDiskStore* self, i32 pos) {
	if(!CFileManDiskStore_IsOpen(self))
		return 1001;

	//utilLogf(LogInfo, "CFileManDiskStore::Seek(0x%08x, pos: %d) called", self, pos);

	if(self->pFile->seek(pos, File::SeekStart) == -1) {
		diskstoreDebugLog(LogInfo, "File::seek failed(in CFileManDiskStore::Seek)");
		return 1001;
	}

	return 0;
}

i32 CFileManDiskStore_SeekToEnd(CFileManDiskStore* self) {
	if(!CFileManDiskStore_IsOpen(self))
		return 1001;
	self->pFile->seek(0, File::SeekEnd);
	return 0;
}

i32 CFileManDiskStore_Read(CFileManDiskStore* self, void* pvBuf, const u32& count) {
	if(!CFileManDiskStore_IsOpen(self)) {
		diskstoreDebugLog(LogErr, "CFileManDiskStore::Read() called on an unopened CFileManDiskStore instance???");
		return 1001;
	}

	diskstoreDebugLogf(LogInfo, "CFileManDiskStore::Read(0x%08x, count: %d) called.", self, count);

	i32 countRead = self->pFile->read(reinterpret_cast<u8*>(pvBuf), count);

	if(countRead != count)
		diskstoreDebugLogf(LogInfo, "File::read returned %d bytes read, user wanted %d", countRead, count);

	if(countRead != count)
		return 1;

	return 0;
}

// compiler was mean and devirtualized these calls :(
i32 CFileManDiskStore_ReadString(CFileManDiskStore* self, europaStdString* pString) {
	diskstoreDebugLogf(LogInfo, "CFileManDiskStore::ReadString(%08x) called", self);
	i32 retVal = CStorage_ReadString(self, pString);
	diskstoreDebugLogf(LogInfo, "read: %s", pString->cStr());
	return retVal;
}

i32 CFileManDiskStore_ReadLine(CFileManDiskStore* self, europaStdString* pLine, char lineFeed, bool clear) {
	diskstoreDebugLogf(LogInfo, "CFileManDiskStore::ReadLine(%08x) called", self);
	i32 retVal = CStorage_ReadLine(self, pLine, lineFeed, clear);
	diskstoreDebugLogf(LogInfo, "read: %s", pLine->cStr());
	return retVal;
}

i32 CFileManDiskStore_Resize(CFileManDiskStore* self, i32 newSize) {
	// Not supported.
	return 1002;
}

i32 CFileManDiskStore_Write(CFileManDiskStore* self, u32 count, void* pvBuf) {
	return 1002;
}

void* CFileManDiskStore_GetShortcutBuffer(CFileManDiskStore* self) {
	return vnil;
}

i32 CFileManDiskStore_GetStorageHints(CFileManDiskStore* self) {
	return 0; // no hints Lol!
}

i32 CFileManDiskStore_GetSize(CFileManDiskStore* self) {
	if(!CFileManDiskStore_IsOpen(self))
		return 0;
	return self->pFile->getSize();
}

i32 CFileManDiskStore_GetCurrPos(CFileManDiskStore* self) {
	if(!CFileManDiskStore_IsOpen(self))
		return 0;

	i32 pos = self->pFile->tell();

	diskstoreDebugLogf(LogInfo, "CFileManDiskStore::GetCurrPos(0x%08x) called, pos: %d", self, pos);
	return pos;
}

bool CFileManDiskStore_IsOpen(CFileManDiskStore* self) {
	return self->pFile != nil(File*);
}

bool CFileManDiskStore_IsAtEnd(CFileManDiskStore* self) {
	if(!CFileManDiskStore_IsOpen(self))
		return true;
	//utilLogf(LogInfo, "CFileManDiskStore::IsAtEnd(0x%08x) called, at end: %s", self->pFile->eof() ? "yes" : "no");
	return self->pFile->eof();
}

}
