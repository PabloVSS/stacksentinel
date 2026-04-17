# StackSentinel — Ferramenta de Introspecção de Stack

## Visão Geral


É uma ferramenta de análise em tempo de execução, desenvolvida em linguagem C, que realiza uma introspecção manual da stack utilizando o encadeamento do frame pointer (RBP).

O objetivo do projeto é mostrar como as chamadas de função são organizadas na memória durante a execução, fornecendo visibilidade direta sobre o comportamento real do programa em nível de baixo nível.

Esse projeto busca reduzir a distância entre o código de alto nível e a execução efetiva no nível de máquina.


## Problema

Esse programa foi pensado a partir de como as ferramentas modernas criam uma camada de conforto que esconde detalhes brutos do hardware para facilitar a vida do desenvolvedor.

Isso limita a capacidade de:

- Analisar o fluxo de execução com precisão  
- Identificar comportamentos anômalos  
- Entender o papel dos endereços de retorno  
- Investigar problemas de baixo nível  


## Solução

Para resolver isso, utilizei o registrador RBP para reconstruir a pilha de execução em tempo real, realizando:

- Stack Walking manual via RBP  
- Classificação de memória via `/proc/self/maps`  
- Validação estrutural de frames  
- Detecção de anomalias estruturais  

## Estrutura

Pastas

```
stack-sentinel/ 
├── src/ │ 
    ├── stack.c │ 
    ├── analyzer.c │ 
    ├── memory_map.c │ 
    ├── symbols.c │ 
    └── main.c 
├── include/ 
├── examples/ 
├── scripts/ 
├── build/ 
└── Makefile
```


Fluxo de execução

```
main 
└── trigger_trace 
    └── walk_stack 
        ├── analyze_frame 
        └── resolve_symbol
```

Os arquivos criados:

    -   stack.c -> Percorre a stack via RBP
    -   analyzer.c -> Validação semântica e detecção de anomalias
    -   symbols.c -> Resolve endereços para nomes de função
    -   memory_map.c -> Leitura e parsing de /proc/self/maps
    -   main.c -> Ponto de Entrada

## Demonstração Experimental

Foi criado no Makefile um modelo de teste que acompanha todos os cenários previstos nesse projeto. Para executá-lo, basta rodar:

*make test-full*

Nos cenários, teremos o **STACK INTROSPECTION**, e para entender o cabeçalho, a explicação está abaixo.

O cabeçalho do `[STACK INTROSPECTION]` significa:

    - ID: A profundidade da chamada (0 é onde travou, 1 é quem chamou o 0, e assim por diante)  
    - ADDRESS: O local exato na memória RAM  
    - SYMBOL (??): O `??` quer dizer que o Sentinel não encontrou o nome da função, geralmente porque o estado da memória foi corrompido pelo overflow  
    - REGION [EXEC]: Indica que o endereço está em uma região executável da memória  

---

### Cenário 1 - Execução Segura

```
Descrição: Entrada dentro dos limites. Nenhuma corrupção detectada.
[Safe] Starting program...
[Safe] Enter input:  [Safe] You entered: pablo
>> Resultado: Memória íntegra. Entrada sanitizada. 
```

Resultado:
    -   Stack integridade
    -   Fluxo de execução preservado
    -   Nenhuma anomalia detectada

### Cenario 2 - Vulnerabilidade de Overflow (Padrão 0x41) 

