#ifndef STRINGUTIL_HPP
#define STRINGUTIL_HPP

/// Returns true if the string [haystack] starts with the string [needle].
bool stringStartsWith(const char* __restrict haystack, const char* __restrict needle);

/// Copies [src] to [dst], lower-casing the string if required.
void stringCopyLower(char* __restrict dst, const char* __restrict src);

#endif
