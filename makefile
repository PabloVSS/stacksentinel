# ==============================================================================
# StackSentinel Makefile - Optimized for Forensic Analysis & Portfolio
# Autor: Pablo Vinícius Sousa Silva
# ==============================================================================

CC = gcc

# Cores para feedback visual no terminal (ANSI)
CYAN  = \033[0;36m
GREEN = \033[0;32m
RED   = \033[0;31m
YELLOW = \033[0;33m
BOLD  = \033[1m
NC    = \033[0m # No Color

# Configurações de Compilação
# -fno-omit-frame-pointer: Essencial para o unwinding manual via RBP
# -no-pie: Facilita a leitura de endereços fixos em demonstrações
CFLAGS_COMMON = -Wall -Wextra -g -Iinclude
CFLAGS_DEBUG  = -fno-omit-frame-pointer -no-pie
CFLAGS        = $(CFLAGS_COMMON) $(CFLAGS_DEBUG)
LDFLAGS       = -ldl

# Diretórios
SRC_DIR = src
EX_DIR  = examples
BUILD   = build
BIN     = $(BUILD)/bin

# Alvos (Binários)
TARGET      = $(BIN)/sentinel
VULN_BIN    = $(BIN)/vulnerable
SAFE_BIN    = $(BIN)/safe
UNSAFE_BIN  = $(BIN)/vulnerable_unsafe

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# ==============================================================================
# REGRAS DE CONSTRUÇÃO
# ==============================================================================

all: build_dirs $(TARGET) examples

$(TARGET): $(SRC_FILES)
	@echo -e "$(CYAN)[BUILD]$(NC) StackSentinel Core"
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

examples: $(VULN_BIN) $(SAFE_BIN) $(UNSAFE_BIN)

$(VULN_BIN): $(EX_DIR)/vulnerable.c
	@echo -e "$(CYAN)[BUILD]$(NC) Vulnerable (Protected)"
	@$(CC) $(CFLAGS) $< -o $@

$(SAFE_BIN): $(EX_DIR)/safe.c
	@echo -e "$(CYAN)[BUILD]$(NC) Safe Execution Example"
	@$(CC) $(CFLAGS) $< -o $@

$(UNSAFE_BIN): $(EX_DIR)/vulnerable.c
	@echo -e "$(CYAN)[BUILD]$(NC) $(RED)Vulnerable (UNPROTECTED)$(NC)"
	@$(CC) $(CFLAGS) -fno-stack-protector -z execstack $< -o $@
	@echo -e "$(RED)[!] Warning: Built $(notdir $@) sem proteções de stack (Canary/NX)$(NC)"

# ==============================================================================
# PIPELINE DE AUDITORIA AUTOMATIZADA
# ==============================================================================

# Roda todos os cenários possíveis e explica as soluções
test-full: all vuln-unprotected
	@echo -e "\n$(BOLD)============================================================$(NC)"
	@echo -e "$(BOLD)      STACKSENTINEL: RELATÓRIO DE AUDITORIA DE MEMÓRIA      $(NC)"
	@echo -e "$(BOLD)============================================================$(NC)"

	@echo -e "\n$(GREEN)[CENÁRIO 1] Execução Nominal (Segura)$(NC)"
	@echo -e "Descrição: Entrada dentro dos limites. Nenhuma corrupção detectada."
	@echo "pablo" | $(SAFE_BIN)
	@echo -e "$(GREEN)>> Resultado: Memória íntegra. Entrada sanitizada.$(NC)"

	@echo -e "\n$(RED)[CENÁRIO 2] Vulnerabilidade de Overflow (Padrão 0x41)$(NC)"
	@echo -e "Descrição: Injeção de 128 'A's em buffer de 16 bytes."
	@python3 -c "print('A' * 128)" | $(UNSAFE_BIN) || \
	echo -e "$(YELLOW)[!] Alerta de Kernel: SIGSEGV Detectado (Stack Smashing)$(NC)"
	@echo -e "\n$(CYAN)>> DISPARANDO ANÁLISE FORENSE (Sentinel):$(NC)"
	@$(TARGET)
	@echo -e "$(GREEN)>> Solução Sugerida: Validar tamanho de input (fgets/strncpy).$(NC)"

	@echo -e "\n$(RED)[CENÁRIO 3] Estouro de Larga Escala (300 Bytes)$(NC)"
	@echo -e "Descrição: Sobrescrita total de RBP e RIP com padrão 'B'."
	@python3 -c "print('B' * 300)" | $(UNSAFE_BIN) || \
	echo -e "$(YELLOW)[!] Alerta de Kernel: SIGSEGV Detectado$(NC)"
	@echo -e "\n$(CYAN)>> DISPARANDO ANÁLISE FORENSE (Sentinel):$(NC)"
	@$(TARGET)
	@echo -e "$(GREEN)>> Solução Sugerida: Habilitar -fstack-protector-all no GCC.$(NC)"

	@echo -e "\n$(BOLD)============================================================$(NC)"
	@echo -e "$(BOLD)             FIM DA AUDITORIA AUTOMATIZADA                  $(NC)"
	@echo -e "$(BOLD)============================================================$(NC)\n"

# ==============================================================================
# TESTES UNITÁRIOS / INDIVIDUAIS
# ==============================================================================

test-analysis: vuln-unprotected $(TARGET)
	@echo -e "\n$(BOLD)>>> SIMULANDO ATAQUE DE 300 BYTES...$(NC)"
	@python3 -c "print('A' * 300)" | $(UNSAFE_BIN) || \
	echo -e "$(RED)[!] STATUS: Binário interrompido por SIGSEGV$(NC)"
	@echo -e "\n$(BOLD)>>> EXECUTANDO INTROSPECÇÃO FORENSE...$(NC)"
	@$(TARGET)

test-safe: $(SAFE_BIN)
	@echo -e "\n$(GREEN)[TEST]$(NC) Safe execution"
	@echo "safe input" | $(SAFE_BIN)

# ==============================================================================
# UTILITÁRIOS
# ==============================================================================

clean:
	@echo -e "$(RED)[CLEAN]$(NC) Removing build directory"
	@rm -rf $(BUILD)

build_dirs:
	@mkdir -p $(BIN)

vuln-unprotected: build_dirs $(UNSAFE_BIN)

.PHONY: all clean examples build_dirs vuln-unprotected test-analysis test-full-audit test-safe