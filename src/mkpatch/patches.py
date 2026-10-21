from mkpatch.patch import PatchStep, PatchContext

# Methods of CStorage which are thunked.
THUNK_DISKSTORE_METHODS = [
	'Construct', # Actually the C++ constructor
	'Destruct', # Actually the C++ destructor
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

class StringPatches(PatchStep):
	def runStep(self, context: PatchContext):
		print(f'Running StringPatches')
		context.patchString(context.ldScript.symbol('IOPString_prefix'), 'host0:')
		context.patchEraseString(context.ldScript.symbol('IOPString_suffix'))
		context.patchString(context.ldScript.symbol('IOPString_IOPRP'), 'host0:sys/ioprp.img')
		context.patchString(context.ldScript.symbol('IOPString_SIO2MAN'), 'sys/sio2man.irx')
		context.patchString(context.ldScript.symbol('IOPString_PADMAN'), 'sys/padman.irx')
		context.patchString(context.ldScript.symbol('IOPString_LIBSD'), 'sys/libsd.irx')
		context.patchString(context.ldScript.symbol('IOPString_SDRDRV'), 'sys/sdrdrv.irx')
		context.patchString(context.ldScript.symbol('IOPString_LECAUDIO'), 'sys/lecaudio.irx')
		context.patchString(context.ldScript.symbol('IOPString_USBD'), 'sys/usbd.irx')
		context.patchString(context.ldScript.symbol('IOPString_USBKEYBD'), 'sys/usbkeybd.irx')
		context.patchString(context.ldScript.symbol('IOPString_MCMAN'), 'sys/mcman.irx')
		context.patchString(context.ldScript.symbol('IOPString_MCSERV'), 'sys/mcserv.irx')

## Performs static thunks for CStorage
class StorageThunks(PatchStep):
	def runStep(self, context: PatchContext):
		print(f'Running StorageThunks')
		# Thunk the CPS2CDStore and CPS2DiskStore class methods to our custom disk store implementation.
		for method in THUNK_DISKSTORE_METHODS:
			context.patchMakeStaticThunk(context.ldScript.symbol(f'CPS2DiskStore_{method}'), context.patchBinary.symbol(f'CFileManDiskStore_{method}'))
			context.patchMakeStaticThunk(context.ldScript.symbol(f'CPS2CDStore_{method}'), context.patchBinary.symbol(f'CFileManDiskStore_{method}'))

class StreamMgrThunks(PatchStep):
	def runStep(self, context: PatchContext):
		print(f'Running StreamMgrThunks')
		context.patchMakeThunkInThunkOutThunk(context.ldScript.symbol('CPS2StreamMgr_AddStreamHook'), context.patchBinary.symbol('StreamMgr_addLecalStreamPath'), context.ldScript.symbol('CPS2StreamMgr_AddStreamThunkout'))
