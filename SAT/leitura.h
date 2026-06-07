#ifndef leitura_h
#define leitura_h
#include <stdbool.h>

/*
    Uma entrada será:
    p cnf 3 3 
    -1 2 0
    1 -2 0
    -1 -3 0

    onde 3 e 3 são, respectivamente, a qtd de literais e de clausulas
    cada linha é uma cláusula
    cada número é um literal, o sinal de menos indica negação
    o zero indica fim de linha.

    O exemplo pode ser lido como: (~x or y) ^ (x or ~y) ^ (~x or ~z)
*/

// Cada clausula é uma lista encadeada com endereço para a próxima
typedef struct clausula 
{
    int *literais;          // array dinâmico para cada linha de entrada
    int qtdLocalLiterais;
    struct clausula *prox;
} clausula;

typedef struct formula 
{
    clausula *head;
    int qtdTotalClausulas;
    int qtdTotalLiterais;
} formula;

// Assinatura das funções

formula* criarFormula();

void inserirClausula(formula *formula, int *literais, int tam);

formula* leArquivo(const char *entrada);

void desalocaFormula(formula *formula);

#endif