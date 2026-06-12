#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
        if(novoTeto != novoChao) // divisão não é inteira exata, impossível satisfazer
        {
            *chao = *teto + 1; // força chao > teto, sinalizando UNSAT
        }
        else
        {
            if(novoTeto < *teto) *teto = novoTeto;
            if(novoChao > *chao) *chao = novoChao; 
        }
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
    int estado = checaFormula(f, v);
    
    if(estado == -1)
    {
        no -> resultado = false;
        return no;
    }

    // diferencial SMT: análise matemático sobre a fórmula!
    if (checaIntervalo(smt, v) == 0)
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

    v -> vetor[variavel] = 1;
    no -> esq = smtSolver(smt, v, variavel + 1);

    if(no -> esq != NULL && no -> esq -> resultado == true) //se já é SAT...
    {
        no -> resultado = true;
        return no;
    }

    v -> vetor[variavel] = -1;
    no -> dir = smtSolver(smt, v, variavel + 1);
    if(no -> dir != NULL && no -> dir -> resultado == true) //se já é SAT...
    {
        no -> resultado = true;
        return no;
    }

    v -> vetor[variavel] = 0;
    return no;
}

void desalocaSMT(problemaSMT *smt)
{
    if(smt != NULL)
    {
        desalocaFormula(smt -> logicaSAT);
        free(smt -> equacoes);
        free(smt);
    }
}

void printArvoreSMT(arvore *raiz, int nivel, char direcao, problemaSMT *smt, int indicePai)
{
    if (raiz == NULL) return;

    for (int i = 0; i < nivel; i++) printf("    ");

    const char *opStr;

    // Nó folha: não tem filhos, só imprime o resultado
    if (raiz->esq == NULL && raiz->dir == NULL)
    {
        if (direcao == 'E') printf("├── ");
        else if (direcao == 'D') printf("└── ");
        else printf("RAIZ: ");

        if (raiz->resultado == true && indicePai >= 1)
        {
            equacao e = smt->equacoes[indicePai];
            operador opExibido = (direcao == 'D') ? inverteOperador(e.op) : e.op;
            switch(opExibido)
            {
                case MENOR_IGUAL: opStr = "<="; break;
                case MAIOR_IGUAL: opStr = ">="; break;
                case IGUAL:       opStr = "=="; break;
                case MENOR:       opStr = "<";  break;
                case MAIOR:       opStr = ">";  break;
                case DIFERENTE:   opStr = "!="; break;
                default:          opStr = "?";  break;
            }
            int ladoDireito = e.c - e.b;
            printf("FOLHA -> SAT! (x %s %d)\n", opStr, ladoDireito / e.a);
        }
        else if (raiz->resultado == true)
            printf("FOLHA -> SAT!\n");
        else
            printf("FOLHA -> UNSAT\n");
        return;
    }

    // Nó interno: tem filhos, imprime a decisão
    if (direcao == 'E' || direcao == 'D')
    {
        equacao e = smt->equacoes[indicePai];
        operador opExibido = (direcao == 'D') ? inverteOperador(e.op) : e.op;
        switch(opExibido)
        {
            case MENOR_IGUAL: opStr = "<="; break;
            case MAIOR_IGUAL: opStr = ">="; break;
            case IGUAL:       opStr = "=="; break;
            case MENOR:       opStr = "<";  break;
            case MAIOR:       opStr = ">";  break;
            case DIFERENTE:   opStr = "!="; break;
            default:          opStr = "?";  break;
        }
        if (direcao == 'E')
            printf("├── [%dx + %d %s %d = TRUE]  ", e.a, e.b, opStr, e.c);
        else
            printf("└── [%dx + %d %s %d = FALSE] ", e.a, e.b, opStr, e.c);
    }
    else
        printf("RAIZ: ");

    // Mostra qual equação será decidida a seguir
    if (raiz->literal >= 1 && raiz->literal <= smt->logicaSAT->qtdTotalLiterais)
    {
        equacao prox = smt->equacoes[raiz->literal];
        switch(prox.op)
        {
            case MENOR_IGUAL: opStr = "<="; break;
            case MAIOR_IGUAL: opStr = ">="; break;
            case IGUAL:       opStr = "=="; break;
            case MENOR:       opStr = "<";  break;
            case MAIOR:       opStr = ">";  break;
            case DIFERENTE:   opStr = "!="; break;
            default:          opStr = "?";  break;
        }
        printf("Decisao (%dx + %d %s %d ?)\n", prox.a, prox.b, opStr, prox.c);
    }
    else printf("\n");

    printArvoreSMT(raiz->esq, nivel + 1, 'E', smt, raiz->literal);
    printArvoreSMT(raiz->dir, nivel + 1, 'D', smt, raiz->literal);
}