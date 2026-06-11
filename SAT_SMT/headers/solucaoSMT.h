#ifndef solucao_SMT
#define solucao_SMT
#include "leituraSMT.h"
#include "leituraSAT.h"
#include "solucaoSAT.h"

int checaIntervalo(problemaSMT *smt, vetorInterpretacoes *vetor);

operador inverteOperador(operador op);

void atualizaLimites(int *teto, int *chao, equacao e);

arvore *smtSolver(problemaSMT *smt, vetorInterpretacoes *v, int variavel);

#endif