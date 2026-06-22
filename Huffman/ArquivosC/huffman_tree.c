#define _POSIX_C_SOURCE 200809L // mudar o padrao do C para aceitar a função strdup
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Headers/huffman_tree.h"
#include "../Headers/heap.h"

// compara a frequencia entre dois nós do tipo NodeHuffman
int comparar_nos_huffman(void *a, void *b)
{
    NodeHuffman *noA = (NodeHuffman *)a;
    NodeHuffman *noB = (NodeHuffman *)b;

    return (noA->frequencia - noB->frequencia); // <0 se noA for menor, >0 se for maior, 0 se iguais
}

// cria um nó isolado com seu caractere e frequencia
static NodeHuffman *criar_no(unsigned char caractere, int frequencia)
{
    NodeHuffman *novo_no = (NodeHuffman *)malloc(sizeof(NodeHuffman));
    if(novo_no == NULL)
    {
        printf("Erro: Falha na alocacao de memoria para o no\n");
        exit(1);
    }

    novo_no->caractere = caractere;
    novo_no->frequencia = frequencia;
    novo_no->esq = NULL;
    novo_no->dir = NULL;

    return novo_no;
}

// constroi a arvore de huffman a partir da tabela de frequencias
NodeHuffman *construir_a_arvore(const int frequencias[])
{
    // conta quantos bytes distintos existem (no maximo 256, pois 2^8 possibilidades)
    int bytes_unicos = 0;
    for(int i = 0; i < 256; i++)
    {
        if(frequencias[i] > 0) bytes_unicos++;
    }

    if(bytes_unicos == 0) return NULL;

    // cria a heap min generica com a função comparadora de frequencias
    Heap *heap = criar_heap(bytes_unicos, comparar_nos_huffman);

    // cria os nós folha de cada byte presente e insere na heap
    for(int i = 0; i < 256; i++)
    {
        if(frequencias[i] > 0)
        {
            // usa o i como referencia ascii apenas para identificar o byte; nao viola o huffman
            NodeHuffman *folha = criar_no((unsigned char)i, frequencias[i]);

            inserir_heap(heap, folha);
        }
    }

    // caso especial: apenas um byte distinto no arquivo
    if(tamanho_heap(heap) == 1)
    {
        // cria um pai sintetico (byte 0) com o unico no a esquerda, garantindo
        // que a compressao sempre caminhe para 0 e evitando segfault na descompressao
        NodeHuffman *unico = (NodeHuffman *) extrair_min_heap(heap);
        NodeHuffman *pai = criar_no(0, unico->frequencia);
        pai->esq = unico;
        pai->dir = NULL;
        return pai;
    }

    // laço principal: funde os dois nós de menor frequencia até sobrar so a raiz
    while(tamanho_heap(heap) > 1) // O(n) pois cada iteração remove 2 e insere 1
    {
        // extrai os dois nós de menor frequencia
        NodeHuffman *esq = (NodeHuffman *) extrair_min_heap(heap);
        NodeHuffman *dir = (NodeHuffman *) extrair_min_heap(heap);

        // cria o nó pai sintetico com a soma das frequencias dos filhos
        NodeHuffman *pai = criar_no(0, esq->frequencia + dir->frequencia);
        pai->esq = esq;
        pai->dir = dir;

        // reinsere o pai na heap, formando a arvore a cada iteração
        inserir_heap(heap, pai);
    }

    // o ultimo nó restante na heap é a raiz da arvore
    NodeHuffman *raiz = (NodeHuffman *) extrair_min_heap(heap);

    // desaloca a heap, ja nao e mais necessaria
    destruir_heap(heap);

    return raiz;
}

// constroi o codigo huffman de cada byte distinto lido
void gerar_codigo_huffman(NodeHuffman *raiz, char *dicionario[256], char *caminho, int profundidade)
{
    if(raiz == NULL) return;

    // folha: nao possui esquerda nem direita, qualquer byte pode estar aqui (incluindo '*' e '\')
    if(raiz->esq == NULL && raiz->dir == NULL)
    {
        caminho[profundidade] = '\0';
        dicionario[(unsigned char)raiz->caractere] = strdup(caminho);
        return;
    }

    // desce para esquerda acrescentando '0' no caminho
    caminho[profundidade] = '0';
    gerar_codigo_huffman(raiz->esq, dicionario, caminho, profundidade + 1);

    // desce para direita acrescentando '1' no caminho
    caminho[profundidade] = '1';
    gerar_codigo_huffman(raiz->dir, dicionario, caminho, profundidade + 1);
}

// destroi apenas a estrutura do dicionario
void liberar_dicionario(char *dicionario[256])
{
    for(int i = 0; i < 256; i++)
    {
        if(dicionario[i] != NULL)
        {
            free(dicionario[i]);
            dicionario[i] = NULL; // evita ponteiro solto apos o free
        }
    }
}

// serializa a arvore em pré-ordem: no interno = '*', folha comum = byte direto,
// folha especial ('*' ou '\') = '\' + byte (escape). Ex: **CB***FEDA
void serializar_arvore(NodeHuffman *raiz, unsigned char *buf, int *pos)
{
    if(raiz == NULL) return;

    if(raiz->esq != NULL || raiz->dir != NULL)
    {
        buf[(*pos)++] = '*';
        serializar_arvore(raiz->esq, buf, pos);
        serializar_arvore(raiz->dir, buf, pos);
    }
    else
    {
        // so usa escape '\' se o byte for '*' ou '\'; demais folhas gravam direto
        if(raiz->caractere == '*' || raiz->caractere == '\\')
        {
            buf[(*pos)++] = '\\';
        }
        buf[(*pos)++] = raiz->caractere;
    }
}

// desserializa o buffer linear de volta para a arvore, tratando nó interno '*',
// folha com escape '\' e folha direta (byte comum)
NodeHuffman *desserializar_arvore(const unsigned char *buf, int *pos, int tamanho)
{
    if(*pos >= tamanho) return NULL;

    unsigned char byte = buf[(*pos)++];

    if(byte == '*')
    {
        // nó interno: reconstroi recursivamente os filhos
        NodeHuffman *no = criar_no(0, 0);
        no->esq = desserializar_arvore(buf, pos, tamanho);
        no->dir = desserializar_arvore(buf, pos, tamanho);
        return no;
    }
    else if(byte == '\\')
    {
        // escape: o proximo byte e o simbolo real da folha
        if(*pos >= tamanho) return NULL;
        unsigned char simbolo = buf[(*pos)++];
        return criar_no(simbolo, 0);
    }
    else
    {
        // byte comum: folha direta, sem escape
        return criar_no(byte, 0);
    }
}

// desaloca a arvore em pos-ordem (filhos antes do pai)
void destruir_arvore_huffman(NodeHuffman *raiz)
{
    if(raiz == NULL) return;

    destruir_arvore_huffman(raiz->esq);
    destruir_arvore_huffman(raiz->dir);

    free(raiz);
}