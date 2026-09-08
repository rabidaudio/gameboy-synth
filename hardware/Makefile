#
# Simple Makefile that compiles all .c and .s files in the same folder
#

# If you move this project you can change the directory 
# to match your GBDK root directory (ex: GBDK_HOME = "C:/GBDK/"
ifndef GBDK_HOME
	GBDK_HOME = "$(GBDIR)/gbdk"
endif

LCC = $(GBDK_HOME)/bin/lcc
GCC = /usr/bin/gcc

LCCFLAGS += -std-c11

LCCFLAGS += -DTARGET_GBDK

GBDK_DEBUG = ON
ifdef GBDK_DEBUG
	LCCFLAGS += -debug -v
else
	LCCFLAGS += -O3
endif


# You can set the name of the .gb ROM file here
PROJECTNAME    = gbhwsynth

BUILD_DIR = build
BINS	    = $(PROJECTNAME).gb
CSOURCES   := src/main.c $(wildcard src/synth/*.c)
ASMSOURCES := $(wildcard src/synth/*.s)

TESTFLAGS = -shared -v -DPYTEST

all:	$(BINS)

# Compile and link all source files in a single call to LCC
$(BINS):	assets $(CSOURCES) $(ASMSOURCES)
	$(LCC) $(LCCFLAGS) -o build/$@ $(CSOURCES) $(ASMSOURCES)

assets:
	png2asset assets/font_10.png -o src/assets/font.c -spr8x8

testobj:
	$(GCC) $(TESTFLAGS) -o build/test/synth.so -fPIC src/synth/synth.c

test: testobj
	.venv/bin/pytest --rootdir tests tests/*

clean:
	rm -rf build/* src/assets/*

run:
	emulicious build/gbhwsynth.gb
