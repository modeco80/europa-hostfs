#!/bin/env python3

import struct
import sys
import os

sys.path.append(os.path.abspath(os.path.dirname(__file__)))

# modules we depend on
from utils.mips import Mips
from utils.elf import ElfWrapper
from utils.ldparse import LdScript

import lief
import pyxdelta

ELF32_EHDR_FMT = "<16sHHIIIIIHHHHHH"
ELF32_PHDR_FMT = "<IIIIIIII"

ELF32_EHDR_SIZE = struct.calcsize(ELF32_EHDR_FMT)
ELF32_PHDR_SIZE = struct.calcsize(ELF32_PHDR_FMT)

PT_LOAD = 1

PF_X = 1
PF_W = 2
PF_R = 4

def alignUp(value: int, align: int) -> int:
	return (value + align - 1) & ~(align - 1)

def patchInjectCore(original: bytes, patchCore: bytes, patchVirtualLoad: int) -> bytes:
	# Parse the ELF header.
	ehdr = list(struct.unpack_from(ELF32_EHDR_FMT, original, 0))

	e_ident = ehdr[0]
	e_phoff = ehdr[5]
	e_phentsize = ehdr[9]
	e_phnum = ehdr[10]

	# Sanity checks.
	if e_ident[:4] != b"\x7fELF":
		raise ValueError("stupidass")

	if e_ident[4] != 1:
		raise ValueError("Not an ELF32. What are we doing here")

	if e_ident[5] != 1:
		raise ValueError("ok you're just being silly on purpose now")

	if e_phentsize != ELF32_PHDR_SIZE:
		raise ValueError(
			f"Incorrect program-header size of {e_phentsize:#x} in ELF file"
		)

	last_phdr_start = e_phoff + (e_phnum-1) * e_phentsize

	# Make sure the last phdr is a PT_LOAD
	ph = struct.unpack_from(
		ELF32_PHDR_FMT,
		original,
		last_phdr_start,
	)
	assert ph[0] == PT_LOAD

	align = 0x1000
	vaddr_offset = patchVirtualLoad % align

	blob_offset = len(original)

	# Round the file end up while preserving the low bits of the virtual address.
	if blob_offset % align <= vaddr_offset:
		blob_offset += vaddr_offset - (blob_offset % align)
	else:
		blob_offset += align + vaddr_offset - (blob_offset % align)

	result = bytearray(original)

	# Inject the patch phdr in place of the last phdr.
	patch_phdr = struct.pack_into(
		ELF32_PHDR_FMT,
		result,
		last_phdr_start,
		PT_LOAD,					# p_type
		blob_offset,				# p_offset
		patchVirtualLoad,			# p_vaddr
		patchVirtualLoad,			# p_paddr
		len(patchCore),				# p_filesz
		len(patchCore),				# p_memsz
		PF_R | PF_W | PF_X,			# p_flags
		align,						# p_align
	)

	if len(result) < blob_offset:
		result.extend(b"\x00" * (blob_offset - len(result)))

	# Append the actual code/data.
	result.extend(patchCore)
	return bytes(result)

THUNK_DISKSTORE_METHODS = [
	'Construct',
	'Destruct',
	'Open',
	'Close',
	'SeekToBeginning',
	'Seek',
	'SeekToEnd',
	'Read',
	'ReadString',
	'ReadLine',
	'Resize',
	'Write',
	'GetSize',
	'GetCurrPos',
	'IsOpen'
]

