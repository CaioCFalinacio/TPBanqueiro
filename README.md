# Trabalho Prático 1: Algoritmo do Banqueiro

## Sobre o projeto

Este projeto implementa o Algoritmo do Banqueiro, com o objetivo de alocar recursos a múltiplos clientes (threads) garantindo que o sistema permaneça sempre em um estado seguro, prevenindo *deadlocks*. O projeto foi desenvolvido em **C**, utilizando a biblioteca **Pthreads** e *locks mutex* para evitar condições de corrida.

## Estrutura de arquivos

- `src/main.c`: Código-fonte principal com a lógica do Banqueiro e das threads.
- `docs/`: Diretório contendo o relatório final em PDF.

## Como Compilar

Para compilar o código fonte, certifique-se de ter o compilador GCC instalado. Abra o terminal na raiz do projeto e execute o seguinte comando:

```bash
gcc -o banqueiro src/main.c -lpthread -Wall
```
## Autores

- Caio César Falinacio dos Santos
- Luiz Fernando Cunha Maia
