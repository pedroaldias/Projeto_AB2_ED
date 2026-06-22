#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Headers/heap.h"

struct heap
{
    int capacidade;   // capacidade maxima de elementos na heap
    int tamanho;      // quantidade atual de elementos na heap
    void **dados;     // array de ponteiros genéricos (array de void *) de dados (para suporta a compactação e descompactação de dados genéricos)
    comp_fn comparar; // a função que sabe comparar os valores do void*
};

Heap* criar_heap(int capacidade, comp_fn comparar)
{
    Heap *h = (Heap *)malloc(sizeof(Heap));
    h->capacidade = capacidade;
    h->tamanho = 0;

    // aloca o array de void* com a capacidade desejada
    h->dados = (void **)malloc(capacidade*sizeof(void *));
    h->comparar = comparar; // função de comparação que define a ordenação da heap

    return h;
}

int tamanho_heap(Heap *h)
{
    return h->tamanho;
}

// troca dois ponteiros de lugar dentro do array de void*
static void trocar(void **a, void **b)
{
    void *temp = *a;
    *a = *b;
    *b = temp;
}

// sobe um elemento na heap ate ele encontrar seu lugar correto entre pai e filhos
static void sift_up(Heap *h, int i)
{
    while(i > 0)
    {
        int pai = (i - 1)/ 2; // encontra o pai do nó atual

        // heap min: se o filho tem menor frequencia que o pai(retorno < 0), eles trocam
        if(h->comparar(h->dados[i], h->dados[pai]) < 0)
        {
            trocar(&h->dados[i], &h->dados[pai]);
            i = pai; // continua subindo
        }
        else break; // ja respeita a propriedade da heap
    }
}

// desce o elemento no indice 'i' ate seu devido lugar, restaurando a propriedade da heap min
static void sift_down(Heap *h, int i)
{
    int menor = i;
    int esq   = (i*2) + 1; // filho a esquerda de um no
    int dir   = (i*2) + 2; // filho a direita de um no

    // verifica se a esquerda existe e tem menor frequencia que o atual menor(retorno < 0)
    if(esq < h->tamanho && h->comparar(h->dados[esq], h->dados[menor]) < 0)
    {
        menor = esq;
    }
    // verifica se a direita existe e tem menor frequencia que o atual menor
    if(dir < h->tamanho && h->comparar(h->dados[dir], h->dados[menor]) < 0)
    {
        menor = dir;
    }

    // se houve troca, repete o processo no ramo afetado
    if(menor != i)
    {
        trocar(&h->dados[i], &h->dados[menor]);
        sift_down(h, menor);
    }
}

// insere um novo no na ultima posição livre e usa sift_up para reposicioná-lo
void inserir_heap(Heap *h, void *dado)
{
    if(h->tamanho >= h->capacidade)
    {
        printf("Erro: Heap cheia\n");
        return;
    }

    h->dados[h->tamanho] = dado;
    h->tamanho++;
    sift_up(h, h->tamanho-1);
}

// extrai o elemento de menor frequencia (raiz) e revalida a heap com sift_down
void *extrair_min_heap(Heap *h)
{
    if(h->tamanho == 0)
    {
        printf("Erro: Heap vazia\n");
        return NULL;
    }

    // na heap min, o menor elemento sempre esta na raiz
    void *minimo = h->dados[0];

    // move o ultimo elemento para a raiz e ajusta o tamanho
    h->dados[0] = h->dados[h->tamanho-1];
    h->tamanho--;

    // sift_down restaura a propriedade da heap a partir da raiz
    sift_down(h, 0);

    return minimo;
}

// destroi apenas a estrutura da heap; os nós internos pertencem a arvore huffman
void destruir_heap(Heap *h)
{
    if(h)
    {
        free(h->dados);
        free(h);
    }
}