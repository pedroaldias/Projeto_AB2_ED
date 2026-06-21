/**
 * @file main.c
 * @brief Ponto de entrada principal para o compactador/descompactador de Huffman.
 * * Este programa automatiza a geração dos nomes dos arquivos de saída,
 * mantendo as extensões originais de forma segura (ex: foto.bmp -> foto.bmp.huff).
 * * Uso do programa no terminal:
 * Compactar:    ./huffman -c <arquivo_origem>
 * Descompactar:  ./huffman -d <arquivo_origem.huff>
 * * @author Pedro Henrique Freire Gama, Pedro Affonso Lopes, Sofia Pacheco Rizzotto e Vitor Gabriel da Silva Santos
 * @date 2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Headers/compress.h"
#include "../Headers/decompress.h"

/**
 * @brief Exibe as instruções de uso correto do programa.
 */
void exibir_ajuda(const char *nome_arquivo)
{
    printf("Uso incorreto dos argumentos!\n");
    printf("Modo de Uso:\n");
    printf("  Compactar:    %s -c <arquivo_origem>\n", nome_arquivo);
    printf("  Descompactar:  %s -d <arquivo_origem.huff>\n", nome_arquivo);
}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        exibir_ajuda(argv[0]);
        return 1;
    }

    char *opcao = argv[1];
    char *origem = argv[2];
    char *destino = NULL;

    // Modo de Compressão
    if(strcmp(opcao, "-c") == 0)
    {
        int tam_destino = strlen(origem) + 6;
        destino = (char *)malloc(tam_destino * sizeof(char));
        if(destino == NULL)
        {
            printf("Erro de alocacao de memoria ao gerar nome do arquivo destino\n");
            return 1;
        }

        strcpy(destino, origem);
        strcat(destino, ".huff");
        printf("Iniciando compressao do arquivo: %s -> %s\n", origem, destino);

        Compressor *c = criar_compressor(origem, destino);
        if(c == NULL)
        {
            printf("Erro critico: Falha ao inicializar o compressor\n");
            free(destino);
            return 1;
        }

        if(executar_compressao(c))
        {
            printf("Sucesso! Arquivo compactado gerado com exito\n");
        }
        else
        {
            printf("Erro: Falha durante a compressao do arquivo\n");
        }

        destruir_compressor(c);
        free(destino);
    }

    // Modo Descompressão
    else if(strcmp(opcao, "-d") == 0)
    {
        int tam_origem = strlen(origem);

        if(tam_origem > 5 && strcmp(&origem[tam_origem-5], ".huff") == 0)
        {
            // Isola só o nome do arquivo sem o caminho
            char *ultimo_separador = strrchr(origem, '\\');
            if(ultimo_separador == NULL)
                ultimo_separador = strrchr(origem, '/');

            char *nome_arquivo = (ultimo_separador != NULL) ? ultimo_separador + 1 : origem;

            int tam_nome = strlen(nome_arquivo) - 5; // remove o .huff
            const char *pasta_saida = "Arquivos_Resultados\\";
            int tam_destino = strlen(pasta_saida) + tam_nome + 1;

            destino = (char *)malloc(tam_destino * sizeof(char));
            if(destino == NULL)
            {
                printf("Erro de alocacao de memoria ao gerar nome do arquivo destino\n");
                return 1;
            }

            strcpy(destino, pasta_saida);
            strncat(destino, nome_arquivo, tam_nome);
            destino[strlen(pasta_saida) + tam_nome] = '\0';
        }
        else
        {
            int tam_destino = tam_origem + 12;
            destino = (char *)malloc(tam_destino*sizeof(char));
            if(destino == NULL)
            {
                printf("Erro de alocacao de memoria ao gerar nome do arquivo destino\n");
                return 1;
            }

            snprintf(destino, tam_destino, "%s.extracted", origem);
        }

        printf("Iniciando descompressao do arquivo: %s -> %s\n", origem, destino);

        Decompressor *d = criar_decompressor(origem, destino);
        if(d == NULL)
        {
            printf("Erro critico: Falha ao inicializar o descompressor\n");
            free(destino);
            return 1;
        }

        if(executar_decompressao(d))
        {
            printf("Sucesso! Arquivo original restaurado com exito\n");
        }
        else
        {
            printf("Erro: Falha durante a descompressao do arquivo\n");
        }

        destruir_decompressor(d);
        free(destino);
    }

    else
    {
        printf("Opcao '%s' invalida!\n", opcao);
        exibir_ajuda(argv[0]);
        return 1;
    }

    return 0;
}