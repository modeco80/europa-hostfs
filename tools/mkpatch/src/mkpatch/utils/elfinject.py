import struct

ELF32_EHDR_FMT = "<16sHHIIIIIHHHHHH"
ELF32_PHDR_FMT = "<IIIIIIII"

ELF32_EHDR_SIZE = struct.calcsize(ELF32_EHDR_FMT)
ELF32_PHDR_SIZE = struct.calcsize(ELF32_PHDR_FMT)

PT_LOAD = 1

PF_X = 1
PF_W = 2
PF_R = 4

__all__ = ['PatchInjector']

class PatchInjector:
	### Injects core by overwriting a junk secondary PT_LOAD program header. Used for Metrowerks linker
	### binaries, since they do not give enough space to make a new program header.
	@staticmethod
	def injectOverwriteLastPHDR(original: bytes, patchCore: bytes, patchVirtualLoad: int) -> bytes:
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
		# TODO: We may want to check if the phdr is actually loading anything,
		# and fail if that happens to be the case.

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
