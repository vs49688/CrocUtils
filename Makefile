#!/usr/bin/make -f

LINCC?=gcc
LINCXX?=g++

WINCC?=x86_64-w64-mingw32-gcc
WINCXX?=x86_64-w64-mingw32-g++

CFLAGS?=-O3 -flto
LDFLAGS?=-static

BUILDROOT?=build

VERSION?=$(shell git describe)

all: $(BUILDROOT)/maptool-linux-amd64-$(VERSION) $(BUILDROOT)/maptool-windows-amd64-$(VERSION).exe

$(BUILDROOT)/linux-amd64/maptool/maptool:
	mkdir -p $(BUILDROOT)/linux-amd64
	cmake \
		-B $(BUILDROOT)/linux-amd64 \
		-G "Unix Makefiles" \
		-DCMAKE_C_COMPILER=$(LINCC) \
		-DCMAKE_CXX_COMPILER=$(LINCXX) \
		-DCMAKE_C_FLAGS="$(CFLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(LDFLAGS)" \
		.
	$(MAKE) -C $(BUILDROOT)/linux-amd64 maptool

$(BUILDROOT)/windows-amd64/maptool/maptool.exe:
	mkdir -p $(BUILDROOT)/windows-amd64
	cmake \
		-B $(BUILDROOT)/windows-amd64 \
		-G "Unix Makefiles" \
		-DCMAKE_C_COMPILER=$(WINCC) \
		-DCMAKE_CXX_COMPILER=$(WINCXX) \
		-DCMAKE_C_FLAGS="$(CFLAGS)" \
		-DCMAKE_EXE_LINKER_FLAGS="$(LDFLAGS)" \
		-DCMAKE_SYSTEM_NAME="Windows" \
		.
	$(MAKE) -C $(BUILDROOT)/windows-amd64 maptool

$(BUILDROOT)/maptool-linux-amd64-$(VERSION): $(BUILDROOT)/linux-amd64/maptool/maptool
	cp $< $@

$(BUILDROOT)/maptool-windows-amd64-$(VERSION).exe: $(BUILDROOT)/windows-amd64/maptool/maptool.exe
	cp $< $@

.PHONY: all
