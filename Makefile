
DEL = rm -f
SRCDIR = src
OBJDIR = obj
BINDIR = bin

# Extra source dirs (these should be under SRCDIR)
LODEPNGDIR = $(SRCDIR)/lodepng
HICOLORDIR = $(SRCDIR)/hicolor
#
EXTRA_SRCDIRS = $(LODEPNGDIR) $(HICOLORDIR)
EXTRA_OBJDIRS = $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(EXTRA_SRCDIRS))
EXTRA_INCDIRS = $(patsubst $(SRCDIR)/%,-I"$(OBJDIR)/%",$(EXTRA_SRCDIRS))

# Add all c source files from $(SRCDIR)
# Create the object files in $(OBJDIR)
CFILES = $(wildcard $(SRCDIR)/*.c) \
         $(wildcard $(LODEPNGDIR)/*.c) \
         $(wildcard $(HICOLORDIR)/*.c)

PACKDIR = package
MKDIRS = $(OBJDIR) $(BINDIR) $(PACKDIR) $(EXTRA_OBJDIRS)

INCS = -I"$(SRCDIR)" -I"$(LODEPNGDIR)" -I"$(HICOLORDIR)"
CFLAGS = $(INCS) -Wno-format-truncation
CFLAGS += -MMD -MP



# Optional: Remove some files that won't/can't be used
# CEXCLUDE = somefile.c anotherfile.c
# CFILES = $(filter-out $(CEXCLUDE),$(CALL))


COBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(CFILES))

ifdef DRAG_AND_DROP_MODE
	EXTRA_FNAME = _drag_and_drop
	CFLAGS+= -DDRAG_AND_DROP_MODE
endif

BIN_NAME = png2hicolorgb
BIN = $(BINDIR)/$(BIN_NAME)$(EXTRA_FNAME)
BIN_WIN = $(BIN).exe


# ignore package dicrectory that conflicts with rule target
.PHONY: package

all: linux

# Compile .c to .o in a separate directory
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


# Linux MinGW build for Windows
# optional -lm for math.h
# (static linking to avoid DLL dependencies)
wincross: TARGET=i686-w64-mingw32
wincross: CC = $(TARGET)-g++
wincross: LDFLAGS = -s -static #lm
wincross: $(COBJ)
	$(CC) -o $(BIN_WIN)  $^ $(LDFLAGS)

# Maxos uses linux target
macos: linux

# Linux build
# optional -lm for math.h
linux: CC = gcc
linux: LDFLAGS = -s #lm
linux: $(COBJ)
	$(CC) $(CFLAGS) -o $(BIN) $^ $(LDFLAGS)

cleanobj:
	$(DEL) $(COBJ) $(DEPS)

clean:
	$(DEL) $(COBJ) $(BIN_WIN) $(BIN) $(DEPS)

cleanexamples:
	$(DEL) example_gbdk/bin/* example_gbdk/obj/*
	$(DEL) example_rgbds/bin/* example_rgbds/obj/*

macoszip: macos cleanexamples
	strip $(BIN)
	# -j discards (junks) path to file
	zip -j $(BIN)-macos.zip $(BIN)
	zip -r $(BIN)-macos.zip example_gbdk example_rgbds
	mv $(BIN)-macos.zip $(PACKDIR)

linuxzip: linux cleanexamples
	strip $(BIN)
	# -j discards (junks) path to file
	zip -j $(BIN)-linux.zip $(BIN)
	zip -r $(BIN)-linux.zip example_gbdk example_rgbds
	mv $(BIN)-linux.zip $(PACKDIR)

wincrosszip: wincross cleanexamples
	strip $(BIN_WIN)
	# -j discards (junks) path to file
	zip -j $(BIN)-windows.zip $(BIN_WIN)
	zip -r $(BIN)-windows.zip example_gbdk example_rgbds
	mv $(BIN)-windows.zip $(PACKDIR)

package:
	${MAKE} clean
	${MAKE} wincrosszip
	${MAKE} clean
	${MAKE} wincrosszip DRAG_AND_DROP_MODE=TRUE
	${MAKE} clean
	${MAKE} linuxzip

# For -MMD and -MP
# Dependencies
DEPS = $(COBJ:%.o=%.d)
-include $(DEPS)


# create necessary directories after Makefile is parsed but before build
# info prevents the command from being pasted into the makefile
$(info $(shell mkdir -p $(MKDIRS)))

