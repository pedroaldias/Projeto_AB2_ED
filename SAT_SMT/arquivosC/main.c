#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../headers/leituraSAT.h"
#include "../headers/solucaoSAT.h"
#include "../headers/leituraSMT.h"
#include "../headers/solucaoSMT.h"
int main(int argc, char *argv[])
{   
    int opcao;

    printf("========================================\n");
    printf("       BEM-VINDO AO SOLVER SAT/SMT      \n");
    printf("========================================\n");
    printf("[1] Resolver problema puramente Logico (SAT)\n");
    printf("[2] Resolver problema Matematico (SMT - LIA)\n");
    printf("Escolha uma opcao: ");

    if (scanf("%d", &opcao) != 1) 
    {
        printf("Entrada invalida. Encerrando...\n");
        return 1;
    }
    printf("\n");
    // =========================================================
    // VARIÁVEIS COMPARTILHADAS (Declaradas antes do IF)
    // =========================================================
    arvore *no = NULL;
    vetorInterpretacoes *interpretacoes = NULL;
    int qtdLiterais = 0; // Auxiliar para o laço de impressão no final

    // Ponteiros das estruturas (iniciam nulos)
    formula *f = NULL;
    problemaSMT *smt = NULL;
    // =========================================================
    // ROTEAMENTO DA SOLUÇÃO (A sua ideia aplicada)
    // =========================================================
    if (opcao == 1)
    {
        const char *nomeArquivo = (argc > 1) ? argv[1] : "entrada.cnf";
        f = leArquivoSAT(nomeArquivo);
        if(f == NULL) return 1;
        interpretacoes = criarVetorInterpretacoes(f);
        no = satSolver(f, interpretacoes, 1);
        qtdLiterais = f->qtdTotalLiterais; // Salva para o print
    }
    else if (opcao == 2)
    {
        const char *nomeArquivo = (argc > 1) ? argv[1] : "entrada.smt";
        smt = leArquivoSMT(nomeArquivo);
        if(smt == NULL) return 1;
        interpretacoes = criarVetorInterpretacoes(smt->logicaSAT);
        no = smtSolver(smt, interpretacoes, 1);
        qtdLiterais = smt->logicaSAT->qtdTotalLiterais; // Salva para o print
    }
    else
    {
        printf("Opcao invalida. Encerrando o programa.\n");
        return 1;
    }
    // =========================================================
    // IMPRESSÃO E DESALOCAÇÃO UNIFICADAS
    // =========================================================
    if(no->resultado == true)
    {
        printf("SAT!\n\nEste foi o caminho bem sucedido:\n");
        for(int i = 1; i <= qtdLiterais; i++) printf("[%d] ", i);
        printf("\n");
        for(int i = 1; i <= qtdLiterais; i++)
        {
            if(interpretacoes->vetor[i] < 0) printf("%d  ", interpretacoes->vetor[i]);
            else printf(" %d  ", interpretacoes->vetor[i]);
        }
        printf("\n\n");

        // Usa a função correta dependendo do modo
        if (smt != NULL) printArvoreSMT(no, 0, 'R', smt, 0);
        else             printArvore(no, 0, 'R');

        printf("\n");
    }
    else
    {
        printf("UNSAT!\n\n");

        if (smt != NULL) printArvoreSMT(no, 0, 'R', smt, 0);
        else             printArvore(no, 0, 'R');
    }
    // Limpeza de memória compartilhada
    desalocaArvore(no);
    desalocaInterpretacoes(interpretacoes);

    // Limpa apenas a estrutura que foi efetivamente alocada no IF
    if (f != NULL) desalocaFormula(f);
    if (smt != NULL) desalocaSMT(smt); // Lembre de criar essa função depois!
    return 0;
}