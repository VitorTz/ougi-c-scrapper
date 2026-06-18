# ============================================================
#  Makefile - C Project + raylib
#  - Automatically clones and compiles raylib (vendored in vendor/raylib)
#  - Recursively finds .c in src/ and .h in include/
#  - Requires GNU Make and git
# ============================================================

# ---------- Executable name (uses the project folder name) ----------
TARGET_NAME := $(notdir $(CURDIR))

SRC_DIR        := src
INC_DIR        := include
BASE_BUILD_DIR := build
BIN_DIR        := bin

CC   := gcc
CSTD := -std=c11

# ---------- Vendored Raylib (downloaded and compiled by this Makefile) ----------
RAYLIB_VERSION := 6.0
RAYLIB_DIR     := vendor/raylib
RAYLIB_SRC     := $(RAYLIB_DIR)/src
RAYLIB_LIB     := $(RAYLIB_SRC)/libraylib.a
RAYLIB_REPO    := https://github.com/raysan5/raylib.git

# ---------- Platform ----------
EXE_EXT := .out
PLATFORM_OS := LINUX
LDLIBS := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lcurl -lwebp -lwebpdecoder -lpqxx -lpq -pthread

# ---------- Build Mode Configuration ----------
# Default mode is release. Override internally via targets.
MODE ?= release

ifeq ($(MODE),debug)
    OPT_FLAGS   := -g -O0
    TARGET_SUFF := _debug
else
    OPT_FLAGS   := -O2
    TARGET_SUFF := 
endif

# Isolate build artifacts and binary names based on mode
BUILD_DIR := $(BASE_BUILD_DIR)/$(MODE)
TARGET    := $(BIN_DIR)/$(TARGET_NAME)$(TARGET_SUFF)$(EXE_EXT)

# ---------- Recursive file search ----------
rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRCS := $(call rwildcard,$(SRC_DIR)/,*.c)
HDRS := $(call rwildcard,$(INC_DIR)/,*.h)

# All subfolders in include/ are added as -I
INC_SUBDIRS := $(sort $(dir $(HDRS)))
INC_FLAGS   := -I$(INC_DIR) $(addprefix -I,$(INC_SUBDIRS)) -I$(RAYLIB_SRC)

# Output objects inside the specific build mode folder
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

CFLAGS  := $(CSTD) -Wall -Wextra $(OPT_FLAGS) -MMD -MP $(INC_FLAGS)
LDFLAGS := -L$(RAYLIB_SRC)

# ---------- Targets ----------
.PHONY: all release debug build_target raylib run run_debug clean distclean rebuild

# Default target
all: release

# Explicit targets for modes (delegates to build_target with MODE override)
release:
	@$(MAKE) --no-print-directory build_target MODE=release

debug:
	@$(MAKE) --no-print-directory build_target MODE=debug

# The actual target that builds the binary
build_target: $(TARGET)

# ---------- Final Link ----------
$(TARGET): $(OBJS) $(RAYLIB_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)
	@echo "Build complete [$(MODE)]: $@"

# ---------- Compile .c files ----------
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(RAYLIB_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------- Clone and compile raylib ----------
$(RAYLIB_LIB):
	@if [ ! -d "$(RAYLIB_DIR)" ]; then \
		echo "-> Cloning raylib $(RAYLIB_VERSION)..."; \
		git clone --branch $(RAYLIB_VERSION) --depth 1 $(RAYLIB_REPO) $(RAYLIB_DIR); \
	fi
	@echo "-> Compiling raylib for $(PLATFORM_OS)..."
	$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_DESKTOP

# ---------- Utilities ----------
run: release
	./$(BIN_DIR)/$(TARGET_NAME)$(EXE_EXT)

run_debug: debug
	./$(BIN_DIR)/$(TARGET_NAME)_debug$(EXE_EXT)

clean:
	rm -rf $(BASE_BUILD_DIR) $(BIN_DIR)

distclean: clean
	rm -rf $(RAYLIB_DIR)

rebuild: clean all

-include $(DEPS)