#ifndef EUROPA_STORAGE_H
#define EUROPA_STORAGE_H

#include <ml/types.h>
#include "europa/stdstring.h"

// Europa is written with the Metrowerks CodeWarrior compiler.
// While most ABI guarantees are covered, virtual functions
// are *not*; Metrowerks uses its own implementation of virtual
// functions and RTTI which is ABI incompatible with the GNU GCC compiler.
// ... Fun.

struct CStorage;

extern "C" {
extern void* CPS2DiskStore_RTTI;

i32 CStorage_ReadByte(CStorage* self, u8&);
i32 CStorage_ReadShort(CStorage* self, i16&);
i32 CStorage_ReadInt(CStorage* self, i32&);

i32 CStorage_ReadString(CStorage* self, europaStdString*);
i32 CStorage_ReadLine(CStorage* self, europaStdString* pLine, char lineFeed, bool clear);

i32 CStorage_Copy(CStorage* self, CStorage* pSrc, u32 len);
i32 CStorage_CopyAll(CStorage* self, CStorage* pSrc);

void CStorage_WriteValue(CStorage*);
void CStorage_WriteLine(CStorage*);
void CStorage_WriteFormattedLine(CStorage*);
void CStorage_WriteFormattedString(CStorage*);
void CStorage_WriteFormattedText(CStorage*);

i32 CStorage_GetChecksum(CStorage* self, u32 startVal);

bool CStorage_IsAtEnd(CStorage* self);

}

struct CStorage_vtable {
	void* mpRTTI;
	u32 pad;

	void(*Destructor)(CStorage* self);
	i32(*Open)(CStorage* self);
	void(*Close)(CStorage* self);

	i32(*SeekToBeginning)(CStorage* self);
	i32(*Seek)(CStorage* self, i32 offset);
	i32(*SeekToEnd)(CStorage* self);

	i32(*Read)(CStorage* self, void* pvBuf, const u32& count);
	i32(*ReadByte)(CStorage* self, u8& byte);
	i32(*ReadShort)(CStorage* self, i16&);
	i32(*ReadInt)(CStorage* self, i32&);

	// doesn't need overrides

	i32(*ReadString)(CStorage* self, europaStdString*);
	i32(*ReadLine)(CStorage* self, europaStdString* pLine, char lineFeed, bool clear);

	i32(*Resize)(CStorage* self, i32 newSize);
	i32(*Write)(CStorage* self, u32 count, const void* pvBuf);

	i32(*Copy)(CStorage* self, CStorage* pSrc, u32 len);
	i32(*CopyAll)(CStorage* self, CStorage* pSrc);

	void(*WriteValue)(CStorage* self);
	void(*WriteLine)(CStorage* self);
	void(*WriteFormattedLine)(CStorage* self);
	void(*WriteFormattedString)(CStorage* self);
	void(*WriteFormattedText)(CStorage* self);

	// overrides

	void*(*GetShortcutBuffer)(CStorage* self);
	i32(*GetStorageHints)(CStorage* self);

	// dont override
	i32(*GetChecksum)(CStorage* self, u32 startVal);

	// override
	i32(*GetSize)(CStorage* self);
	i32(*GetCurrPos)(CStorage* self);
	bool(*IsOpen)(CStorage* self);

	// don't override
	bool(*IsAtEnd)(CStorage* self);
};

struct CStorage {
	const CStorage_vtable* vtable;
};

struct CDiskStore : CStorage {
	europaStdString path;
	i32 flags;
	i32 unk2;
};

// note: CPS2Disk/CDStore ctor is
// self, europaStdString* filename, i32 flags.

#endif
