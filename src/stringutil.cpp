#include <ml/string.h>
#include "stringutil.hpp"

bool stringStartsWith(const char* __restrict haystack, const char* __restrict needle) {
	const u32 haystackLength = strlen(haystack);
	const u32 needleLength = strlen(needle);

	// We can't completely store the needle in the haystack,
	// so don't even bother to compare for its existance.
	if(haystackLength < needleLength)
		return false;

	const u32 haystackWords = haystackLength / sizeof(u32);
	const u32 haystackOffcut = haystackLength % sizeof(u32);
	const u32 needleWords = needleLength / sizeof(u32);
	const u32 needleOffcut = needleLength % sizeof(u32);

	const u32* pHaystackWords = reinterpret_cast<const u32*>(haystack);
	const u32* pNeedleWords = reinterpret_cast<const u32*>(needle);

	// Iff both the haystack and needle are at least one word long, it is possible
	// for us to compare a word at a time. In the case that this is not true,
	// we have to (unfortunately) fall back to a slow byte-by-byte comparision.
	if(haystackWords != 0 && needleWords != 0) {
		for(u32 i = 0; i < needleWords; ++i)
			if(pHaystackWords[i] != pNeedleWords[i])
				return false;

		// Compare any remainder that exists as bytes.
		for(u32 i = (needleWords*sizeof(u32)); i < needleLength; ++i)
			if(haystack[i] != needle[i])
				return false;
	} else {
		for(u32 i = 0; i < needleLength; ++i)
			if(haystack[i] != needle[i])
				return false;
	}

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
