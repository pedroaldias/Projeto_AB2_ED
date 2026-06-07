#include <stdio.h>
#include <stdlib.h>
#include "leitura.h"
#include "solucao.h"
#include <stdbool.h>

vetorInterpretacoes* criarVetorInterpretacoes(formula *f)
{
    vetorInterpretacoes *interpretacoes = malloc(sizeof(vetorInterpretacoes));
    interpretacoes -> vetor = malloc(sizeof(int) * (f -> qtdTotalLiterais + 1)); // +1 pois vamos começar a preencher de i = 1
    for(int i = 1; i <= f -> qtdTotalLiterais; i++)
    {
        interpretacoes -> vetor[i] = 0;
    }

    return interpretacoes;
}

int checaFormula(formula *f, vetorInterpretacoes *interpretacoes)
{
    clausula *atual = f -> head;
    bool clausulaVerdadeira = false;
    bool formulaInconclusiva = false;

    while(atual != NULL)
    {
        bool clausulaVerdadeira = false;
        bool faltaAtribuicao = false;

        for(int i = 0; i <= atual -> qtdLocalLiterais; i++)
        {
            int literal = atual -> literais[i];
            int var = abs(literal);
            int valorAtribuido = interpretacoes -> vetor[var];

            if(valorAtribuido == 0) faltaAtribuicao = true;
            else
            {
                if(literal > 0 && valorAtribuido == 1 || literal < 0 && valorAtribuido == -1)
                {
                    clausulaVerdadeira = true;
                    break;
                }
            }
        }

        if(clausulaVerdadeira == false)
        {
            if(faltaAtribuicao == false)
            {
                return -1;
            }
            else formulaInconclusiva = true;
        }

        atual = atual -> prox;
    }

    if(formulaInconclusiva == true) return 0; //inconclusivo
    else return 1; //SAT!

}

arvore *satSolver(formula *f, vetorInterpretacoes *i, int variavel)
{
    arvore *no = malloc(sizeof(arvore));
    no -> literal = variavel;
    no -> esq = NULL;
    no -> dir = NULL;
    no -> resultado = false;

    int estado = checaFormula(f, i);
    if(estado == 1)
    {
        no -> resultado = true;
        return no;
    }
    if(estado == -1)
    {
        no -> resultado = false;
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
    no -> esq = satSolver(f, i, variavel + 1);

    if(no -> esq != NULL && no -> esq -> resultado == true) //se já é SAT...
    {
        no -> resultado = true;
        return no;
    }

    i -> vetor[variavel] = -1;
    no -> dir = satSolver(f, i, variavel + 1);
    if(no -> dir != NULL && no -> dir -> resultado == true) //se já é SAT...
    {
        no -> resultado = true;
        return no;
    }

    i -> vetor[variavel] = 0;
    return no;

}

void printArvore(arvore *raiz, int nivel, char direcao)
{
    if (raiz == NULL) return;

    // 1. Cria a indentação baseada na profundidade do nó
    for (int i = 0; i < nivel; i++) {
        printf("    "); // 4 espaços por nível
    }

    // 2. Imprime o caminho tomado para chegar neste nó
    if (direcao == 'E') {
        printf("├── [x%d = 1] ", raiz->literal - 1); // Subtrai 1 pois o nó pai tomou a decisão
    } else if (direcao == 'D') {
        printf("└── [x%d = -1] ", raiz->literal - 1);
    } else {
        printf("RAIZ: ");
    }

    // 3. Verifica o estado do nó atual (Decisão ou Folha)
    if (raiz->esq == NULL && raiz->dir == NULL) 
    {
        // Nó Folha: Bateu na parede (UNSAT) ou achou a saída (SAT)
        if (raiz->resultado == true) {
            printf("FOLHA -> SAT!\n");
        } else {
            printf("FOLHA -> UNSAT\n");
        }
    } 
    else 
    {
        // Nó de Decisão: Mostra qual variável ele vai ramificar a seguir
        printf("Decisao (Variavel x%d)\n", raiz->literal);
    }

    // 4. Chamadas recursivas para desenhar os filhos
    printArvore(raiz->esq, nivel + 1, 'E');
    printArvore(raiz->dir, nivel + 1, 'D');
}

void desalocaArvore(arvore *raiz)
{
    if (raiz == NULL) return;

    desalocaArvore(raiz->esq);
    desalocaArvore(raiz->dir);

    free(raiz);
}

void desalocaInterpretacoes(vetorInterpretacoes *i)
{
    free(i -> vetor);
    free(i);
}
