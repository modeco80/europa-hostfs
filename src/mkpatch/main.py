#!/bin/env python3
import mkpatch
import pyxdelta
import sys
import os

# need this for local imports
sys.path.append(os.path.abspath(os.path.dirname(__file__)))
from patches import StringPatches, StorageThunks, StreamMgrThunks

REGION = sys.argv[1]
GAME = sys.argv[2]

ORIGINAL_ELF_PATH = f'../elf/europa_{GAME}_{REGION}.elf'
PATCHED_ELF_PATH = f'../elf/europa_{GAME}_{REGION}_hostfs_patched.elf'
XDELTA_PATCH_PATH = sys.argv[3] # Provided by build system now :)

def main():
	# set up mkpatch
	args = mkpatch.Arguments()
	args.patchElfPath = f'bin/europa_hostfs_{GAME}_{REGION}.elf'
	args.originalElfPath = ORIGINAL_ELF_PATH
	args.apiLdScriptPath = f'../europa_api/ld/{GAME}_{REGION}.ld'

	# Add steps
	args.patchSteps.append(StringPatches())
	args.patchSteps.append(StorageThunks())
	args.patchSteps.append(StreamMgrThunks())

	# run mkpatch
	binary = mkpatch.main(args)

	# Write the patched ELF file and make the xdelta patch
	binary.write(PATCHED_ELF_PATH)
	pyxdelta.run(ORIGINAL_ELF_PATH, PATCHED_ELF_PATH, XDELTA_PATCH_PATH)

if __name__ == "__main__":
	main()
