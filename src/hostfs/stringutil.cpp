#include <ml/string.h>
#include "stringutil.hpp"

// evil. Does NOT need to be fast.
bool stringStartsWith(const char* __restrict haystack, const char* __restrict needle) {
	const u32 haystackLength = strlen(haystack);
	const u32 needleLength = strlen(needle);

	// can't completely store the needle, so don't bother
	if(haystackLength < needleLength)
		return false;

	// Though we could unroll these to word at a time, I suppose.
	for(u32 i = 0; i < needleLength; ++i)
		if(haystack[i] != needle[i])
			return false;

	return true;
}

void stringCopyLower(char* __restrict dst, const char* __restrict src) {
	const u32 len = strlen(src);
	for(u32 i = 0; i < len; ++i) {
		if(src[i] >= 'A' && src[i] <= 'Z') {
			dst[i] = src[i] | 0x20;
		} else {
			dst[i] = src[i];
		}
	}
	dst[len] = '\0';
}
