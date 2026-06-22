#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Headers/decompress.h"
#include "../Headers/huffman_tree.h"
#include "../Headers/bitio.h"
#include "../Headers/header.h"

struct decompressor
{
    char *origem;
    char *destino;
};

// cria o decompressor duplicando os nomes dos arquivos de origem e destino
Decompressor *criar_decompressor(const char *arquivo_origem, const char *arquivo_destino)
{
    if(arquivo_origem == NULL || arquivo_destino == NULL) return NULL;

    Decompressor *d = (Decompressor *)malloc(sizeof(Decompressor));
    if(d == NULL) return NULL;

    // duplica as strings para o decompressor nao depender dos ponteiros originais
    d->origem = strdup(arquivo_origem);
    d->destino = strdup(arquivo_destino);

    return d;
}

// executa a descompressao: le o header, reconstroi a arvore e decodifica os bits ate o destino
int executar_decompressao(Decompressor *d)
{
    if(d == NULL) return 0;

    // PASSO 1: abre o arquivo compactado para leitura bit a bit
    BitFile *f_origem = open_bit_file(d->origem, "rb");
    if(f_origem == NULL) return 0;

    // PASSO 2: le o cabecalho, reconstruindo a arvore e obtendo o lixo
    int lixo = 0;
    int tam_arvore = 0;
    NodeHuffman *raiz_huffman = ler_header(f_origem, &lixo, &tam_arvore);
    if(raiz_huffman == NULL) // arquivo corrompido
    {
        close_bit_file(f_origem);
        return 0;
    }

    // apos o header, o BitFile ja esta posicionado no primeiro bit dos dados;
    // pega o tamanho total do arquivo para calcular quantos bytes sao de dados
    FILE *f_tam = fopen(d->origem, "rb");
    fseek(f_tam, 0, SEEK_END);
    long tam_total_bytes = ftell(f_tam);
    fclose(f_tam);

    // header = 2 bytes fixos + tamanho_arvore bytes; o resto e dado comprimido
    int tam_header_bytes = 2 + tam_arvore;
    long bytes_dados = tam_total_bytes - tam_header_bytes;
    long long bits_uteis = ((bytes_dados * 8) - lixo);
    if(bytes_dados <= 0 || bits_uteis <= 0)
    {
        // protege contra arquivo vazio ou corrompido
        destruir_arvore_huffman(raiz_huffman);
        close_bit_file(f_origem);
        return 0;
    }

    // PASSO 4: abre o arquivo de destino para gravar os bytes decodificados
    FILE *f_destino = fopen(d->destino, "wb");
    if(f_destino == NULL)
    {
        destruir_arvore_huffman(raiz_huffman);
        close_bit_file(f_origem);
        return 0;
    }

    // PASSO 5: decodifica caminhando na arvore bit a bit ate atingir bits_uteis
    NodeHuffman *atual = raiz_huffman; // copia da raiz para caminhar sem alterar a raiz
    long long bits_lidos = 0;

    while(bits_lidos < bits_uteis)
    {
        int bit = read_bit(f_origem);
        if(bit == -1)
        {
            // medida de seguranca contra arquivo corrompido
            break;
        }

        bits_lidos++;

        // 0 vai para esquerda, 1 vai para direita; sem direita forca esquerda (caso de byte unico)
        if(atual->dir == NULL) atual = atual->esq;
        else if(bit == 1)
        {
            atual = atual->dir;
        }
        else // bit == 0
        {
            atual = atual->esq;
        }

        // chegou numa folha: encontrou um byte original
        if(atual->esq == NULL && atual->dir == NULL)
        {
            fputc(atual->caractere, f_destino);
            atual = raiz_huffman; // volta para a raiz
        }
    }

    // PASSO 7: limpeza geral e fechamento dos arquivos usados
    fclose(f_destino);
    destruir_arvore_huffman(raiz_huffman);
    close_bit_file(f_origem);

    // sucesso se conseguimos ler exatamente os bits uteis previstos
    return (bits_lidos == bits_uteis);
}

// destroi a estrutura do decompressor sem apagar os dados usados
void destruir_decompressor(Decompressor *d)
{
    if(d != NULL)
    {
        free(d->origem);
        free(d->destino);
        free(d);
    }
}