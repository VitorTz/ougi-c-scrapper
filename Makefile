# ============================================================
#  Makefile - C Project + raylib
#  - Automatically clones and compiles raylib (vendored in vendor/raylib)
#  - Recursively finds .c in src/ and .h in include/
#  - Supports custom entry points from script/ and test/ directories
#  - Requires GNU Make and git
# ============================================================

# ---------- Executable name (uses the project folder name by default) ----------
TARGET_NAME := $(notdir $(CURDIR))

SRC_DIR        := src
INC_DIR        := include
BASE_BUILD_DIR := build
BIN_DIR        := bin

CC   := gcc
CSTD := -std=c11

# ---------- Vendored Raylib ----------
RAYLIB_VERSION := 6.0
RAYLIB_DIR     := vendor/raylib
RAYLIB_SRC     := $(RAYLIB_DIR)/src
RAYLIB_LIB     := $(RAYLIB_SRC)/libraylib.a
RAYLIB_REPO    := https://github.com/raysan5/raylib.git

# ---------- Platform ----------
EXE_EXT := .out
PLATFORM_OS := LINUX
LDLIBS := -lraylib -lGL -lm -ldl -lrt -lX11 -lcurl -lwebp -lwebpdecoder -lpqxx -lpq -pthread

# ---------- Recursive file search ----------
rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRCS := $(call rwildcard,$(SRC_DIR)/,*.c)
HDRS := $(call rwildcard,$(INC_DIR)/,*.h)

# ---------- Custom Entry Point Logic ----------
# Detect if the user passed a script/xxx or test/xxx goal
CUSTOM_ENTRY := $(filter script/% test/%, $(MAKECMDGOALS))

ifneq ($(CUSTOM_ENTRY),)
    # Extract the first matching entry (e.g., script/my_file) and append .c
    ENTRY_BASE := $(word 1, $(CUSTOM_ENTRY))
    ENTRY_FILE := $(ENTRY_BASE).c

    # Remove the default main.c from the compilation list
    SRCS := $(filter-out $(SRC_DIR)/main.c, $(SRCS))

    # Add the target script/test file to the compilation list
    SRCS += $(ENTRY_FILE)

    # Override target name to match the script/test filename
    TARGET_NAME := $(notdir $(ENTRY_BASE))

    # Dummy rule to prevent Make from throwing "No rule to make target" for the folder path
$(CUSTOM_ENTRY):
	@:
endif

# ---------- Build Mode Configuration ----------
MODE ?= release

# Detect if 'debug' or 'release' were explicitly passed in the command line
ifneq ($(filter debug,$(MAKECMDGOALS)),)
    MODE := debug
endif
ifneq ($(filter release,$(MAKECMDGOALS)),)
    MODE := release
endif

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

# All subfolders in include/ are added as -I
INC_SUBDIRS := $(sort $(dir $(HDRS)))
INC_FLAGS   := -I$(INC_DIR) $(addprefix -I,$(INC_SUBDIRS)) -I$(RAYLIB_SRC)

# Output objects map the relative source path (e.g., build/release/script/file.o)
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

CFLAGS  := $(CSTD) -Wall -Wextra $(OPT_FLAGS) -MMD -MP -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE $(INC_FLAGS)
LDFLAGS := -L$(RAYLIB_SRC)

# ---------- Targets ----------
.PHONY: all release debug build_target raylib run clean distclean rebuild

# Default target
all: release

# Explicit mode targets act as aliases to the main build step
release: build_target
debug: build_target

# The actual target that builds the binary
build_target: $(TARGET)

# ---------- Final Link ----------
$(TARGET): $(OBJS) $(RAYLIB_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)
	@echo "Build complete [$(MODE)]: $@"

# ---------- Compile .c files ----------
# Using %.c allows tracking files across src/, script/, and test/ folders smoothly
$(BUILD_DIR)/%.o: %.c | $(RAYLIB_LIB)
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
# Dynamically runs the exact target that was just built
run: build_target
	./$(TARGET)

clean:
	rm -rf $(BASE_BUILD_DIR) $(BIN_DIR)

distclean: clean
	rm -rf $(RAYLIB_DIR)

rebuild: clean all

-include $(DEPS)