```
[CENÁRIO 2] Vulnerabilidade de Overflow (Padrão 0x41) 
Descrição: Injeção de 128 'A's em buffer de 16 bytes.
[Vulnerable] Starting program...
[Vunerable] Enter unput:  [Vunerable] You entered: AAAAAAAAAAAAAAA

>> DISPARANDO ANÁLISE FORENSE (Sentinel): 
Enter input: asadddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddsadsaaaaa

[STACK INTROSPECTION]
ID   ADDRESS            SYMBOL               MODULE          REGION    
--------------------------------------------------------------------------------
#0   0x401314           ??                   build/bin/senti [EXEC]
  [!] WARNING: Unaligned address (ABI violation)
 [EXEC] (/home/pablo/Documentos/Portfolio/low_level/C/stacksentinel/build/bin/sentinel)
#1   0x401364           ??                   build/bin/senti [EXEC]
  [!] WARNING: Unaligned address (ABI violation)
 [EXEC] (/home/pablo/Documentos/Portfolio/low_level/C/stacksentinel/build/bin/sentinel)
#2   0x7f8452827c4e     ??                   /usr/lib/libc.s [LIB]
  [!] WARNING: Unaligned address (ABI violation)
 [EXEC] (/usr/lib/libc.so.6)
#3   [STOP] NULL return address reached

```
Neste cenário, o programa reservou 16 bytes na memória para a variável de entrada, mas foi feito um input com 128 caracteres 'A'.

Padrão 0x41: é o valor hexadecimal do caractere 'A'

O caminho do ataque foi:

    -   Frames 0 e 1: execução ainda dentro do binário (build/bin/sentinel)
    -   Frame 2: o fluxo já alcança a libc, indicando que o retorno da função foi afetado

O fim da linha #3 [STOP] NULL return address reached mostra que:

    -   A CPU tentou ler o próximo endereço de retorno
    -   Em vez de encontrar um endereço válido, encontrou um valor inválido ou NULL
    -   O Sentinel interrompe a análise para evitar comportamento indefinido
---

### Cenario 3 - Estouro de Larga Escala (300 Bytes) 

```
[CENÁRIO 3] Estouro de Larga Escala (300 Bytes) 
Descrição: Sobrescrita total de RBP e RIP com padrão 'B'.
[Vulnerable] Starting program...
[Vunerable] Enter unput:  [Vunerable] You entered: BBBBBBBBBBBBBBB

>> DISPARANDO ANÁLISE FORENSE (Sentinel): 
Enter input: saaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaad

[STACK INTROSPECTION]
ID   ADDRESS            SYMBOL               MODULE          REGION    
--------------------------------------------------------------------------------
#0   0x401314           ??                   build/bin/senti [EXEC]
  [!] WARNING: Unaligned address (ABI violation)
 [EXEC] (/home/pablo/Documentos/Portfolio/low_level/C/stacksentinel/build/bin/sentinel)
#1   0x401364           ??                   build/bin/senti [EXEC]
  [!] WARNING: Unaligned address (ABI violation)
 [EXEC] (/home/pablo/Documentos/Portfolio/low_level/C/stacksentinel/build/bin/sentinel)
#2   0x7ff0fac27c4e     ??                   /usr/lib/libc.s [LIB]
  [!] WARNING: Unaligned address (ABI violation)
 [EXEC] (/usr/lib/libc.so.6)
#3   [STOP] NULL return address reached
--------------------------------------------------------------------------------


```

Neste cenário temos um caso mais crítico que o cenário 2.

A entrada de 300 bytes sobrescreve completamente:

    -   RBP → perda da referência dos frames
    -   RIP → perda do fluxo de execução

A mensagem [!] WARNING: Unaligned address (ABI violation) indica que o fluxo de execução foi corrompido e os endereços não respeitam o alinhamento esperado pela ABI.

## Validação com GDB

Para validar o comportamento da stack em nível de CPU, utilizei o GDB como ferramenta de inspeção direta.

### Inicialização do GDB

```
Reading symbols from build/bin/vulnerable_unsafe...
```

O binário foi compilado com -g, permitindo:

    -   Mapear endereços → linha de código
    -   Inspeção simbólica (vulnerable_function) 

### Breakpoints definidos

Foram definidos 2 Breakpoints, que são:

```
Breakpoint 1 at 0x40114e → linha 5
Breakpoint 2 at 0x40116e → linha 6
```
    -   BP1: Antes do input(printf)
    -   BP2: logo após/dentro da função de velneráveç

