#include <stdio.h>
#include <stdlib.h>
#include "../Headers/header.h"
#include "../Headers/huffman_tree.h"
#include "../Headers/bitio.h"

/*
* Formato do arquivo .huff (conforme especificacao do professor)
*
* Byte 0 e 1 (16 bits gravados como 2 bytes normais via fputc):
*   bits 7..5 do byte 0 = quantidade de bits de lixo no último byte (0-7)
*   bits 4..0 do byte 0 + byte 1 completo = tamanho da árvore serializada (13 bits)
*
* Byte 2 até (2 + tamanho_arvore - 1):
*   bytes da árvore em pré-ordem conforme serializar_arvore()
*   gravados diretamente via fputc, sem empacotamento bit a bit
*
* Bytes restantes:
*   dados comprimidos bit a bit, o último byte pode ter bits de lixo no final
*/

/* tamanho máximo do buffer de serialização da árvore:
* pior caso: 256 folhas, cada folha = 2 bytes ('\' + símbolo) apenas para * e \
* cada nó interno = 1 byte ('*'), máximo 255 nós internos
* total máximo: 256*2 + 255 = 767 bytes — usamos 1024 com folga */
#define TAM_MAX_ARVORE 1024

struct header
{
    int lixo;                                    // quantidade de bits lixo no ultimo byte
    int tamanho_arvore;                          // tamanho de bytes totais da arvore serializada(linear)
    unsigned char arvore_buffer[TAM_MAX_ARVORE]; // bytes da arvore serializados guardado em forma de pre-ordem, Ex: *\*\AB, guardando as representaçoes dos simbolos dos bytes e sua sequencia
};

// cria um header e serializa a arvore linearmente obtendo o tamanho da arvore em relaçao a quantidade total de bytes representados nela, essa função precisa que passe o numero de lixo na arvore
Header *criar_header(NodeHuffman *raiz, int lixo)
{
    if(raiz == NULL) return NULL;

    Header *h = (Header *)malloc(sizeof(Header));
    if(h == NULL) return NULL;

    h->lixo = lixo;

    // serializa a arvore no buffer interno mapeando-a linearmente
    int pos = 0;
    serializar_arvore(raiz, h->arvore_buffer, &pos);
    h->tamanho_arvore = pos;

    return h;
}

// [ALTERADO] função a ser a primeira chamada na compactação, onde escrevemos nosso header no arquivo compactado
// agora grava os 2 bytes do cabecalho e os bytes da arvore diretamente via fputc (modo byte puro)
// conforme o formato do professor: byte1 = (lixo << 5) | (tam_arvore >> 8), byte2 = tam_arvore & 0xFF
// seguido dos bytes da arvore serializada diretamente, sem empacotamento bit a bit
void escrever_header(Header *h, FILE *f) // [ALTERADO] recebe FILE* em vez de BitFile*
{
    if(h == NULL || f == NULL) return;

    // monta os 2 bytes do cabecalho usando mascaras de bit conforme especificacao do professor
    // byte1: os 3 bits mais significativos guardam o lixo, os 5 menos guardam os bits altos do tamanho
    // byte2: guarda os 8 bits baixos do tamanho da arvore
    unsigned char byte1 = (unsigned char)((h->lixo << 5) | (h->tamanho_arvore >> 8));
    unsigned char byte2 = (unsigned char)(h->tamanho_arvore & 0xFF);

    fputc(byte1, f); // grava o 1o byte do cabecalho diretamente em modo byte
    fputc(byte2, f); // grava o 2o byte do cabecalho diretamente em modo byte

    // grava todos os bytes da arvore serializada diretamente via fputc, sem empacotamento bit a bit
    for(int i = 0; i < h->tamanho_arvore; i++)
    {
        fputc(h->arvore_buffer[i], f);
    }
}

// [ALTERADO] função que sera a primeira a ser chamada na descompactação
// agora le os 2 bytes do cabecalho e os bytes da arvore diretamente via fgetc (modo byte puro)
// obtem o numero de bits lixo e o tamanho da arvore e logo apos
// le todos os bytes que representam a arvore serializada em pre-ordem e reconstroi a arvore
NodeHuffman *ler_header(BitFile *bf, int *lixo_out, int *tam_arvore_out) // assinatura mantida para nao alterar decompress.c
{
    if(bf == NULL) return NULL;

    // [ALTERADO] acessa o FILE* interno do BitFile para ler o cabecalho em modo byte puro
    // o BitFile continua sendo usado para a leitura bit a bit dos dados apos o cabecalho
    FILE *f = get_arquivo(bf);
    if(f == NULL) return NULL;

    // le os 2 bytes do cabecalho diretamente via fgetc, conforme formato do professor
    int b1 = fgetc(f);
    int b2 = fgetc(f);
    if(b1 == EOF || b2 == EOF) return NULL; // medida de seguranca contra arquivos corrompidos

    // extrai o lixo dos 3 bits mais significativos do byte1
    *lixo_out = (b1 >> 5) & 0x07;

    // extrai o tamanho da arvore: 5 bits baixos do byte1 mais os 8 bits do byte2
    int tamanho_arvore = ((b1 & 0x1F) << 8) | (b2 & 0xFF);
    if(tamanho_arvore == 0) return NULL;
    *tam_arvore_out = tamanho_arvore; // armazena o tamanho da arvore para uso no decompress.c

    // [ALTERADO] le os bytes da arvore serializada diretamente via fgetc, sem leitura bit a bit
    unsigned char *buf = (unsigned char *)malloc(tamanho_arvore * sizeof(unsigned char));
    if(buf == NULL) return NULL;

    for(int i = 0; i < tamanho_arvore; i++)
    {
        int c = fgetc(f);
        if(c == EOF)
        {
            free(buf);
            return NULL; // medida de seguranca contra arquivos corrompidos
        }
        buf[i] = (unsigned char)c;
    }

    // reconstruindo a arvore a partir do buffer que representa ela serializada linearmente em pre-ordem
    // apos esse ponto o BitFile esta posicionado no primeiro byte dos dados comprimidos
    // e o read_bit ja pode ser usado normalmente pois o FILE* esta no lugar certo
    int pos = 0;
    NodeHuffman *raiz = desserializar_arvore(buf, &pos, tamanho_arvore);
    free(buf); // agora que ja reconstrui a arvore nao preciso mais dela

    return raiz;
}

// função para apagar apenas a estrutura do header sem apagar os dados usados
void destruir_header(Header *h)
{
    if(h != NULL) free(h);
}