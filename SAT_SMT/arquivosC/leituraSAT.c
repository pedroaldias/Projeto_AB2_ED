#include <stdio.h>
#include <stdlib.h>
#include "../headers/leituraSAT.h"

formula* criarFormula()
{
    formula *novaFormula = malloc(sizeof(formula));
    novaFormula -> head = NULL;
    novaFormula -> qtdTotalClausulas = 0;
    novaFormula -> qtdTotalLiterais = 0;

    return novaFormula;
}

void inserirClausula(formula *formula, int *literais, int tam)
{
    clausula *novaClausula = malloc(sizeof(clausula));
    novaClausula -> prox = NULL;

    novaClausula -> literais = malloc(sizeof(int) * tam);
    for(int i = 0; i < tam; i++)
    {
        novaClausula -> literais[i] = literais[i];
    }

    novaClausula -> qtdLocalLiterais = tam;
    if(formula -> head == NULL) formula -> head = novaClausula;
    else
    {
        clausula *atual = formula -> head;
        while(atual -> prox != NULL) atual = atual -> prox;
        atual -> prox = novaClausula;
    }
}

formula* leArquivoSAT(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL) 
    {
        printf("Erro ao abrir o arquivo.\n");
        return NULL;
    }

    formula *formula = criarFormula();
    char tipo;

    while (fscanf(file, " %c", &tipo) == 1) 
    {
        if (tipo == 'c') while (fgetc(file) != '\n'); // a linha é um comentário, ignore
        else if (tipo == 'p') // cabeçalho!
        {
            char format[4];
            fscanf(file, " %3s %d %d", format, &formula -> qtdTotalLiterais, &formula -> qtdTotalClausulas);
            break; 
        }
    }

    int fim = formula -> qtdTotalClausulas;
    int i = 0;
    int buffer[formula -> qtdTotalLiterais];

    while(fim != 0)
    {
        int num;
        
        if(fscanf(file, "%d", &num) != 1) break;
        if(num != 0)
        {
            buffer[i] = num;
            i++;
        }
        else
        {
            inserirClausula(formula, buffer, i);
            i = 0;
            fim--;
        }
    }

    fclose(file);
    return formula;
}

void desalocaFormula(formula *formula)
{
    if(formula == NULL) return;
    clausula *atual = formula -> head;

    while(atual != NULL)
    {
        clausula *prox = atual -> prox;

        if(atual -> literais != NULL) free(atual -> literais);
        free(atual);
        atual = prox;
    }

    free(formula);
}
