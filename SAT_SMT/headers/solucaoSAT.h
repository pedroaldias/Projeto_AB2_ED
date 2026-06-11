#ifndef solucaoSAT_h
#define solucaoSAT_h
#include "leituraSAT.h"
#include <stdbool.h>

typedef struct vetorInterpretacoes
{
    int *vetor;
} vetorInterpretacoes;

typedef struct arvore
{
    int literal;
    struct arvore *esq;
    struct arvore *dir;
    bool resultado;
} arvore;

vetorInterpretacoes* criarVetorInterpretacoes(formula *f);

arvore *satSolver(formula *f, vetorInterpretacoes *v, int variavel);

int checaFormula(formula *f, vetorInterpretacoes *v);

void printArvore(arvore *raiz, int nivel, char direcao);

void desalocaArvore(arvore *raiz);

void desalocaInterpretacoes(vetorInterpretacoes *i);

#endif