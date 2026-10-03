/* C library */
strlen = 0x00404868;
strcat = 0x00404348;
memcpy = 0x00402030;
memset = 0x004021e8;
sprintf = 0x00404020;
vsprintf = 0x00408970;

malloc = 0x00401470;
free = 0x00401498;


/* Hook specific things */

__HOOK_sbrk_break = 0x0048e194;

/* original value of the Europa ELF's _end symbol, as captured in the program break address */
__HOOK_ORIG_END = 0x00619500;

/* The fun stuff */

/* PS2 SDK */

/* FIO */
sceOpen = 0x003fa110;
sceClose = 0x003fa268;
sceRead = 0x003fa490;
sceLSeek = 0x003fa320;

/* libcdvd */
sceCdStInit = 0x00415d90;
sceCdStStart = 0x00415dc0;
sceCdStStop = 0x00415e28;
sceCdStRead = 0x00415e60;

/* Europa! */

main = 0x003a8b50;
mainTrampoline = 0x0003a8b58;

LECOutputConsoleImpl = 0x0012c9f0;

/* IOP module strings */

IOPString_IOPRP = 0x004cf3b0;

IOPString_prefix = 0x004cf398;
IOPString_suffix = 0x004cf3a8;

IOPString_SIO2MAN = 0x004cf3d0;
IOPString_PADMAN = 0x004cf3e0;
IOPString_LIBSD = 0x004cf3f0;
IOPString_SDRDRV = 0x004cf400;
IOPString_LECAUDIO = 0x004cf410;
IOPString_USBD = 0x004cf428;
IOPString_USBKEYBD = 0x004cf440;
IOPString_MCMAN = 0x004cf458;
IOPString_MCSERV = 0x004cf468;

/* CStorage */

CStorage_ReadByte = 0x0011a240;
CStorage_ReadShort = 0x0011a270;
CStorage_ReadInt = 0x0011a2a0;

CStorage_ReadString = 0x0011a2d0;
CStorage_ReadLine = 0x0011a380;

CStorage_Copy = 0x0011a450;
CStorage_CopyAll = 0x0011a690;

CStorage_WriteValue = 0x0011a940;
CStorage_WriteLine = 0x0011ab70;
CStorage_WriteFormattedLine = 0x0011a800;
CStorage_WriteFormattedString = 0x0011a8b0;
CStorage_WriteFormattedText = 0x0011a770;

CStorage_GetChecksum = 0x0011ac80;

CStorage_IsAtEnd = 0x0011ac20;

/* Disk CStorage impl garbage. */

CPS2CDStore_Construct = 0x302990;
CPS2CDStore_Destruct = 0x302b20;
CPS2CDStore_Open = 0x302c20;
CPS2CDStore_Close = 0x302d20;
CPS2CDStore_SeekToBeginning = 0x302d50;
CPS2CDStore_Seek = 0x302da0;
CPS2CDStore_SeekToEnd = 0x302e00;
CPS2CDStore_Read = 0x302e60;
CPS2CDStore_ReadString = 0x302ed0;
CPS2CDStore_ReadLine = 0x302f50;
CPS2CDStore_Write = 0x302ff0;
CPS2CDStore_Resize = 0x303000;
CPS2CDStore_GetSize = 0x303010;
CPS2CDStore_GetCurrPos = 0x303060;
CPS2CDStore_IsOpen = 0x3030b0;

/* CPS2DiskStore methods */
CPS2DiskStore_RTTI = 0x004d3358;

CPS2DiskStore_Construct = 0x13c520;
CPS2DiskStore_Destruct = 0x13c6b0;
CPS2DiskStore_Open = 0x13c7b0;
CPS2DiskStore_Close = 0x13ca80;
CPS2DiskStore_SeekToBeginning = 0x13cad0;
CPS2DiskStore_Seek = 0x13cb00;
CPS2DiskStore_SeekToEnd = 0x13cb30;
CPS2DiskStore_Read = 0x13cb60;
CPS2DiskStore_ReadString = 0x13cbc0;
CPS2DiskStore_ReadLine = 0x13cc60;
CPS2DiskStore_Write = 0x13cd20;
CPS2DiskStore_Resize = 0x13cd30;
CPS2DiskStore_GetSize = 0x13cd40;
CPS2DiskStore_GetCurrPos = 0x13cd70;
CPS2DiskStore_IsOpen = 0x13cda0;


/* Cd sex */
ps2LoadCdCatalog = 0x00302170;
ps2QueryCdCatalog = 0x003025d0;
ps2CdCatalogLoaded = 0x005cf9f4;
ps2CdRead = 0x00302890;
ps2FileExists = 0x00303680;

