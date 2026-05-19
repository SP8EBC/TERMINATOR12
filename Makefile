# =====================================================================
# TERMINATOR12 -- top-level Makefile
#
# Most common targets:
#   make deps        # install all dependencies (Arch / yay)
#   make             # build the binary (OGN disabled by default)
#   make WITH_OGN=1  # build with the OGN/Glidernet module
#   make run         # launch the application
#   make clean       # remove build artifacts
#   make distclean   # clean + drop fetched libsvg2 sources
#   make help        # print this help screen
#
# Control variables:
#   WITH_OGN=1       # enables the OGN module (needs protobuf-c)
#   CC=clang         # override the compiler
# =====================================================================

BIN       := TERMINATOR12
BUILD_DIR := build

CC ?= gcc

# enable OGN module with `make WITH_OGN=1`
WITH_OGN ?= 0

# ---------- pkg-config probes ----------
PKG        := pkg-config
PKG_PKGS   := sdl2 SDL2_gfx SDL2_image SDL2_ttf libxml-2.0
PKG_CFLAGS := $(shell $(PKG) --cflags $(PKG_PKGS))
PKG_LIBS   := $(shell $(PKG) --libs   $(PKG_PKGS))

# ---------- includes ----------
INCLUDES := \
	-Isrc \
	-Iconfig \
	-Icompat \
	-Ilib/c-logger/src \
	-Ilib/libsvg2/include \
	-Ilib/libsvg2/include/parser \
	-Ilib/libsvg2/sources \
	-Ilib/libsvg2/sources/parser

# ---------- flags ----------
CFLAGS  ?= -O0 -g3 -Wall -Wextra -fmessage-length=0 -MMD -MP
# GCC 15 promotes a few legacy diagnostics to hard errors. The upstream
# libsvg2 sources use malloc()/free() without #include <stdlib.h>, so we
# downgrade implicit-function-declaration back to a warning.
CFLAGS  += -Wno-error=implicit-function-declaration -Wno-error=implicit-int
CFLAGS  += $(INCLUDES) $(PKG_CFLAGS)
LDLIBS  := $(PKG_LIBS) -lm -lpthread

# ---------- sources ----------
APP_SRCS := $(wildcard src/*.c src/draw/*.c src/heap/*.c src/srtm/*.c)
LOG_SRCS := $(wildcard lib/c-logger/src/*.c)
SVG_SRCS := $(wildcard lib/libsvg2/sources/*.c lib/libsvg2/sources/parser/*.c)

# ---------- OGN module (optional) ----------
ifeq ($(WITH_OGN),1)
    OGN_SRCS   := $(wildcard src/ogn/*.c)
    APP_SRCS   += $(OGN_SRCS)
    OGN_CFLAGS := $(shell $(PKG) --cflags libprotobuf-c)
    OGN_LIBS   := $(shell $(PKG) --libs libprotobuf-c)
    CFLAGS     += -DWITH_OGN=1 -Isrc/ogn $(OGN_CFLAGS)
    LDLIBS     += $(OGN_LIBS)
endif

ALL_SRCS := $(APP_SRCS) $(LOG_SRCS) $(SVG_SRCS)
OBJS     := $(ALL_SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS     := $(OBJS:.o=.d)

# =====================================================================
.PHONY: all deps deps-ogn run clean distclean help protoc
all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)

# ---------------------------------------------------------------------
# One-shot dependency installation (Arch + yay). Also clones submodules
# and fetches the libsvg2 sources from upstream.
# ---------------------------------------------------------------------
deps:
	@echo "==> Installing packages (sdl2, sdl2_gfx, sdl2_image, sdl2_ttf, libxml2, pkgconf)"
	yay -S --needed --noconfirm sdl2-compat sdl2_gfx sdl2_image sdl2_ttf libxml2 pkgconf
	@echo "==> Initializing git submodules (c-logger)"
	git submodule update --init --recursive
	@if [ ! -d lib/libsvg2/sources ]; then \
		echo "==> Fetching libsvg2 sources from upstream"; \
		if [ ! -d lib/libsvg2-upstream ]; then \
			git clone --depth=1 https://github.com/agambier/libsvg2.git lib/libsvg2-upstream; \
		fi; \
		cp -r lib/libsvg2-upstream/sources lib/libsvg2/sources; \
	fi
	@echo "==> Done. Now run: make"

# Extra dependency required only for the OGN module (protobuf-c).
deps-ogn:
	@echo "==> Installing protobuf-c (required for WITH_OGN=1)"
	yay -S --needed --noconfirm protobuf-c
	@echo "==> Done. Now run: make WITH_OGN=1"

# Regenerate the protobuf-c sources from the .proto schema. The
# generated .pb-c.{h,c} are checked in, so end users do not have to
# run this target.
protoc:
	cd src/ogn && protoc-c --c_out=. ogn_messages.proto

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(BUILD_DIR) $(BIN)

distclean: clean
	rm -rf lib/libsvg2/sources lib/libsvg2-upstream

help:
	@echo "TERMINATOR12 -- available make targets:"
	@echo ""
	@echo "  make deps        - one-time install of all dependencies (yay)"
	@echo "  make deps-ogn    - install extra deps required for the OGN module"
	@echo "  make             - build the default image (OGN disabled)"
	@echo "  make WITH_OGN=1  - build with the OGN/Glidernet live APRS module"
	@echo "  make run         - launch the application"
	@echo "  make protoc      - regenerate protobuf-c sources (developers only)"
	@echo "  make clean       - remove build artifacts"
	@echo "  make distclean   - clean + drop fetched libsvg2 sources"
	@echo "  make help        - print this screen"
