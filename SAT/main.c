#include <stdio.h>
#include <stdbool.h>
#include "leitura.h"
#include "solucao.h"

int main(int argc, char *argv[])
{   
    const char *nomeArquivo;
    if(argc > 1)
    {
        nomeArquivo = argv[1];
    }
    else nomeArquivo = "entrada.cnf";

    formula *f = leArquivo(nomeArquivo);

    if(f == NULL)
    {
        printf("Houve um problema com esta fórmula.\n");
        return 1;
    }

    // Se chegou aqui está tudo certo!
    vetorInterpretacoes *interpretacoes = criarVetorInterpretacoes(f);

    // Vai começar a montar a árvore com a fórmula lida do arquivo, um vetor inicializado do zero e a primeira variável
    arvore *no = satSolver(f, interpretacoes, 1);

    if(no -> resultado == true)
    {
        printf("SAT!\n\n");
        printf("Este foi o caminho bem sucedido:\n");
        for(int i = 1; i < f -> qtdTotalLiterais + 1; i++)
        {
            printf("[%d] ", i);
        }
        printf("\n");
        for(int i = 1; i < f -> qtdTotalLiterais + 1; i++)
        {
            if(interpretacoes -> vetor[i] < 0) printf("%d  ", interpretacoes -> vetor[i]);
            else printf(" %d  ", interpretacoes -> vetor[i]);
            
        }
        printf("\n\n");
        printArvore(no, 0, 'R');
        printf("\n");
    }
    else
    {
        printf("UNSAT!\n\n");
        printArvore(no, 0, 'R');
    }

    desalocaArvore(no);
    desalocaFormula(f);
    desalocaInterpretacoes(interpretacoes);

    return 0;
}