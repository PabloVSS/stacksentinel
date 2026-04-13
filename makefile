# =========================
# CONFIG
# =========================

CC      = gcc
CFLAGS = -Wall -Wextra -g -fno-omit-frame-pointer -Iinclude
SRC_DIR = src
EX_DIR  = examples
BUILD   = build
BIN     = $(BUILD)/bin

TARGET      = $(BIN)/sentinel
VULN_BIN    = $(BIN)/vulnerable
SAFE_BIN    = $(BIN)/safe

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# =========================
# DEFAULT TARGET
# =========================

all: build_dirs $(TARGET) examples

# =========================
# BUILD CORE
# =========================

$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) $^ -o $@

# =========================
# BUILD EXAMPLES
# =========================

examples: $(VULN_BIN) $(SAFE_BIN)

$(VULN_BIN): $(EX_DIR)/vulnerable.c
	$(CC) $(CFLAGS) $< -o $@

$(SAFE_BIN): $(EX_DIR)/safe.c
	$(CC) $(CFLAGS) $< -o $@

# =========================
# RUN
# =========================

run: all
	@echo "Running StackSentinel..."
	@$(TARGET)

demo: all
	@chmod +x scripts/run_demo.sh
	@./scripts/run_demo.sh

# =========================
# DEBUG / DISASSEMBLY
# =========================

debug: all
	gdb $(TARGET)

disasm: all
	objdump -d $(TARGET) | less

# =========================
# TEST INPUT (rápido)
# =========================

test-vuln: $(VULN_BIN)
	@echo "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" | $(VULN_BIN)

test-safe: $(SAFE_BIN)
	@echo "safe input" | $(SAFE_BIN)

# =========================
# CLEAN
# =========================

clean:
	rm -rf $(BUILD)

# =========================
# DIRECTORIES
# =========================

build_dirs:
	mkdir -p $(BIN)

# =========================
# PHONY
# =========================

.PHONY: all clean run demo debug disasm examples build_dirs test-vuln test-safe