Neste caso, estou observando o estado antes e depois do ataque.

```
Breakpoint 1 at 0x40114e → linha 5
Breakpoint 2 at 0x40116e → linha 6
```

### Execução do payload

```
Starting program: ... < /tmp/payload.txt
```
    -   Input redirecionado (stdin)
    -   payload massivo (A * N)
    -   Execução não interativa

###  Antes do Overflow

Stack Frame:


```
frame at 0x7fffffffde80
saved rip = 0x4011a1
```

A Estrutura é:

```

| return address (RIP) | ← 0x7fffffffde78
| saved RBP            | ← 0x7fffffffde70
| buffer[16]           | ← região abaixo
```

Registradores críticos:

```
rbp = 0x7fffffffde70
rsp = 0x7fffffffde60
rip = 0x40114e
```

    -   RBP: A base do frame atual
    -   RSP: Topo da Stack
    -   RIP: Ponto atual de execução

### Dump da stack (antes)

```
0x7fffffffde70: 0x00007fffffffde80  ← próximo RBP
0x7fffffffde78: 0x00000000004011a1  ← return address
```

Cadeia válida:

```
vulnerable_function → main → libc
```

### Execução do overflow

É feito a entrada de 600 bytes de caracteres 'A', em uma memória de 16 bytes.


```
buffer[16] → overflow →
→ sobrescreve saved RBP
→ sobrescreve return address
→ continua corrompendo stack

```

### Falha (SIGSEGV)

```
Program received signal SIGSEGV
```

    -   *ret* tenta usar o endereço inválido
    -   endereço foi subrescrito com 0x41...

Observação: 0x41... é o hexadecimal de 'A'.

### Estado APÓS overflow

Registrador Critico:

```
rbp = 0x4141414141414141
```
    -   A base do frame foi corrompido
    -   Demonstrando um controle do frame pointer

### RIP no momento do crash


```
rip = 0x401197
```
    -   Esta dentro da função
    -   Crash ocorre ao tentar sair (ret)

### Backtrace corrompido

```
#1  0x4141414141414141 in ?? ()
#2  0x4141414141414141 in ?? ()
```

    -   Endereço de retorno sobrescritos
    -   Cadeia de chamadas destruída
    -   Fluxo de execução comprometido

### Dump da stack (pós-overflow)

```
0x7fffffffde78: 0x4141414141414141
0x7fffffffde88: 0x4141414141414141
...
```

    -   payload dominou a memória
    -   overflow linear e contínuo
    -   Atigindo assim:
        - RBP
        - RIP
        - Frames acima

### Conclusão Final

Antes:
```
[ buffer (16 bytes) ]
[ saved RBP ]
[ return address ]
```

Depois:
```
[ AAAAAAAAAAAAAAAA ]
[ AAAAAAAAAAAAAAAA ] ← RBP corrompido
[ AAAAAAAAAAAAAAAA ] ← RET corrompido
[ AAAAAAAAAAAAAAAA ] ← stack flooding
```

Neste projeto foi demonstrado o stack-based buffer overflow (CWE-121), com corrupção da call stack e crash determinístico.

## Conclusão

No cenário 2 (128 bytes), tivemos corrupção parcial da stack, levando à quebra do fluxo de execução.

Já no cenário 3 (300 bytes), ocorre sobrescrita completa dos mecanismos de controle (RBP/RIP).

Nesse ponto:

O programa perde totalmente o controle da execução
O fluxo pode ser redirecionado para qualquer endereço
Em um cenário real, isso permitiria execução de código arbitrário

O estouro de larga escala não apenas quebra o software, mas permite controle direto do fluxo da CPU.


## Installation

Compilar

```
 make
```
    

Executar

```
 ./build/bin/sentinel
```

Teste Full

```bash
  make test-full
```

Validação com GDB

```bash
make gdb-vuln
```
## Author

- Pablo Vinícius Sousa Silva
Low-Level Software Engineer | Systems Programming | Memory Analysis & Runtime Internals