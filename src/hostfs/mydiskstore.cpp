#include <europa/storage.h>
#include <europa/stdstring.h>

#include "file.hpp"
#include "filemanager.hpp"
#include "utils/log.hpp"
#include <ml/cxx/string.hpp>
#include <new>

// Note that we can't be above 0x30 bytes. Not that I think that will be a problem
// but hey.

struct CMyDiskStore : CDiskStore {
	ml::String actualPath;
	File* file;
};

extern "C" {

void CMyDiskStore_Destruct(CMyDiskStore* self);
i32 CMyDiskStore_Open(CMyDiskStore* self);
void CMyDiskStore_Close(CMyDiskStore* self);
i32 CMyDiskStore_SeekToBeginning(CMyDiskStore* self);
i32 CMyDiskStore_Seek(CMyDiskStore* self, i32 pos);
i32 CMyDiskStore_SeekToEnd(CMyDiskStore* self);
i32 CMyDiskStore_Read(CMyDiskStore* self, void* pvBuf, const u32& count);

i32 CMyDiskStore_Resize(CMyDiskStore* self, i32 newSize);
i32 CMyDiskStore_Write(CMyDiskStore* self, u32 count, void* pvBuf);

void* CMyDiskStore_GetShortcutBuffer(CMyDiskStore* self);

i32 CMyDiskStore_GetStorageHints(CMyDiskStore* self);

i32 CMyDiskStore_GetSize(CMyDiskStore* self);
i32 CMyDiskStore_GetCurrPos(CMyDiskStore* self);
bool CMyDiskStore_IsOpen(CMyDiskStore* self);

const static CStorage_vtable CMyDiskStore_vtable = {
	&CPS2DiskStore_RTTI,
	0,

	// Virtual function pointers
	reinterpret_cast<void(*)(CStorage*)>(CMyDiskStore_Destruct),
	reinterpret_cast<i32(*)(CStorage*)>(CMyDiskStore_Open),
	reinterpret_cast<void(*)(CStorage*)>(CMyDiskStore_Close),
	reinterpret_cast<i32(*)(CStorage*)>(CMyDiskStore_SeekToBeginning),
	reinterpret_cast<i32(*)(CStorage*, i32)>(CMyDiskStore_Seek),
	reinterpret_cast<i32(*)(CStorage*)>(CMyDiskStore_SeekToEnd),

	reinterpret_cast<i32(*)(CStorage*, void*, const u32&)>(CMyDiskStore_Read),
	CStorage_ReadByte,
	CStorage_ReadShort,
	CStorage_ReadInt,

	// for people nice enough to use the vtable.
	CStorage_ReadString,
	CStorage_ReadLine,

	reinterpret_cast<i32(*)(CStorage*, i32)>(CMyDiskStore_Resize),
	reinterpret_cast<i32(*)(CStorage*, u32, const void*)>(CMyDiskStore_Write),

	CStorage_Copy,
	CStorage_CopyAll,

	reinterpret_cast<void(*)(CStorage*)>(CStorage_WriteValue),
	reinterpret_cast<void(*)(CStorage*)>(CStorage_WriteLine),
	reinterpret_cast<void(*)(CStorage*)>(CStorage_WriteFormattedLine),
	reinterpret_cast<void(*)(CStorage*)>(CStorage_WriteFormattedString),
	reinterpret_cast<void(*)(CStorage*)>(CStorage_WriteFormattedText),

	reinterpret_cast<void*(*)(CStorage*)>(CMyDiskStore_GetShortcutBuffer),
	reinterpret_cast<i32(*)(CStorage*)>(CMyDiskStore_GetStorageHints),
	CStorage_GetChecksum,

	reinterpret_cast<i32(*)(CStorage*)>(CMyDiskStore_GetSize),
	reinterpret_cast<i32(*)(CStorage*)>(CMyDiskStore_GetCurrPos),
	reinterpret_cast<bool(*)(CStorage*)>(CMyDiskStore_IsOpen),
	CStorage_IsAtEnd
};


void CMyDiskStore_Construct(CMyDiskStore* self, europaStdString* pString, i32 flags) {
	new (self) CMyDiskStore;
	self->vtable = &CMyDiskStore_vtable;

	self->flags = flags;
	self->actualPath.assign(pString->cStr());
	self->file = nil(File*);
}

void CMyDiskStore_Destruct(CMyDiskStore* self) {
	if(self->file) {
		CMyDiskStore_Close(self);
	}

	self->~CMyDiskStore();
}

i32 CMyDiskStore_Open(CMyDiskStore* self) {
	utilLogf(LogInfo, "CMyDiskStore::Open() called. %s\n", self->actualPath.cStr());
	self->file = FileMan_openFile(self->actualPath.cStr());
	if(self->file == nil(File*))
		return 0x66; // File couldn't be opened
	return 0;
}

void CMyDiskStore_Close(CMyDiskStore* self) {
	if(self->file) {
		FileMan_closeFile(self->file);
		self->file = nil(File*);
	}
}

i32 CMyDiskStore_SeekToBeginning(CMyDiskStore* self) {
	if(self->file == nil(File*))
		return 1001;
	self->file->seek(0, File::SeekStart);
	return 0;
}

i32 CMyDiskStore_Seek(CMyDiskStore* self, i32 pos) {
	if(self->file == nil(File*))
		return 1001;
	if(self->file->seek(pos, File::SeekStart) == -1)
		return 1001;
	return 0;
}

i32 CMyDiskStore_SeekToEnd(CMyDiskStore* self) {
	if(self->file == nil(File*))
		return 1001;
	self->file->seek(0, File::SeekEnd);
	return 0;
}

i32 CMyDiskStore_Read(CMyDiskStore* self, void* pvBuf, const u32& count) {
	if(self->file == nil(File*))
		return 1001;

	i32 countRead = self->file->read(reinterpret_cast<u8*>(pvBuf), count);
	return count == countRead;
}

// compiler was mean and devirtualized these calls :(
i32 CMyDiskStore_ReadString(CMyDiskStore* self, europaStdString* pString) {
	return CStorage_ReadString(self, pString);
}

i32 CMyDiskStore_ReadLine(CMyDiskStore* self, europaStdString* pLine, char lineFeed, bool clear) {
	return CStorage_ReadLine(self, pLine, lineFeed, clear);
}

i32 CMyDiskStore_Resize(CMyDiskStore* self, i32 newSize) {
	// Not supported.
	return 1002;
}

i32 CMyDiskStore_Write(CMyDiskStore* self, u32 count, void* pvBuf) {
	return 1002;
}

void* CMyDiskStore_GetShortcutBuffer(CMyDiskStore* self) {
	return vnil;
}


i32 CMyDiskStore_GetStorageHints(CMyDiskStore* self) {
	return 0; // no hints Lol!
}

i32 CMyDiskStore_GetSize(CMyDiskStore* self) {
	if(self->file == nil(File*))
		return 0;
	return self->file->getSize();
}

i32 CMyDiskStore_GetCurrPos(CMyDiskStore* self) {
	if(self->file == nil(File*))
		return 0;
	return self->file->tell();
}

bool CMyDiskStore_IsOpen(CMyDiskStore* self) {
	return self->file != nil(File*);
}

}
