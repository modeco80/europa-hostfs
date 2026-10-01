#ifndef EUROPA_STD_STRING_H
#define EUROPA_STD_STRING_H

#include <ml/types.h>

// The repressentation object pointed to inside of an [europaStdString] object.
struct europaStdStringRepr {
	u32 length;
	u32 capacity;
	u32 unk;
	char* mpszString;
};

// Europa's version of the `std::string` class.
// We currently do not support any mutation on these objects,
// and only support read-only operations.
struct europaStdString {
	europaStdStringRepr* repr;

	const char* cStr() const {
		if(repr)
			return repr->mpszString;
		return nil(const char*);
	}
};

#endif
