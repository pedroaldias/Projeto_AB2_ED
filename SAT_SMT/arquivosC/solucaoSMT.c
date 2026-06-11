#include <math.h>
#include <limits.h>
#include "../headers/leituraSMT.h"
#include "../headers/leituraSAT.h"
#include "../headers/solucaoSMT.h"
#include "../headers/solucaoSAT.h"

operador inverteOperador(operador op)
{
    switch(op) 
    {
        case MENOR:       return MAIOR_IGUAL;
        case MAIOR:       return MENOR_IGUAL;
        case MENOR_IGUAL: return MAIOR;
        case MAIOR_IGUAL: return MENOR;
        case IGUAL:       return DIFERENTE;
        case DIFERENTE:   return IGUAL;
    }
    return op;
}

void atualizaLimites(int *teto, int *chao, equacao e)
{
    int ladoDireito = e.c - e.b;
    /*
        A Teoria LIA garante que
        ax < K   => ax <= K - 1
        ax > K   => ax >= K + 1

        Aplicamos essa regra em código para não 
        termos que lidar com os operadores < e >
    */

    if(e.op == MAIOR)
    {
        // maior igual!
        e.op = MAIOR_IGUAL;
        ladoDireito += 1;
    }
    else if(e.op == MENOR)
    {
        // menor igual!
        e.op = MENOR_IGUAL;
        ladoDireito -= 1;
    }

    double divisao = (double)ladoDireito / e.a;

    int novoTeto = (int)floor(divisao);
    int novoChao = (int)ceil(divisao);

    if(e.op == MENOR_IGUAL)
    {
        if(novoTeto < *teto) *teto = novoTeto;
    }
    else if(e.op == MAIOR_IGUAL)
    {
        if(novoChao > *chao) *chao = novoChao;
    }
    else if(e.op == IGUAL)
    {
        if(novoTeto < *teto) *teto = novoTeto;
        if(novoChao > *chao) *chao = novoChao; 
    }
    else
    {
        /*o "diferente" usa a divisão crua, sem arredondamento de teto/chão, 
        pois x != 2.5 não afeta números inteiros*/
        if (*chao == ladoDireito / e.a && *teto == ladoDireito / e.a)
        {
        // forçando conflito matemático
        *chao = *teto + 1; 
        }
    }
}

int checaIntervalo(problemaSMT *smt, vetorInterpretacoes *i)
{
    int teto = INT_MAX;
    int chao = INT_MIN;

    
    for(int j = 1; j <= smt->logicaSAT->qtdTotalLiterais; j++)
    {
        equacao e = smt -> equacoes[j];

        int decisao = i -> vetor[j];

        if(decisao == 0) continue;
        if(decisao == -1)
        {
            e.op = inverteOperador(e.op);
        }

        //passando o endereço na memória para modificarmos o valor, sem perdê-lo, na função
        atualizaLimites(&teto, &chao, e);
    }

    if(chao > teto) return 0; //UNSAT

    return 1; //até aqui tudo bem...
}

arvore *smtSolver(problemaSMT *smt, vetorInterpretacoes *v, int variavel)
{
    arvore *no = malloc(sizeof(arvore));
    no -> literal = variavel;
    no -> esq = NULL;
    no -> dir = NULL;
    no -> resultado = false;

    formula *f = smt -> logicaSAT;
    int estado = checaFormula(f, i);
    
    if(estado == -1)
    {
        no -> resultado = false;
        return no;
    }

    // diferencial SMT: análise matemático sobre a fórmula!
    if (checaIntervalo(smt, i) == 0)
    {
        no -> resultado = false;
        return no; // Poda Antecipada: UNSAT Matemático!
    }

    if(estado == 1)
    {
        no -> resultado = true;
        return no;
    }

    // só roda se a checagem para a fórmula for inconclusiva
    if(variavel > f -> qtdTotalLiterais) 
    {
        no -> resultado = false;
        return no;
    }

    no -> literal = variavel;

    i -> vetor[variavel] = 1;
    no -> esq = smtSolver(smt, i, variavel + 1);

    if(no -> esq != NULL && no -> esq -> resultado == true) //se já é SAT...
    {
        no -> resultado = true;
        return no;
    }

    i -> vetor[variavel] = -1;
    no -> dir = smtSolver(smt, i, variavel + 1);
    if(no -> dir != NULL && no -> dir -> resultado == true) //se já é SAT...
    {
        no -> resultado = true;
        return no;
    }

    i -> vetor[variavel] = 0;
    return no;
}