import lief

from mkpatch.utils import ElfWrapper, LdScript, PatchInjector, alignUp
from mkpatch.utils import Mips
from mkpatch.patch import PatchStep, PatchContext
from enum import Enum

# Exports
__all__ = [ 'PatchInjectCoreKind', 'Arguments', 'main' ]

## The kind of core injection you want to do.
class PatchInjectCoreKind(Enum):
	# Overwite the last program header.
	OVERWRITE_LAST_PHDR = 0
	# Inject a new program header.
	INJECT_NEW_PHDR = 1

## Arguments for [mkpatch.main()].
class Arguments():
	## Path to the patch code's ELF file.
	patchElfPath: str

	## Selects injection type. Not yet used.
	patchInjectKind: PatchInjectCoreKind

	## Path to the original ELF file.
	originalElfPath: str

	## Path to the LD script which will be used to find game symbols.
	apiLdScriptPath: str

	## List of patch steps to perform.
	patchSteps: list[PatchStep]

	def __init__(self):
		self.patchElfPath = ''
		self.patchInjectKind = PatchInjectCoreKind.OVERWRITE_LAST_PHDR
		self.originalElfPath = ''
		self.apiLdScriptPath = ''
		self.patchSteps = []

## Main for mkpatch. This is expected to be called by project-specific code.
def main(args: Arguments):
	# Open the ELF file so we can get at symbols in it.
	# Open the version-specific LD file so we can grab other symbols we need too
	patchElf = ElfWrapper(args.patchElfPath)
	ldScript = LdScript.new(args.apiLdScriptPath)

	# Create the binary patch core from the ELF file.
	patchCore = patchElf.makeCore()
	patchCoreEntry = patchElf.symbol('BLOB_LOAD_ADDRESS')

	# Read the original ELF file into memory.
	with open(args.originalElfPath, 'rb') as f:
		origElfData = f.read()

	# Patch the ELF to inject the patch core as a new PT_LOAD program header.
	# We do this outside of LIEF because LIEF can't do this in place, and it's
	# out-of-place injection moves the section load addresses, which breaks the executable.
	patched = PatchInjector.injectOverwriteLastPHDR(origElfData, patchCore, patchCoreEntry)

	# Once the patch core is injeted, we can use LIEF to perform the rest of our patches.
	liefBinary = lief.parse(patched)

	# Patch the program break (heap start) to be ahead of the core, so that memory allocations do not overwrite
	# the patch code core. Not doing this would cause errors and eventually a crash, so it is quite important!
	origEnd = liefBinary.get_int_from_virtual_address(ldScript.symbol('__HOOK_sbrk_break'), 4)
	liefBinary.patch_address(ldScript.symbol('__HOOK_sbrk_break'), alignUp(patchCoreEntry + len(patchCore), 0x1000))

	MAIN_STATIC_TRAMPOLINE = Mips.j(patchElf.symbol('_start')) + Mips.nop()
	# Once done, patch the start of the main() function to jump to the core entry point.
	liefBinary.patch_address(ldScript.symbol('main'), list(MAIN_STATIC_TRAMPOLINE))

	# Create the patch context. This is passed to individual patch steps.
	ctx = PatchContext()
	ctx.patchBinary = patchElf
	ctx.liefBinary = liefBinary
	ctx.ldScript = ldScript

	# Run each patch step. If a patch step throws any error, give up.
	for step in args.patchSteps:
		step.runStep(ctx)

	return liefBinary
