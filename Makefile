# -----------------------------------------------------------------------------

megabuild		= 0
attachdebugger	= 0

# -----------------------------------------------------------------------------

MAKE			= make
CP				= cp
MV				= mv
RM				= rm -f
CAT				= cat

SRC_DIR			= ./src
UI_SRC_DIR		= ./src/ui
UIELT_SRC_DIR	= ./src/ui/uielements
DRVRS_SRC_DIR	= ./src/drivers
EXE_DIR			= ./exe
BIN_DIR			= ./bin

# mega65 fork of ca65: https://github.com/dillof/cc65
AS				= ca65mega
ASFLAGS			= -g -D megabuild=$(megabuild) --cpu 45GS02 -U --feature force_range -I ./exe
LD				= ld65
C1541			= c1541
CC1541			= cc1541
SED				= sed
PU				= pucrunch
BBMEGA			= b2mega
LC				= crush 6
GCC				= gcc
MC				= MegaConvert
MEGAADDRESS		= megatool -a
MEGACRUNCH		= megatool -c
MEGAIFFL		= megatool -i
MEGAMOD			= MegaMod
EL				= etherload
XMEGA65			= D:\PCTOOLS\xemu\xmega65.exe
MEGAFTP			= mega65_ftp -e
TILEMIZER		= python3 tilemizer.py

CONVERTBREAK	= 's/al [0-9A-F]* \.br_\([a-z]*\)/\0\nbreak \.br_\1/'
CONVERTWATCH	= 's/al [0-9A-F]* \.wh_\([a-z]*\)/\0\nwatch store \.wh_\1/'

CONVERTVICEMAP	= 's/al //'

.SUFFIXES: .o .s .out .bin .pu .b2 .a

default: all

VPATH = src

# Common source files
ASM_SRCS = asm.s decruncher.s iffl.s irqload.s
C_SRCS = main.c

OBJS = $(ASM_SRCS:%.s=$(EXE_DIR)/%.o) $(C_SRCS:%.c=$(EXE_DIR)/%.o)
OBJS_DEBUG = $(ASM_SRCS:%.s=$(EXE_DIR)/%-debug.o) $(C_SRCS:%.c=$(EXE_DIR)/%-debug.o)

BINFILES  = $(BIN_DIR)/glacial_chars0.bin
BINFILES += $(BIN_DIR)/glacial_pal0.bin
BINFILES += $(BIN_DIR)/song.mod

BINFILESMC  = $(BIN_DIR)/glacial_chars0.bin.addr.mc
BINFILESMC += $(BIN_DIR)/glacial_pal0.bin.addr.mc
BINFILESMC += $(BIN_DIR)/song.mod.addr.mc

# -----------------------------------------------------------------------------

$(BIN_DIR)/glacial_chars0.bin: $(BIN_DIR)/glacial.bin
	$(MC) $< cm1:2 d1:0 cl1:20000 rc1:0

$(BIN_DIR)/alldata.bin: $(BINFILES)
	$(MEGAADDRESS) $(BIN_DIR)/glacial_chars0.bin      00020000
	$(MEGAADDRESS) $(BIN_DIR)/glacial_pal0.bin        0000c000
	$(MEGAADDRESS) $(BIN_DIR)/song.mod                00040000
	$(MEGACRUNCH) $(BIN_DIR)/glacial_chars0.bin.addr
	$(MEGACRUNCH) $(BIN_DIR)/glacial_pal0.bin.addr
	$(MEGACRUNCH) $(BIN_DIR)/song.mod.addr	
	$(MEGAIFFL) $(BINFILESMC) $(BIN_DIR)/alldata.bin

$(EXE_DIR)/%.o: %.s
	as6502 --target=mega65 --list-file=$(@:%.o=%.lst) -o $@ $<

$(EXE_DIR)/%.o: %.c
	cc6502 --target=mega65 --core 45gs02 -O2 --list-file=$(@:%.o=%.lst) -o $@ $<

$(EXE_DIR)/%-debug.o: %.s
	as6502 --target=mega65 --debug --list-file=$(@:%.o=%.lst) -o $@ $<

$(EXE_DIR)/%-debug.o: %.c
	cc6502 --target=mega65 --debug --list-file=$(@:%.o=%.lst) -o $@ $<

$(EXE_DIR)/hello.prg: $(OBJS)
	ln6502 --target=mega65 mega65-plain.scm -o $@ $^ --core 45gs02 --output-format=prg --rtattr printf=reduced --rtattr exit=simplified --list-file=$(EXE_DIR)/hello.lst

$(EXE_DIR)/hello.prg.mc: $(EXE_DIR)/hello.prg
	$(MEGACRUNCH) -f 200e $(EXE_DIR)/hello.prg

# -----------------------------------------------------------------------------

$(EXE_DIR)/hello.d81: $(EXE_DIR)/hello.prg.mc  $(BIN_DIR)/alldata.bin
	$(RM) $@
	$(CC1541) -n "hello" -i " 2024" -d 19 -v\
	 \
	 -f "hello" -w $(EXE_DIR)/hello.prg.mc \
	 -f "data" -w $(BIN_DIR)/alldata.bin \
	$@

# -----------------------------------------------------------------------------

run: $(EXE_DIR)/hello.d81

ifeq ($(megabuild), 1)
	$(MEGAFTP) -c "put .\hello.d81 hello.d81" -c "quit"
	$(EL) -m HELLO.D81 -r $(EXE_DIR)/hello.prg.mc
ifeq ($(attachdebugger), 1)
	m65dbg --device /dev/ttyS2
endif
else
ifeq ($(attachdebugger), 1)
	cmd.exe /c "$(XMEGA65) -uartmon :4510 -autoload -8 $(EXE_DIR)/hello.d81" & m65dbg -l tcp 4510
else
	cmd.exe /c "$(XMEGA65) -autoload -8 $(EXE_DIR)/hello.d81"
endif
endif

clean:
	-rm -f $(OBJS) $(OBJS:%.o=%.lst) $(OBJS_DEBUG) $(OBJS_DEBUG:%.o=%.lst)
	-rm -f $(EXE_DIR)/hello.d81 $(EXE_DIR)/hello.elf $(EXE_DIR)/hello.prg $(EXE_DIR)/hello.prg.mc $(EXE_DIR)/hello.lst $(EXE_DIR)/hello-debug.lst
