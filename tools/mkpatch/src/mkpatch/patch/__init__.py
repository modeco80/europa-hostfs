#!/bin/env python3

import lief
from mkpatch.utils import ElfWrapper, LdScript
from mkpatch.utils import Mips
from abc import ABC, abstractmethod

# Exports
__all__ = [ 'PatchContext', 'PatchStep']

### Provides important context to [PatchStep], which is required.
### Also provides helper methods.
class PatchContext:
	patchBinary: ElfWrapper
	## The LIEF binary
	liefBinary: lief.Binary
	ldScript: LdScript

	def patchString(self, addr: int, newString: str):
		encoded = newString.encode('ascii') + b'\x00'
		self.liefBinary.patch_address(addr, list(encoded))

	def patchEraseString(self, addr: int):
		self.liefBinary.patch_address(addr, [0])

	def patchMakeStaticThunk(self, patch: int, thunkto: int):
		THUNK = Mips.j(thunkto) + Mips.nop()
		self.liefBinary.patch_address(patch, list(THUNK))

	## Patches an address to call a thunk-in function, and then jump to a thunk-out location.
	def patchMakeThunkInThunkOutThunk(self, patch: int, thunkto: int, thunkOut: int):
		THUNK = Mips.move(Mips.a0, Mips.v0) + Mips.jal(thunkto) + Mips.nop() + Mips.j(thunkOut) + Mips.nop()
		self.liefBinary.patch_address(patch, list(THUNK))

## Abstraction over a patch step.
class PatchStep(ABC):
	## Runs the patch step.
	@abstractmethod
	def runStep(self, context: PatchContext):
		pass
