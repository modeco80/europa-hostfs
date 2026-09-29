#ifndef EUROPA_STD_STRING_H
#define EUROPA_STD_STRING_H

#include <ml/types.h>

struct europaStdStringRepr {
	u32 length;
	u32 capacity;
	u32 unk;
	char* mpszString;
};

struct europaStdString {
	europaStdStringRepr* repr;

	const char* cStr() const {
		if(repr)
			return repr->mpszString;
		return nil(const char*);
	}
};

#endif
