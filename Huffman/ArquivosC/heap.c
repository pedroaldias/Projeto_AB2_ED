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

    //aloca o array de void * capaz de alocar a capacidade desejadad
    h->dados = (void **)malloc(capacidade*sizeof(void *));
    h->comparar = comparar; //recebe a função de comparação passada que será utilizada para definir como a heap opera

    return h;
}

int tamanho_heap(Heap *h)
{
    return h->tamanho;
}

// troca dois ponteiros de lugar dentro do array de void *
static void trocar(void **a, void **b)
{
    // o ponteiro temp recebe o conteudo de a que é um ponteiro na memoria,
    // o conteudo de a vira o conteudo de b outro ponteiro na memoria e o conteudo de b vira o conteudo de a que estava na variavel temp,
    // funciona pois a copia do parametro é do ponteiro que guarda o ponteiro entao quando mexe no conteudo mexe no ponteiro em si
    void *temp = *a;
    *a = *b;
    *b = temp;
}

// sift_up sobe um elemento na array da heap ate ele esta num local adequado para ser pai dos filhos atuais
// eu so troco entre o filho e o pai nao troco com o esquerda nem direita pois isso nao invalida a propriedade da heap onde a frequencia a relação é entre pai e filhos e nao entre filhos
static void sift_up(Heap *h, int i)
{
    while(i > 0)
    {
        int pai = (i - 1)/ 2; // encontra o pai do nó atual

        // se o filho (nó atual) tiver menor frequencia que o pai eles trocam
        // pois por ser uma heap min eu coloco os itens da menor frequencia no topo ate a maior frequencia descendo na heap
        if(h->comparar(h->dados[i], h->dados[pai]) < 0) // pai tem maior frequencia que o filho pois a função retornar <0 se o segundo item for maior que o primeiro
        {
            trocar(&h->dados[i], &h->dados[pai]);
            i = pai; // continua o processo até ou não ser possivel pois i se tornou negativo ou ate encotrar o local adequado onde i tem maior frequencia que o pai
        }
        else break; // se o i atual tiver maior frequencia que o pai apenas para
    }
}

// sift_down desce o elemento no indice 'i' até o seu devido, aqui usaremos ele para descer o ultimo elemento, assim garantindo que apos uma mudança o array continue uma heap
static void sift_down(Heap *h, int i)
{
    int menor = i;
    int esq   = (i*2) + 1;
    int dir   = (i*2) + 2;

    // testo se o indice a esquerda existe na heap e se ele tem menor frequencia do que o atual indice menor
    if(esq < h->tamanho && h->comparar(h->dados[esq], h->dados[menor]) < 0)
    {
        menor = esq;
    }
    // testo se o indice a direita existe na heap e se ele tem menor frequencia do que o atual indice menor, que pode ser o indice i original ou o indice a esquerda
    if(dir < h->tamanho && h->comparar(h->dados[dir], h->dados[menor]) < 0)
    {
        menor = dir;
    }

    // se o menor for diferente de i preciso aplicar o swap e testar dnv para ele o sift_down para respeitar a propriedade da heap min do pai ter menor frequencia que os filhos sempre
    if(menor != i)
    {
        trocar(&h->dados[i], &h->dados[menor]);
        sift_down(h, menor); // chama recursivamente para garantir que o ramo afetado é valido
    }
}

// inserindo um no na heap onde assumo que ele esta no indice atual disponivel, imagina uma queue, e mudo ele de posição na array de acordo com a propriedade da heap
void inserir_heap(Heap *h, void *dado)
{
    if(h->tamanho >= h->capacidade)
    {
        printf("Erro: Heap cheia\n");
        return;
    }

    //coloca um novo no da heap na posição atual livre pois desse modo podemos usar o sift_up para encontrar o local adequado desse no na heap
    h->dados[h->tamanho] = dado;
    h->tamanho++;
    // usando o sift_up ele vai subir na heap ate o local onde se torna um nó valido
    sift_up(h, h->tamanho-1);
}


// extrai o elemento de menor frequencia, desvalidando a heap min mas logo apois valida ela atraves do preenchimento da lacuna e a manunteção da invalidez local dos no por sift_down
void *extrair_min_heap(Heap *h)
{
    if(h->tamanho == 0)
    {
        printf("Erro: Heap vazia\n");
        return NULL;
    }

    // por usarmos uma heap min, o elemento de menor frequencia sempre esta na raiz ou topo da array
    void *minimo = h->dados[0];

    // escolho o ultimo elemento da heap para caminhar nela reorganizando a heap e garantindo que ela é valida pois ira reorganizar os nós em relação a sua interação de frequencia
    h->dados[0] = h->dados[h->tamanho-1];
    h->tamanho--; // retirei o elemento minimo preciso ajustar o tamanho

    // como as mini heaps são validas o sift_down ira garantir localmente que o nó atual é uma heap e refazer essse processo até que ele encontre um nó que respeite a condição de heap min
    // ligitimando totalmente a heap pois os que eram validos não foram alterados e validou todos que obtiveram erros lcoalmente
    sift_down(h, 0);

    return minimo;
}

// destroi apenas a estrutura da heap e não os dados internos nos nó pois eles são usados na arvore huffman
void destruir_heap(Heap *h)
{
    if(h)
    {
        free(h->dados); //destroi o ponteiro da heap
        free(h); // destroi o endereço da heap
    }
}