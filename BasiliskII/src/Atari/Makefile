# Atari makefile for Basilisk II.
# Using Vincent Rivière's GCC 4.6.4 cross-compiler

## System specific configuration

CC = ~/gccfilter -c m68k-atari-mint-gcc
NATIVE_GCC = ~/gccfilter -c gcc
STRIPX = tools/stripx

LIBCMINI_DIR	= ext/libcmini
GEMLIB_DIR		= ext/gemlib
GEMLIB			= $(GEMLIB_DIR)/libgem.a

LIBCMINI_INC	= -I$(LIBCMINI_DIR)/include
LIBCMINI_LIB	= -L$(LIBCMINI_DIR)/build/m68020-60
LIBCMINI_CRT	= $(LIBCMINI_DIR)/build/minicrt0.o
#LIBCMINI_CRT	= $(LIBCMINI_DIR)/build/crt0.o
LIBCMINI_CFL	= -ffast-math -nostdlib 
LIBCMINI_LFL	= #-nodefaultlibs -nostdlib -nostartfiles -lcmini
# -lgcc -lstdc++ -lcmini -lgcc

INCLUDE = $(LIBCMINI_INC) -I$(GEMLIB_DIR) -I../include -I../native_cpu -I. -nodefaultlibs -nostdlib
LDFLAGS = $(LIBCMINI_LIB) -l:$(GEMLIB) -lgcc -lstdc++ -lcmini -lgcc


SYMFLAGS = -Wl,--traditional-format

PARCP			= ./tools/parcp421/apple/parcp
PARCP_DST		= D:\BASILISK


# target parameter
ifeq ($(target),)
TARGET = release
else
TARGET = $(target)
endif

ifeq ($(TARGET),debug)
TARGET_SUFFIX = D
TARGET_DEF = -g -O1
#TARGET_DEF += -DHEAVYDEBUG
STRIP =
else
TARGET_SUFFIX =
TARGET_DEF = -O3
#TARGET_DEF += -DNDEBUG
#STRIP = $(STRIPX) -s -v -f $(BIN)
STRIP = $(STRIPX) -v -f $(BIN)
endif


# cpu parameter
CPU = $(cpu)
ifeq ($(CPU),68030)
	CPU_MACH = -m68030
	CPU_DEF = -DCPU=030
	CPU_SUFFIX = 030
else ifeq ($(CPU),68040)
	CPU_MACH = -m68040
	CPU_DEF = -DCPU=040
	CPU_SUFFIX = 040
else ifeq ($(CPU),68060)
	CPU_MACH = -m68060
	CPU_DEF = -DCPU=060
	CPU_SUFFIX = 060
else
	CPU = 68020-60
	CPU_MACH = -m68020-60
	CPU_DEF = -DCPU=020
	CPU_SUFFIX =
endif


OUT_APP = basilisk.prg
OUT_DIR = ../../bin/
OBJ_DIR = obj/$(CPU)$(TARGET_SUFFIX)
DIST_NAME = Basilisk
DIST_ZIP = basilisk.zip
DIST_DIR = dist/$(DIST_NAME)

APP = $(OUT_DIR)$(OUT_APP)
BIN = $(OBJ_DIR)/$(OUT_APP)

DEFS = -DATARI $(CPU_DEF) $(TARGET_DEF)

CXXFLAGS = -std=c++0x \
	$(LIBCMINI_CFL) \
	$(DEFS) \
	$(SYMFLAGS) \
	-fpermissive -fomit-frame-pointer -fno-exceptions \
	-Wl,-Map,mapfile \

SFLAGS = $(DEFS)

SRCS = blank.S \
	../main.cpp main_atari.cpp ../prefs.cpp ../prefs_items.cpp \
    prefs_atari.cpp prefs_editor_atari.cpp sys_atari.cpp ../rom_patches.cpp \
    ../slot_rom.cpp ../rsrc_patches.cpp ../emul_op.cpp \
    ../macos_util.cpp ../xpram.cpp xpram_atari.cpp ../timer.cpp \
    timer_atari.cpp clip_atari.cpp ../adb.cpp ../serial.cpp \
    serial_atari.cpp ../ether.cpp ether_atari.cpp ../sony.cpp ../disk.cpp \
    ../cdrom.cpp ../scsi.cpp scsi_atari.cpp ../video.cpp video_atari.cpp \
    ../audio.cpp audio_atari.cpp  \
    ../user_strings.cpp user_strings_atari.cpp \
	debug_atari.cpp input_atari.cpp zeropage.cpp 68030_pmmu.cpp \
	asm_support.S asm_support030.S asm_support040.S asm_support060.S \
	ext/68040_pmmu.S \
	ext/c2p/c2p1x1_8_from_8.S ext/c2p/c2p1x1_8_from_8_halfx.S ext/c2p/c2p1x1_8_from_8_halfx_060.S \
	ext/c2p/c2p1x1_4_from_8.S ext/c2p/c2p1x1_4_from_8_halfx.S \
	ext/c2p/c2p1x1_4_from_4.S \
	

