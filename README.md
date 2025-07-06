# Hilzer’s Barbershop Problem — Operating Systems Project


## Introduction

This project is based on the **Little Book of Semaphores** by Allen B. Downey, specifically focusing on **Hilzer’s Barbershop Problem** (Section 5.4). The goal is to adapt the semaphore-based solution from the book into a version using **locks** and **condition variables** in C with **pthreads**. Additionally, this project aims to investigate and compare how different generative AI tools, namely **GPT** and **Cloud**, approach the same concurrency problem.

The project involves both technical implementation and a comparative study of different AI tools, with students required to interact with two AI tools, document the process, and critically analyze the results.

## Objectives

- **Technical Objective**: Implement a solution for the Hilzer's Barbershop problem using **locks** and **condition variables** in C with pthreads.
- **AI Tools Comparison**: Investigate and compare the different generative AI tools (**GPT** and **Cloud**) to solve the same concurrency problem. Document the interactions and provide a critical analysis of the outcomes.

## Problem Overview

The **Hilzer’s Barbershop problem** simulates a barbershop with multiple barbers, customers, and a limited number of chairs for customers. The problem involves handling concurrency with multiple barbers serving customers, ensuring synchronization among them, and managing the flow of customers in and out of the shop.

The specific requirements for this project are to:
- Adapt the semaphore-based solution to use **locks** and **condition variables**.
- Implement synchronization and resource management to ensure proper concurrency among the barbers and customers.
- Compare how different AI tools (**GPT** and **Cloud**) propose solutions for this concurrency problem.




Este repositório contém três implementações distintas para o problema de concorrência conhecido como **Hilzer’s Barbershop**, adaptado do livro _The Little Book of Semaphores_ (Seção 5.4). As soluções foram implementadas em C utilizando `pthreads`, `mutexes` e `variáveis de condição`.

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
└── minha/        # Solução final desenvolvida pelo grupo
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

Significado:

- `12` clientes (threads) serão criados.
- `3` barbeiros disponíveis para atendimento.
- A barbearia comporta no máximo `10` pessoas simultaneamente.
- O sofá comporta até `4` pessoas sentadas.
