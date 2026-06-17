# ============================================================
#  Makefile - Projeto C + raylib
#  - Clona e compila a raylib automaticamente (vendorizada em vendor/raylib)
#  - Busca .c recursivamente em src/ e .h recursivamente em include/
#  - Requer GNU Make (gmake) e git
# ============================================================

# ---------- Nome do executável (usa o nome da pasta do projeto) ----------
TARGET_NAME := $(notdir $(CURDIR))

SRC_DIR   := src
INC_DIR   := include
BUILD_DIR := build
BIN_DIR   := bin

CC   := gcc
CSTD := -std=c11

# ---------- Raylib vendorizada (baixada e compilada por este Makefile) ----------
RAYLIB_VERSION := 6.0
RAYLIB_DIR     := vendor/raylib
RAYLIB_SRC     := $(RAYLIB_DIR)/src
RAYLIB_LIB     := $(RAYLIB_SRC)/libraylib.a
RAYLIB_REPO    := https://github.com/raysan5/raylib.git

# ---------- Detecção de plataforma ----------
ifeq ($(OS),Windows_NT)
    PLATFORM_OS := WINDOWS
    EXE_EXT     := .exe
    LDLIBS      := -lraylib -lopengl32 -lgdi32 -lwinmm
else
    UNAME_S := $(shell uname -s)
    EXE_EXT :=
    ifeq ($(UNAME_S),Darwin)
        PLATFORM_OS := MACOS
        LDLIBS := -lraylib -framework CoreVideo -framework IOKit -framework Cocoa -framework OpenGL
    else
        PLATFORM_OS := LINUX
        LDLIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
endif

TARGET := $(BIN_DIR)/$(TARGET_NAME)$(EXE_EXT)

# ---------- Busca recursiva de arquivos ----------
rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRCS := $(call rwildcard,$(SRC_DIR)/,*.c)
HDRS := $(call rwildcard,$(INC_DIR)/,*.h)

# Todas as subpastas de include/ entram como -I, permitindo
# #include "arquivo.h" de qualquer nível de profundidade.
INC_SUBDIRS := $(sort $(dir $(HDRS)))
INC_FLAGS   := -I$(INC_DIR) $(addprefix -I,$(INC_SUBDIRS)) -I$(RAYLIB_SRC)

OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# ---------- Build Mode configuration ----------
# Default mode is release. Override by passing DEBUG=1
DEBUG ?= 0

ifeq ($(DEBUG), 1)
    # Debug flags: generate debug symbols and disable optimizations
    OPT_FLAGS := -g -O0
else
    # Release flags: optimize for performance
    OPT_FLAGS := -O2
endif

CFLAGS  := $(CSTD) -Wall -Wextra $(OPT_FLAGS) -MMD -MP $(INC_FLAGS)

LDFLAGS := -L$(RAYLIB_SRC)

# ---------- Debug target alias ----------
.PHONY: all raylib run clean distclean rebuild debug

all: $(TARGET)

debug:
	$(MAKE) all DEBUG=1

# ---------- Link final ----------
$(TARGET): $(OBJS) $(RAYLIB_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)
	@echo "Build concluido: $@"

# ---------- Compilação dos .c (espelha a estrutura de src/ em build/) ----------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(RAYLIB_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------- Clona e compila a raylib (só roda se libraylib.a ainda não existir) ----------
$(RAYLIB_LIB):
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		echo "-> Clonando raylib $(RAYLIB_VERSION)..."; \
		git clone --branch $(RAYLIB_VERSION) --depth 1 $(RAYLIB_REPO) $(RAYLIB_DIR); \
	fi
	@echo "-> Compilando raylib para $(PLATFORM_OS)..."
	$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_DESKTOP

raylib: $(RAYLIB_LIB)

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# remove também a raylib vendorizada (reset completo)
distclean: clean
	rm -rf $(RAYLIB_DIR)

rebuild: clean all

-include $(DEPS)