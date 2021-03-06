## Architecture
ARCH := ia32
EXTRA_DEFS=-DKDEBUG=1
EXTRA_DEFS+=-DPAGE_POISON=1
EXTRA_DEFS+=-DOBJCACHE_POISON=1
EXTRA_DEFS+=-DOBJCACHE_DEBUG_FREE=1

TOPDIR :=  .
#$(shell /bin/pwd)
KERNEL_DIR := $(TOPDIR)/kernel
ARCH_DIR := $(TOPDIR)/arch-$(ARCH)
FS_DIR := $(TOPDIR)/fs
DRIVERS_DIR := $(TOPDIR)/drivers

.PHONY: all clean squeaky boot_floppy userland

## Target toolchain prefix
CROSS_COMPILE=

## command locations
SH := /bin/sh
RM := rm
MAKE := make
LN := ln
CP := cp

ifdef SPARSE
GCC := $(CROSS_COMPILE)cgcc
EXTRA_DEFS+=-Wno-old-initializer
else
GCC := $(CROSS_COMPILE)gcc
endif
CC  := $(CROSS_COMPILE)gcc
AR  := $(CROSS_COMPILE)ar
STRIP := $(CROSS_COMPILE)strip

# Default target
TARGET: all

# Compiler flags
#	-fno-inline \
#
CFLAGS  :=-pipe -ggdb -O2 \
	-mtune=corei7 \
	-flto -fwhole-program \
	-m32 -ffreestanding -fno-stack-protector \
	-Wall -Wsign-compare -Wcast-align -Waggregate-return \
	-Wstrict-prototypes -Wmissing-prototypes \
	-Wmissing-declarations -Wmissing-noreturn \
	-Wmissing-format-attribute \
	-I$(TOPDIR)/include $(EXTRA_DEFS)

./include/arch:
	@echo " [SYMLINK] ./include/arch -> arch-$(ARCH)"
	@$(LN) -sf arch-$(ARCH) ./include/arch

# templates
%.o %.d: %.c ./include/arch Makefile
	@echo " [C] $(patsubst %.d, %.c, $@)"
	@$(GCC) $(CFLAGS) \
		-MMD -MF $(patsubst %.o, %.d, $@) -MT $(patsubst %.d, %.o, $@) \
		-c -o $(patsubst %.d, %.o, $@) $< 
%.d %.o: %.S ./include/arch Makefile
	@echo " [ASM] $(patsubst %.d, %.S, $@)"
	@$(GCC) $(CFLAGS) -D__ASM__ \
		-MMD -MF $(patsubst %.o, %.d, $@) -MT $(patsubst %.d, %.o, $@) \
		-c -o $(patsubst %.d, %.o, $@) $< 

include arch-$(ARCH)/Makefile
include kernel/Makefile
include fs/Makefile
include drivers/Makefile

IMAGE_OBJ := $(patsubst %.S, %.o, $(ARCH_ASM_SOURCES)) \
		$(patsubst %.c, %.o, $(ARCH_C_SOURCES)) \
		$(patsubst %.c, %.o, $(KERNEL_C_SOURCES)) \
		$(patsubst %.c, %.o, $(FS_C_SOURCES)) \
		$(patsubst %.c, %.o, $(DRIVERS_C_SOURCES))

ALL_SOURCES := $(ARCH_C_SOURCES) $(ARCH_ASM_SOURCES) \
		$(KERNEL_C_SOURCES) \
		$(FS_C_SOURCES) \
		$(DRIVERS_C_SOURCES)

# Generate dependencies
ALL_DEPS := $(patsubst %.S, %.d, $(ARCH_ASM_SOURCES)) \
		$(patsubst %.c, %.d, $(ARCH_C_SOURCES)) \
		$(patsubst %.c, %.d, $(KERNEL_C_SOURCES)) \
		$(patsubst %.c, %.d, $(FS_C_SOURCES)) \
		$(patsubst %.c, %.d, $(DRIVERS_C_SOURCES))

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),squeaky)
-include $(ALL_DEPS)
endif
endif

kernel.elf: Makefile $(IMAGE_OBJ) $(ARCH_DIR)/kernel.lnk
	@echo " [LINK] $@"
	@$(GCC) $(CFLAGS) -Wl,-melf_i386 -Wl,-nostdlib \
		-nostartfiles \
		-Wl,-T,$(ARCH_DIR)/kernel.lnk -o $@ \
		$(IMAGE_OBJ)

kernel.elf.stripped: kernel.elf
	@echo " [STRIP] $@"
	@$(CP) $< $@
	@$(STRIP) $@

kernel.elf.gz: kernel.elf.stripped
	@echo " [COMPRESS] $@"
	@gzip -c < $< > $@

all: kernel.elf.gz

userland:
	@echo " [USERLAND]"
	+$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) \
		BOOT_FLOPPY="../boot.img" \
		-C user boot_floppy

boot.img: userland kernel.elf.gz menu.lst
	@echo " [BOOTFLOPPY] $@"
	@e2fsck -y ./boot.img || true
	@e2cp kernel.elf.gz ./boot.img:kernel
	@e2cp menu.lst ./boot.img:

boot_floppy: boot.img

clean:
	$(RM) -f $(IMAGE_OBJ) $(ALL_DEPS) \
		$(KERNEL_OBJ) $(KERNEL_DIR)/kernel.o \
		$(ARCH_OBJ) $(ARCH_DIR)/arch.o \
		$(FS_OBJ) $(FS_DIR)/fs.o \
		kernel.elf kernel.elf.stripped kernel.elf.gz
	make -C user clean

squeaky: clean
	$(RM) -f ./include/arch
