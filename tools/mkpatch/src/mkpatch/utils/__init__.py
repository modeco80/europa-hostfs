#!/bin/env python3

# Re-export modules
from .elf import ElfWrapper
from .ldparse import LdScript
from .mips import Mips
from .elfinject import PatchInjector

def alignUp(value: int, align: int) -> int:
	return (value + align - 1) & ~(align - 1)
