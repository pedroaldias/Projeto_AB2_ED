#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/leituraSMT.h"
#include "../headers/leituraSAT.h"
#include "../headers/solucaoSMT.h"

problemaSMT* leArquivoSMT(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Erro ao abrir o arquivo.\n");
        return NULL;
    }

    problemaSMT *smt = malloc(sizeof(problemaSMT));
    smt -> logicaSAT = criarFormula();
    char tipo;

    while(fscanf(file, " %c", &tipo) == 1)
    {
        if (tipo == 'c') while (fgetc(file) != '\n');
        else if (tipo == 'p')
        {
            char format[4];
            fscanf(file, " %3s %d %d", format, &smt->logicaSAT->qtdTotalLiterais, &smt->logicaSAT->qtdTotalClausulas);
            
            smt -> equacoes = malloc(sizeof(equacao) * (smt->logicaSAT->qtdTotalLiterais + 1));
        }
        else if(tipo == 'e')
        {
            int indice, a, b, c;
            char stringOperador[3];
            fscanf(file, " %d %d %d %s %d", &indice, &a, &b, stringOperador, &c);
            
            equacao e;
            e.a = a;
            e.b = b;
            e.c = c;

            if (strcmp(stringOperador, "<=") == 0) e.op = MENOR_IGUAL;
            else if (strcmp(stringOperador, "==") == 0) e.op = IGUAL;
            else if (strcmp(stringOperador, ">=") == 0) e.op = MAIOR_IGUAL;
            else if (strcmp(stringOperador, ">") == 0)  e.op = MAIOR;
            else if (strcmp(stringOperador, "<") == 0)  e.op = MENOR;
            else if(strcmp(stringOperador, "!=") == 0) e.op = DIFERENTE;

            smt -> equacoes[indice] = e;
        }
        else
        {
            ungetc(tipo, file);

            //int buffer[smt->logicaSAT->qtdTotalLiterais] pode dar erro, pesquisar melhor porque
            int *buffer = malloc(sizeof(int) * smt->logicaSAT->qtdTotalLiterais); //buffer alocado dinamicamente

            int fim = smt->logicaSAT->qtdTotalClausulas;
            while(fim != 0)
            {
                int tamClausula = 0;
                int literalLido;
                while (fscanf(file, "%d", &literalLido) == 1 && literalLido != 0)
                {
                    buffer[tamClausula] = literalLido;
                    tamClausula++;
                }

                inserirClausula(smt -> logicaSAT, buffer, tamClausula);
                fim--;
            }
            free(buffer);
            break;
        }
    }
    return smt;
}