# The fun begins...
def main():
	REGION = sys.argv[1]
	GAME = sys.argv[2]

	# Open the ELF file so we can get at symbols in it.
	# Open the version-specific LD file so we can grab other symbols we need too
	patchElf = ElfWrapper(f'bin/europa_hostfs_{GAME}_{REGION}.elf')
	ldScript = LdScript.new(f'../europa_api/ld/{GAME}_{REGION}.ld')

	# This trampoline is written to the start of main().
	MAIN_STATIC_TRAMPOLINE = Mips.j(patchElf.symbol('_start')) + Mips.nop()

	# Create the patch core.
	patchCore = patchElf.makeCore()
	patchCoreEntry = patchElf.symbol('BLOB_LOAD_ADDRESS')

	# Read the original ELF file into memory.
	with open(f'../../elf/europa_{GAME}_{REGION}.elf', 'rb') as f:
		origCode = f.read()

	# Patch the ELF to inject the patch core as a new PT_LOAD header.
	# We do this outside of LIEF because LIEF can't do this in place, and it's
	# out-of-place moves sections, which is untenable.
	patched = patchInjectCore(origCode, patchCore, patchCoreEntry)

	# Once the patch core is injeted, we can use LIEF to perform the rest of our patches.
	rollingLief = lief.parse(patched)

	origEnd = rollingLief.get_int_from_virtual_address(ldScript.symbol('__HOOK_sbrk_break'), 4)
	adjustedEnd = alignUp(patchCoreEntry + len(patchCore), 0x1000)

	print(f'Original _end: 0x{origEnd:08x}, patch adjusted: 0x{adjustedEnd:08x}')

	def patchString(addr: int, newString: str):
		encoded = newString.encode('ascii') + b'\x00'
		rollingLief.patch_address(addr, list(encoded))
		print(f'Patched string at 0x{addr:08x}')

	def patchEraseString(addr: int):
		rollingLief.patch_address(addr, [0])
		print(f'Erased string at 0x{addr:08x}')

	def patchMakeStaticThunk(patch: int, thunkto: int):
		THUNK = Mips.j(thunkto) + Mips.nop()
		rollingLief.patch_address(patch, list(THUNK))
		print(f'Static thunk of 0x{patch:08x} to 0x{thunkto:08x} applied.')

	def patchMakeStreamMgrThunk(patch: int, thunkto: int, thunkOut: int):
		THUNK = Mips.move(Mips.a0, Mips.v0) + Mips.jal(thunkto) + Mips.nop() + Mips.j(thunkOut) + Mips.nop()
		rollingLief.patch_address(patch, list(THUNK))
		print(f'Static thunk of 0x{patch:08x} to 0x{thunkto:08x} applied.')

	# Patch the program break to be ahead of the core, so that memory allocations do not break us.
	# Once done, patch the start of main() to jump to the core entry point.
	rollingLief.patch_address(ldScript.symbol('__HOOK_sbrk_break'), adjustedEnd)
	rollingLief.patch_address(ldScript.symbol('main'), list(MAIN_STATIC_TRAMPOLINE))

	patchString(ldScript.symbol('IOPString_prefix'), 'host0:')
	patchEraseString(ldScript.symbol('IOPString_suffix'))
	patchString(ldScript.symbol('IOPString_IOPRP'), 'host0:sys/ioprp.img')
	patchString(ldScript.symbol('IOPString_SIO2MAN'), 'sys/sio2man.irx')
	patchString(ldScript.symbol('IOPString_PADMAN'), 'sys/padman.irx')
	patchString(ldScript.symbol('IOPString_LIBSD'), 'sys/libsd.irx')
	patchString(ldScript.symbol('IOPString_SDRDRV'), 'sys/sdrdrv.irx')
	patchString(ldScript.symbol('IOPString_LECAUDIO'), 'sys/lecaudio.irx')
	patchString(ldScript.symbol('IOPString_USBD'), 'sys/usbd.irx')
	patchString(ldScript.symbol('IOPString_USBKEYBD'), 'sys/usbd.irx')
	patchString(ldScript.symbol('IOPString_MCMAN'), 'sys/mcman.irx')
	patchString(ldScript.symbol('IOPString_MCSERV'), 'sys/mcserv.irx')

	patchMakeStreamMgrThunk(ldScript.symbol('CPS2StreamMgr_AddStreamHook'), patchElf.symbol('StreamMgr_addLecalStreamPath'), ldScript.symbol('CPS2StreamMgr_AddStreamThunkout'))

	# Thunk the CPS2CDStore and CPS2DiskStore class methods to our custom disk store implementation.
	for method in THUNK_DISKSTORE_METHODS:
		patchMakeStaticThunk(ldScript.symbol(f'CPS2DiskStore_{method}'), patchElf.symbol(f'CFileManDiskStore_{method}'))
		patchMakeStaticThunk(ldScript.symbol(f'CPS2CDStore_{method}'), patchElf.symbol(f'CFileManDiskStore_{method}'))


	# Write the patched ELF file to disk.
	rollingLief.write(f'../../elf/europa_{GAME}_{REGION}_hostfs_patched.elf')

	# Make the xdelta patch.
	pyxdelta.run(f'../../elf/europa_{GAME}_{REGION}.elf', f'../../elf/europa_{GAME}_{REGION}_hostfs_patched.elf', f'../../europa_{GAME}_{REGION}_hostfs_patch.xdelta')


if __name__ == '__main__':
	main()
