#ifndef STRINGUTIL_HPP
#define STRINGUTIL_HPP

/// Returns true if [haystack] starts with needle. Note that this is implemented
/// in a very boneheaded way, as it is not called hot. If it is, it probably should be
/// reimplmented.
bool stringStartsWith(const char* __restrict haystack, const char* __restrict needle);

/// Copies [src] to [dst], lower-casing the string if required.
void stringCopyLower(char* __restrict dst, const char* __restrict src);

#endif
