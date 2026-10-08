# Europa HostFS Patch

This is a HostFS patch for the Europa based games.

# Installation

Extract all files from the disc. 

Patch the game ELF file using an xdelta patcher of your choosing.

### Starfighter-specific

You may remove the `EUROPA.CD` file, since it is not used or read by the game at all anymore.

Using a text editor of your choosing, edit `EUROPACD.REG`, and scroll down to the portion of the file which contains with `LucasArts Entertainment Company LLC\Europa Dev Platform\Streaming`. After that line should be a line that looks like:

```
"StreamDirs"= ...
```

The strings in there have $ characters in them, which need to be removed. Once removed, save the file, and that's that.

## Continuation

Add the directory you extracted the files to as a search directory in PCSX2. Open Game Properties on the patched elf and ensure that "Host Filesystem" is checked (if you have this checked in your global settings, you do not need to do this step. If you have to ask, you probably don't, which means you probably do need to do this step).

Launch the patched ELF from your game list and enjoy!

# How does this work?

The ELF is statically patched to jump to code that we also patch into the ELF file (besides some boring string patches and static trampolines).

The code that the ELF is hijacked into running to sets up more advanced hooks, and includes a re-implementation of the game's CStorage interface, which it uses for file I/O. While this was not entirely nesscary, I found it cleaner to completely reimplement the class, since the file layer that the reimplementation is built on is also used for other things.

The game's lookup for CD sectors is hijacked to instead return a pointer to filename that we consume in specific cases:
	- Calls to LECAL (their own re-imagination of RAD's Miles Sound System?? I'm not sure as to why, since it was on PS2 to my knowledge, but... hey) were originally hardcoded to use CD LBAs. The patch code re-orders the arguments to the RPC call function, and changes the RPC command to the correct one to use host filesystem, which works!
	- The SCE CD stream functionality, used by the movie playback, is also hijacked to use our file manager layer.

# Building

You need:

- Python
- Pipenv
- ee-gcc `2.96-ee-001003-1` (place it in `tools/cc/2.96-ee-001003-1`)
- The original Europa ELF file

Make an `elf` directory in the root of the repository, and copy the original game ELF to `elf/[game]_[region].elf`.

Install python dependencies and enter the pipenv managed virtualenv:
```
pipenv install
pipenv shell
```

Build the patch. It will be pressent at the root of the repository as `europa_[game]_[region]_hostfs_patch.xdelta`.

```
make
```


