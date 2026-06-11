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
#include "compress.h"
#include "decompress.h"

/**
 * @brief Exibe as instruções de uso correto do programa.
 */
void exibir_ajuda(const char *nome_arquivo) //exibi uma tela de ajuda de como usar o programa caso acha erro
{
    printf("Uso incorreto dos argumentos!\n");
    printf("Modo de Uso:\n");
    printf("  Compactar:    %s -c <arquivo_origem>\n", nome_arquivo);
    printf("  Descompactar:  %s -d <arquivo_origem.huff>\n", nome_arquivo);
}

int main(int argc, char *argv[])
{
    // codigo aceita exatamente 3 argumentos, o executavel, a opcao de compactar ou descompactar e o arquivo de origem
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
        // localiza o ultimo ponto para isolar o nome
        char *ultimo_ponto = strrchr(origem, '.');
        int comprimento_base = strlen(origem);

        if(ultimo_ponto != NULL)
        {
            comprimento_base = ultimo_ponto - origem;
        }

        // Geração automática alocando espaço para "nome_do_aquivo" + ".huff" + '\0'
        int tam_destino = comprimento_base + 6;
        destino = (char *)malloc(tam_destino*sizeof(char));
        if(destino == NULL)
        {
            printf("Erro de alocacao de memoria ao gerar nome do arquivo destino\n");
            return 1;
        }

        // isola o noome base e monta o arquivo final como .huff
        strncpy(destino, origem, comprimento_base);
        destino[comprimento_base] = '\0';
        strcat(destino, ".huff");
        printf("Iniciando compressao do arquivo: %s -> %s\n", origem, destino);

        // Chama o compressor ADT
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

        // verifica se o arquivo de fato termina com .huff como medida de segurança para descompactar
        if(tam_origem > 5 && strcmp(&origem[tam_origem-5], ".huff") == 0)
        {
            //removemos o .huff do final do nome do nosso arquivo alocando uma string menor
            int tam_destino = (tam_origem - 5) + 1; // tira os 5 caracteres do ".huff" e adiciona 1 '\0'
            destino = (char *)malloc(tam_destino*sizeof(char));
            if(destino == NULL)
            {
                printf("Erro de alocacao de memoria ao gerar nome do arquivo destino\n");
                return 1;
            }
            strncpy(destino, origem, tam_origem - 5); // função que copia um pedaço de uma string em outra mas com um limite de caracteres em relaçao ao tamanho e nao indice
            destino[tam_origem - 5] = '\0';
        }
        else
        {
            // Fallback caso o usuario passe o nome de um arquivo que nao é um .huff, sendo uma medida de segurança caso isso ocorra
            // esse Fallback permite que caso o arquivo seja um .huff mesmo que nao tenha escrito .huff ele segue no fluxo normal do codigo e so para caso nao seja um arquivo .huff realmente
            // pois ele ira falhar na leitura do cabeçalho ou formando uma arvore totalmente errada e parando o codigo de maneira segura evitando crash
            int tam_destino = tam_origem + 12;
            destino = (char *)malloc(tam_destino*sizeof(char));
            if(destino == NULL)
            {
                printf("Erro de alocacao de memoria ao gerar nome do arquivo destino\n");
                return 1;
            }

            snprintf(destino, tam_destino, "%s.extracted", origem);
            // função para jogar um texto dentro de uma variavel destino passando tamanho maximo, sendo apenas com a função de guarda nao printando nada na tela
        }

        printf("Iniciando descompressao do arquivo: %s -> %s\n", origem, destino);

        // Chama o Descompressor ADT
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

    // caso seja selecionado uma opção invalida
    else
    {
        printf("Opcao '%s' invalida!\n", opcao);
        exibir_ajuda(argv[0]);
        return 1;
    }

    return 0;
}