DISASM =


## Rules
.PHONY: clean cleanTarget info dist distclean

target: info $(BIN)
	cp $(BIN) $(APP)
	@echo

debug:
	@make cpu=$(CPU) target=debug

release:
	@make cpu=$(CPU) target=release

info:
	@echo Building $(APP) \($(CPU_MACH) : $(TARGET)\)

$(OBJ_DIR):
	@[ -d obj ] || mkdir obj > /dev/null 2>&1
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

define SRCS_LIST_TO_OBJS
	$(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(foreach file, $(SRCS), \
	$(basename $(notdir $(file))))))
endef
OBJS = $(SRCS_LIST_TO_OBJS)

SRC_PATHS += $(sort $(foreach file, $(SRCS), $(dir $(file))))
VPATH :=
VPATH += $(addprefix :, $(subst  ,:, $(filter-out $($(subst, :, ,$(VPATH))), $(SRC_PATHS))))

$(BIN): $(OBJ_DIR) $(DISASM) $(STRIPX) $(LIBCMINI_CRT) $(GEMLIB) $(OBJS)
	$(CC) $(CXXFLAGS) $(LIBCMINI_CRT) $(OBJS) $(LDFLAGS) -o $(BIN)
	$(STRIP)

$(LIBCMINI_CRT):
	cd $(LIBCMINI_DIR) && make

$(GEMLIB):
	cd $(GEMLIB_DIR) && make

$(STRIPX): $(STRIPX).c
	$(NATIVE_GCC) $(STRIPX).c -o $(STRIPX)


$(OBJ_DIR)/asm_support030.o : asm_support030.S
	$(CC) $(INCLUDE) $(SFLAGS) -m68030 -c $< -o $@

$(OBJ_DIR)/asm_support040.o : asm_support040.S
	$(CC) $(INCLUDE) $(SFLAGS) -m68040 -c $< -o $@

$(OBJ_DIR)/asm_support060.o : asm_support060.S
	$(CC) $(INCLUDE) $(SFLAGS) -m68060 -c $< -o $@

$(OBJ_DIR)/%.o : %.S
	$(CC) $(INCLUDE) $(SFLAGS) $(CPU_MACH) -c $< -o $@

$(OBJ_DIR)/%.o : %.cpp
	$(CC) $(INCLUDE) $(CXXFLAGS) $(CPU_MACH) -c $< -o $@

%.disasm : %.cpp
	$(CC) $(INCLUDE) $(CXXFLAGS) $(CPU_MACH) -fverbose-asm -g -S $< -o $@

disasm: $(DISASM)

dist: $(BIN)
	@[ -d dist ] || mkdir dist > /dev/null 2>&1
	@[ -d $(DIST_DIR) ] || mkdir $(DIST_DIR) > /dev/null 2>&1
	cp -f $(BIN) $(DIST_DIR)/$(OUT_APP)
	cp -f ../../COPYING $(DIST_DIR)/license.txt
	cp -f readme.txt $(DIST_DIR)/readme.txt
	cp -f basilisk.inf $(DIST_DIR)/basilisk.inf
	cp -f basilisk.rsc $(DIST_DIR)/basilisk.rsc
	cd dist && zip -9 temp.zip -r $(DIST_NAME) && mv temp.zip $(DIST_ZIP)


distclean:
	make clean
	find . -name '.DS_Store' -type f -delete
	rm -f $(DIST_DIR)/*
	rm -d -f $(DIST_DIR)
	rm -f dist/*
	rm -d -f dist
	rm -f -d obj
	rm -f mapfile
	rm -f $(STRIPX)
	cd $(GEMLIB_DIR) && make clean
	cd $(LIBCMINI_DIR) && make clean
	rm -d -f $(LIBCMINI_DIR)/build

clean:
	@rm -f *~ *.bak *.disasm obj.0000.* parcp.cmd mapfile
	@make cleanTarget target=debug
	@make cleanTarget target=release
	@make cleanTarget cpu=68030 target=debug
	@make cleanTarget cpu=68040 target=debug
	@make cleanTarget cpu=68060 target=debug
	@make cleanTarget cpu=68030 target=release
	@make cleanTarget cpu=68040 target=release
	@make cleanTarget cpu=68060 target=release
	@rm -f $(APP)

cleanTarget:
	@rm -f $(BIN)
	@rm -f $(OBJ_DIR)/*
	@rm -f $(OBJ_DIR)/.DS_Store
	@rm -f -d $(OBJ_DIR)


go:
	echo "CD $(PARCP_DST)" > parcp.cmd
	echo "LCD $(OUT_DIR)" >> parcp.cmd
	echo "PUT $(OUT_APP)" >> parcp.cmd
	echo "PUT basilisk.rsc" >> parcp.cmd
	echo "EXEC -n $(OUT_APP)" >> parcp.cmd
	echo "QUIT" >> parcp.cmd
	$(PARCP) -b parcp.cmd


#-------------------------------------------------------------------------
# DO NOT DELETE THIS LINE -- make depend depends on it.
