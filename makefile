# ==============================================================================
# StackSentinel Makefile - Optimized for Forensic Analysis & Portfolio
# Autor: Pablo Vinícius Sousa Silva
# ==============================================================================

CC = gcc

# Cores ANSI
CYAN  = \033[0;36m
GREEN = \033[0;32m
RED   = \033[0;31m
YELLOW= \033[0;33m
BOLD  = \033[1m
NC    = \033[0m

# Flags
CFLAGS_COMMON = -Wall -Wextra -g -Iinclude
CFLAGS_DEBUG  = -fno-omit-frame-pointer -no-pie
CFLAGS        = $(CFLAGS_COMMON) $(CFLAGS_DEBUG)

# Flags opcionais (debug avançado)
CFLAGS_SEC    = -fstack-protector-all -D_FORTIFY_SOURCE=2
LDFLAGS       = -ldl

# Diretórios
SRC_DIR = src
EX_DIR  = examples
BUILD   = build
BIN     = $(BUILD)/bin

# Binários
TARGET      = $(BIN)/sentinel
VULN_BIN    = $(BIN)/vulnerable
SAFE_BIN    = $(BIN)/safe
UNSAFE_BIN  = $(BIN)/vulnerable_unsafe

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# ==============================================================================
# BUILD
# ==============================================================================

all: build_dirs $(TARGET) examples

$(TARGET): $(SRC_FILES) | build_dirs
	@printf "$(CYAN)[BUILD]$(NC) StackSentinel Core\n"
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

examples: $(VULN_BIN) $(SAFE_BIN) $(UNSAFE_BIN)

$(VULN_BIN): $(EX_DIR)/vulnerable.c | build_dirs
	@printf "$(CYAN)[BUILD]$(NC) Vulnerable (Protected)\n"
	@$(CC) $(CFLAGS) $(CFLAGS_SEC) $< -o $@

$(SAFE_BIN): $(EX_DIR)/safe.c | build_dirs
	@printf "$(CYAN)[BUILD]$(NC) Safe Execution Example\n"
	@$(CC) $(CFLAGS) $< -o $@

$(UNSAFE_BIN): $(EX_DIR)/vulnerable.c | build_dirs
	@printf "$(CYAN)[BUILD]$(NC) $(RED)Vulnerable (UNPROTECTED)$(NC)\n"
	@$(CC) $(CFLAGS) -fno-stack-protector -z execstack $< -o $@
	@printf "$(RED)[!] Warning: Built %s sem proteções de stack (Canary/NX)$(NC)\n" "$(notdir $@)"

# ==============================================================================
# PIPELINE DE AUDITORIA
# ==============================================================================

test-full: all vuln-unprotected
	@printf "\n$(BOLD)============================================================$(NC)\n"
	@printf "$(BOLD)      STACKSENTINEL: RELATÓRIO DE AUDITORIA DE MEMÓRIA      $(NC)\n"
	@printf "$(BOLD)============================================================$(NC)\n"

	@printf "\n$(GREEN)[CENÁRIO 1] Execução Nominal (Segura)$(NC)\n"
	@printf "Descrição: Entrada dentro dos limites. Nenhuma corrupção detectada.\n"
	@echo "pablo" | $(SAFE_BIN)
	@printf "$(GREEN)>> Resultado: Memória íntegra. Entrada sanitizada.$(NC)\n"

	@printf "\n$(RED)[CENÁRIO 2] Vulnerabilidade de Overflow (Padrão 0x41)$(NC)\n"
	@printf "Descrição: Injeção de 128 'A's em buffer de 16 bytes.\n"
	@python3 -c "print('A' * 128)" | $(UNSAFE_BIN) || \
	printf "$(YELLOW)[!] SIGSEGV detectado (Stack Smashing)$(NC)\n"

	@printf "\n$(CYAN)>> DISPARANDO ANÁLISE FORENSE (Sentinel):$(NC)\n"
	@$(TARGET)

	@printf "$(GREEN)>> Solução: Validar tamanho de input (fgets/strncpy).$(NC)\n"

	@printf "\n$(RED)[CENÁRIO 3] Estouro de Larga Escala (300 Bytes)$(NC)\n"
	@printf "Descrição: Sobrescrita massiva da stack com padrão 'B'.\n"
	@python3 -c "print('B' * 300)" | $(UNSAFE_BIN) || \
	printf "$(YELLOW)[!] SIGSEGV detectado$(NC)\n"

	@printf "\n$(CYAN)>> DISPARANDO ANÁLISE FORENSE (Sentinel):$(NC)\n"
	@$(TARGET)

	@printf "$(GREEN)>> Solução: Habilitar -fstack-protector-all.$(NC)\n"

	@printf "\n$(BOLD)============================================================$(NC)\n"
	@printf "$(BOLD)             FIM DA AUDITORIA AUTOMATIZADA                  $(NC)\n"
	@printf "$(BOLD)============================================================$(NC)\n"

# ==============================================================================
# TESTES
# ==============================================================================

test-analysis: vuln-unprotected $(TARGET)
	@printf "\n$(BOLD)>>> SIMULANDO ATAQUE DE 300 BYTES...$(NC)\n"
	@python3 -c "print('A' * 300)" | $(UNSAFE_BIN) || \
	printf "$(RED)[!] STATUS: SIGSEGV$(NC)\n"

	@printf "\n$(BOLD)>>> EXECUTANDO INTROSPECÇÃO...$(NC)\n"
	@$(TARGET)

test-safe: $(SAFE_BIN)
	@printf "\n$(GREEN)[TEST]$(NC) Safe execution\n"
	@echo "safe input" | $(SAFE_BIN)

# ==============================================================================
# GDB ANALYSIS - FORÇA OVERFLOW REAL + INSPEÇÃO DE STACK
# ==============================================================================
gdb-vuln: $(UNSAFE_BIN)
	@printf "$(YELLOW)[GDB] Execução com controle real de overflow$(NC)\n"

	@python3 -c "print('A'*600)" > /tmp/payload.txt

	@gdb -q $(UNSAFE_BIN) \
		-ex "set pagination off" \
		-ex "set breakpoint pending on" \
		-ex "break vulnerable_function" \
		-ex "break *vulnerable_function+40" \
		-ex "run < /tmp/payload.txt" \
		-ex "printf \"\n>>> [ANTES DO INPUT]\n\"" \
		-ex "info frame" \
		-ex "info registers" \
		-ex "x/32gx \$$rsp" \
		-ex "continue" \
		-ex "printf \"\n>>> [APÓS gets() - STACK AO VIVO]\n\"" \
		-ex "info registers" \
		-ex "bt" \
		-ex "x/64gx \$$rsp" \
		-ex "quit"
		
# ==============================================================================
# UTIL
# ==============================================================================

clean:
	@printf "$(RED)[CLEAN]$(NC) Removing build directory\n"
	@rm -rf $(BUILD)

build_dirs:
	@mkdir -p $(BIN)

vuln-unprotected: build_dirs $(UNSAFE_BIN)

.PHONY: all clean examples build_dirs vuln-unprotected \
        test-analysis test-full test-safe gdb-vuln