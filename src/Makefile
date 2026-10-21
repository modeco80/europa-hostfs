NAME = europa_hostfs_$(GAME)_$(REGION)
TYPE := elf
DEBUG ?= 0

# Runtime sources (startup code, and code
# needed to link the ml libs)
RUNTIME := \
	crt0.S \
	runtime/ml_abort.cpp

SOURCES := \
	$(RUNTIME) \
	main.cpp \
	file.cpp \
	filemanager.cpp \
	stringutil.cpp \
	console.cpp \
	cdcatalog.cpp \
	cdst_hooks.cpp \
	lecal.cpp \
	diskstore.cpp

LINKSCRIPT = $(OBJDIR)/blob.ld
LINKLIBS := \
	$(PATCHSDK)/src/utils/obj/libutils.a \
	$(PATCHSDK)/src/ml/lib/core/obj/libml3_core.a \
	$(PATCHSDK)/src/ml/lib/cxx/obj/libml3_cxx.a \
	$(PATCHSDK)/src/ml/lib/hw/obj/libml3_hw.a \

# User C/CXX flags.
USER_CFLAGS = -DREGION_$(REGION) -DGAME_$(GAME) \
	-I $(TOP)/europa_api/include \
	-I $(PATCHSDK)/src/utils/include
USER_CXXFLAGS := \
	-Wno-pmf-conversions


# Pre deps
EXTRATARGETS_PRE = $(LINKSCRIPT)
EXTRATARGETS_POST = \
	$(TOP)/europa_$(GAME)_$(REGION)_hostfs_patch.xdelta

# All files that make up the mkpatch tool.
# Used to make sure that we rebuild if the Python code is changed.
MKPATCH_DEPS := \
	mkpatch/main.py \
	mkpatch/patches.py \

include $(PATCHSDK)/mk/makedef.mk

# Preprocess the linker script to include game symbols here.
$(OBJDIR)/blob.ld: $(OBJDIR)/ blob.ld.in
	$(info Preprocessing linker script $@)
	sed 's|@REGION@|$(GAME)_$(REGION)|' blob.ld.in > $@

$(TOP)/europa_$(GAME)_$(REGION)_hostfs_patch.xdelta: bin/$(NAME).elf $(MKPATCH_DEPS)
	$(info Making patched ELF $@)
	mkpatch/main.py $(REGION) $(GAME) $@

