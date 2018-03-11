# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies.
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1994 by Linus Torvalds
# Changes for MMIX by Eiji Yoshiya
#

LD		:= $(LD) -m elf64mmix

LDFLAGS_vmlinux := -Ttext 0x8000000000000000 -Bstatic
CFLAGS		+= -mno-base-addresses -pipe

head-y				:= arch/mmix/kernel/head.o
libs-y 				+= arch/mmix/lib/
core-y				+= arch/mmix/kernel/ \
				   arch/mmix/mm/
drivers-$(CONFIG_VMMMIX)	+= arch/mmix/vmmmix_io/
drivers-$(CONFIG_DE0)		+= arch/mmix/DE0/

all: vmlinux.bin

CPPFLAGS_vmlinux.lds	:= -Ummix

vmlinux.bin: vmlinux
	mmix-objcopy -O binary vmlinux vmlinux.bin
