# Problema da Barbearia de Hilzer — Projeto de Sistemas Operacionais

## Introdução

Este projeto é baseado no livro **The Little Book of Semaphores**, de Allen B. Downey, com foco específico no **Problema da Barbearia de Hilzer** (Seção 5.4). O objetivo é adaptar a solução baseada em semáforos proposta no livro para uma versão utilizando **locks (mutexes)** e **variáveis de condição** em C com **pthreads**.

Além da implementação técnica, este projeto também investiga comparativamente como diferentes ferramentas de IA generativa, nomeadamente **ChatGPT** e **Claude (Cloud)**, abordam o mesmo problema de concorrência.

O trabalho envolve tanto a implementação em C quanto a análise crítica das soluções geradas pelas ferramentas.

## Visão Geral do Problema

O **Problema da Barbearia de Hilzer** simula uma barbearia com múltiplos barbeiros, clientes e um número limitado de cadeiras para os clientes. O desafio está em gerenciar a concorrência entre vários barbeiros atendendo simultaneamente, garantindo sincronização e controle correto do fluxo de entrada e saída de clientes.

## 📁 Estrutura de Pastas

```
Operating-Systems/
├── cloude/       # Solução gerada com a ferramenta CloudE
│   ├── barbershop
│   ├── barbershop.c
│   ├── link_prompt.txt
│   └── Makefile
│
├── gpt/          # Solução gerada com auxílio do ChatGPT
│   ├── barbershop
│   ├── barbershop.c
│   ├── link_prompt.txt
│   └── Makefile
│
└── minha/        # Solução desenvolvida pelo grupo
    ├── barbershop
    ├── barbershop.c
    ├── link_prompt_pseudo.txt
    ├── pseudocodigo.txt
    └── Makefile
```

## ⚙️ Parâmetros de Execução

O programa recebe os seguintes parâmetros via linha de comando:

- `MAX_CUSTOMERS`: número total de clientes (threads) a serem geradas.
- `NUM_BARBERS`: número de barbeiros (threads).
- `MAX_PEOPLE_IN_SHOP`: capacidade máxima de pessoas simultaneamente dentro da barbearia.
- `SOFA_CAPACITY`: quantidade máxima de clientes que podem sentar no sofá.

## 🛠️ Compilação e Execução

1. Navegue até a pasta da solução desejada:

```bash
cd cloude
# ou
cd gpt
# ou
cd minha
```

2. Compile o programa:

```bash
make
```

3. Execute o programa com os parâmetros desejados:

```bash
./barbershop MAX_CUSTOMERS NUM_BARBERS MAX_PEOPLE_IN_SHOP SOFA_CAPACITY
```

### 💡 Exemplo

```bash
./barbershop 12 3 10 4
```

**Significado:**

- `12` clientes (threads) serão criados.
- `3` barbeiros disponíveis para atendimento.
- A barbearia comporta no máximo `10` pessoas simultaneamente.
- O sofá comporta até `4` pessoas sentadas.
