#ifndef EUROPA_STD_STRING_H
#define EUROPA_STD_STRING_H

#include <ml/types.h>

struct europaStdStringRepr {
	u32 unk1;
	u32 unk2;
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
