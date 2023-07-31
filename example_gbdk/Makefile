#
# A Makefile that compiles all .c and .s files in "src" and "res"
# subdirectories and places the output in a "obj" subdirectory
#

# If you move this project you can change the directory
# to match your GBDK root directory (ex: GBDK_HOME = "C:/GBDK/"
# GBDK_HOME = ../../../
GBDK_HOME = ~/git/gbdev/gbdk2020/gbdk-2020-git/build/gbdk/

LCC = $(GBDK_HOME)bin/lcc

# You can set flags for LCC here
LCCFLAGS += -debug            # Uncomment to enable debug output
# LCCFLAGS += -v              # Uncomment for lcc verbose output
LCCFLAGS += -Wm-yC            # GB Color required for Hi Color
LCCFLAGS += -Wf-MMD -Wf-Wp-MP # Header file dependency output (-MMD) for Makefile use + per-header Phoney rules (-MP)

# For Hi Color:
PNG2HICOLORGB = ../bin/png2hicolorgb
HICOLOR_FLAGS = --type=1 --csource # Use conversion method 1, C source output
# Add the object dir as an include dir since that's
# where the converted png source files will get generated
LCCFLAGS += -I$(OBJDIR) -I$(SRCDIR)


# You can set the name of the .gb ROM file here
PROJECTNAME = hicolor_pic

BINDIR      = bin
SRCDIR      = src
OBJDIR      = obj
RESDIR      = res
HICOLORDIR  = $(RESDIR)/hicolor
MKDIRS      = $(OBJDIR) $(BINDIR) # See bottom of Makefile for directory auto-creation


BINS	    = $(BINDIR)/$(PROJECTNAME).gb
CSOURCES    = $(foreach dir,$(SRCDIR),$(notdir $(wildcard $(dir)/*.c))) $(foreach dir,$(RESDIR),$(notdir $(wildcard $(dir)/*.c)))
ASMSOURCES  = $(foreach dir,$(SRCDIR),$(notdir $(wildcard $(dir)/*.s)))

# For Hi Color:
# The HICOLORS entries should be first in the OBJS list so they get generated before anything which might depend on them
HICOLORS     = $(foreach dir,$(HICOLORDIR),$(notdir $(wildcard $(dir)/*.png)))
OBJS         = $(HICOLORS:%.png=$(OBJDIR)/%.o) $(CSOURCES:%.c=$(OBJDIR)/%.o) $(ASMSOURCES:%.s=$(OBJDIR)/%.o)

all: $(BINS)

# Dependencies (using output from -Wf-MMD -Wf-Wp-MP)
DEPS = $(OBJS:%.o=%.d)

-include $(DEPS)


# == Start Hi Color conversion ==

# Convert png images in res/hicolor to .c files (which incbin the generated .til/.map/.pal/.atr files)
# The resulting C files will get compiled to object files afterward
.SECONDEXPANSION:
$(OBJDIR)/%.c: $(HICOLORDIR)/%.png
	$(PNG2HICOLORGB) $< $(HICOLOR_FLAGS) -o=$@

# Compile hicolor .c files in "obj/" to .o object files
$(OBJDIR)/%.o:	$(OBJDIR)/%.c
	$(LCC) $(LCCFLAGS) -c -o $@ $<

# == End Hi Color conversion ==


# Compile .c files in "src/" to .o object files
$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	$(LCC) $(LCCFLAGS) -c -o $@ $<

# Compile .c files in "res/" to .o object files
$(OBJDIR)/%.o:	$(RESDIR)/%.c
	$(LCC) $(LCCFLAGS) -c -o $@ $<

# Compile .s assembly files in "src/" to .o object files
$(OBJDIR)/%.o:	$(SRCDIR)/%.s
	$(LCC) $(LCCFLAGS) -c -o $@ $<

# If needed, compile .c files in "src/" to .s assembly files
# (not required if .c is compiled directly to .o)
$(OBJDIR)/%.s:	$(SRCDIR)/%.c
	$(LCC) $(LCCFLAGS) -S -o $@ $<

# Link the compiled object files into a .gb ROM file
$(BINS): $(OBJS)
	$(LCC) $(LCCFLAGS) -o $(BINS) $(OBJS)


clean:
	rm -f  $(OBJDIR)/*.* $(BINDIR)/*.*

compile.bat: Makefile
	@echo "REM Automatically generated from Makefile" > compile.bat
	@make -sn | sed y/\\//\\\\/ | sed s/mkdir\ -p\/mkdir\/ | grep -v make >> compile.bat

# create necessary directories after Makefile is parsed but before build
# info prevents the command from being pasted into the makefile
$(info $(shell mkdir -p $(MKDIRS)))
