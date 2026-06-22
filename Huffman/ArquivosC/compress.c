#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Headers/compress.h"
#include "../Headers/huffman_tree.h"
#include "../Headers/bitio.h"
#include "../Headers/header.h"

struct compressor
{
    char *origem;  // nome do arquivo de entrada
    char *destino; // nome do arquivo de saida
};

// cria o compressor e duplica os nomes dos arquivos para garantir o encapsulamento
Compressor *criar_compressor(const char *arquivo_origem, const char *arquivo_destino)
{
    if(arquivo_origem == NULL || arquivo_destino == NULL) return NULL;

    Compressor *c = (Compressor *)malloc(sizeof(Compressor));
    if(c == NULL) return NULL; // medida de segurança

    // duplica as strings para o compressor nao depender dos ponteiros originais
    c->origem = strdup(arquivo_origem);
    c->destino = strdup(arquivo_destino);

    return c;
}

// executa todo o processo de compressao: conta frequencias, monta a arvore, grava o header e os dados bit a bit
int executar_compressao(Compressor *c)
{
    if(c == NULL) return 0;

    // PASSO 1: conta a frequencia dos bytes no arquivo original
    FILE *f_origem = fopen(c->origem, "rb");
    if(f_origem == NULL) return 0;

    int frequencias[256] = {0};
    int caractere;
    while((caractere = fgetc(f_origem)) != EOF) // percorre todos os bytes e soma +1 na sua frequencia
    {
        frequencias[caractere]++;
    }

    // PASSO 2: cria a arvore de huffman e o dicionario de codigos
    NodeHuffman *arvore_huffman = construir_a_arvore(frequencias);
    if(arvore_huffman == NULL)
    {
        fclose(f_origem);
        return 0;
    }

    // tabela de strings com o codigo de cada um dos 256 bytes
    char caminho[257];
    char *dicionario[256] = {NULL};
    gerar_codigo_huffman(arvore_huffman, dicionario, caminho, 0);
    // caminho tem espaço para o maior codigo possivel (256 bits + \0) e e reescrito durante a recursao

    // PASSO 3: calcula o total de bits (frequencia * tamanho do codigo) e o lixo do ultimo byte
    long long total_bits = 0;
    for(int i = 0; i < 256; i++)
    {
        if(dicionario[i] != NULL)
        {
            total_bits += (long long)frequencias[i] * strlen(dicionario[i]);
        }
    }

    // lixo = bits que faltam para completar o ultimo byte (0 caso ja esteja completo)
    int lixo = (8 - (int)(total_bits % 8)) % 8;

    // PASSO 4: abre o destino como FILE* normal para gravar o cabecalho em modo byte puro
    // o cabecalho e a arvore sao gravados como bytes; so os dados usam o bit a bit do BitFile
    FILE *f_saida = fopen(c->destino, "wb");
    if(f_saida == NULL)
    {
        // limpeza de emergencia pois nao foi possivel criar o arquivo de saida
        liberar_dicionario(dicionario);
        destruir_arvore_huffman(arvore_huffman);
        fclose(f_origem);
        return 0;
    }

    Header *h = criar_header(arvore_huffman, lixo);
    escrever_header(h, f_saida); // grava cabecalho em modo byte puro
    destruir_header(h);

    // envolve o FILE* ja aberto em um BitFile para gravar os dados bit a bit no mesmo arquivo
    BitFile *f_destino = wrap_bit_file(f_saida, "wb");
    if(f_destino == NULL)
    {
        liberar_dicionario(dicionario);
        destruir_arvore_huffman(arvore_huffman);
        fclose(f_origem);
        fclose(f_saida);
        return 0;
    }

    // volta o ponteiro do arquivo original para o inicio
    rewind(f_origem);

    // PASSO 5: le o arquivo original byte a byte e grava os bits do codigo correspondente
    while((caractere = fgetc(f_origem)) != EOF)
    {
        char *codigo = dicionario[caractere];

        // percorre a string de 0s e 1s do codigo e grava bit a bit
        for(int i = 0; codigo[i] != '\0'; i++)
        {
            if(codigo[i] == '1')
            {
                write_bit(f_destino, 1);
            }
            else
            {
                write_bit(f_destino, 0);
            }
        }
    }

    // PASSO 6: fecha os arquivos e libera toda a memoria utilizada
    // close_bit_file faz o flush dos ultimos bits e tambem fecha f_saida (mesmo FILE* do wrap)
    close_bit_file(f_destino);
    fclose(f_origem);
    liberar_dicionario(dicionario);
    destruir_arvore_huffman(arvore_huffman);

    return 1; // compressao realizada com sucesso
}

// destroi o compressor da memoria, liberando as strings duplicadas
void destruir_compressor(Compressor *c)
{
    if(c != NULL)
    {
        free(c->origem);
        free(c->destino);
        free(c);
    }
}