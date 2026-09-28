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
sceCdStInit = 0x002cd8a0;
sceCdStStart = 0x002cd8d0;
sceCdStStop = 0x002cd968;
sceCdStRead = 0x002cd9a0;

/* Europa! */

main = 0x003a8b50;
mainTrampoline = 0x0003a8b58;

