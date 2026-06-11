#define _POSIX_C_SOURCE 200809L // mudar o padrao do C para aceitar a função strdup
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman_tree.h"
#include "heap.h"

// compara a frequencai entre dois nós do tipo NodeHuffman
int comparar_nos_huffman(void *a, void *b)
{
    // escreve os void como NodeHuffman para nossa função
    NodeHuffman *noA = (NodeHuffman *)a;
    NodeHuffman *noB = (NodeHuffman *)b;

    return (noA->frequencia - noB->frequencia); // caso noB seja mais frequente o retorno é <0, se noA for maior o retorno sera >0, se forem iguais o retorno é zero
}

// criamos um nó do tipo huffman isolado inicialmente apenas com seu caractere e frequencia
static NodeHuffman *criar_no(unsigned char caractere, int frequencia)
{
    NodeHuffman *novo_no = (NodeHuffman *)malloc(sizeof(NodeHuffman));
    if(novo_no == NULL)
    {
        printf("Erro: Falha na alocacao de memoria para o no\n");
        exit(1);
    }

    // defini as caracteres de um nó isolado com os parametros passados
    novo_no->caractere = caractere;
    novo_no->frequencia = frequencia;
    novo_no->esq = NULL;
    novo_no->dir = NULL;

    return novo_no;
}

// função que ira construir nossa arvore huffman propriamente dita
NodeHuffman *construir_a_arvore(const int frequencias[]) // possivel bug nessa função pela questao de como é aboradado o no interno @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
{
    // 1. primeiro quantos caracteres/bytes distintos possuimos , todo tipo de arquivo é formado por umas combinação ou nao de no maximo 256 bytes distintos
    // sao 256 bytes distintos no maximo para todo arquivo pois fisicamente o computador so le 1 byte por vez formado por 8 bits que por sua vez possuem 2 estados possiveis logo
    // 2^8 possibilidades para formar um byte, sendo assim no maximo 256 bytes distintos em qualquer arquivo
    int bytes_unicos = 0;
    for(int i = 0; i < 256; i++)
    {
        if(frequencias[i] > 0) bytes_unicos++;
    }

    if(bytes_unicos == 0) return NULL; 

    // 2. cria uma heap min generica passando nossa função comparadora, nao adicionamos nada na heap ainda, apenas passamos o tamanho limite da heap min e a função comparadora
    Heap *heap = criar_heap(bytes_unicos, comparar_nos_huffman);

    // 3. Monta os nós folha iniciais e joga eles dentro da heap min, cria o nós dos bytes e coloca eles na heap
    for(int i = 0; i < 256; i++)
    {
        if(frequencias[i] > 0)
        {
            // crio um no do tipo huffman passando o 

            // passo o i como seu equivalente em 1 byte no codigo ascii e isso nao viola o huffman essa operação é apenas para o codigo 
            // ter uma referencia do que isso é em ascii para depois forma o codigo huffman equivalente
            // e apesar de usar ascii para guardar o i ele pode mapear bytes de musica e imagens, isso é apenas uma questao de ponto de referencia para o codigo e nao do byte em si
            NodeHuffman *folha = criar_no((unsigned char)i, frequencias[i]);

            inserir_heap(heap, folha); // coloca cada folha com frequencia maior que zero na heap min
        }
    }

    //caso o arquivo tenha 1 unico byte distinto precisa ser tratado como um caso especifal
    if(tamanho_heap(heap) == 1)
    {
        // caso tenha um no unico eu crio ele retirando da heap e crio um no pai de byte 0 com a frequencai desse unico sendo um sintetico e faço o unico estar a esquerda,
        // sendo sempre ignorado e caindo para a compressão na direção do 0, logo garantido que na oacha segmantaion fault na descompressao
        NodeHuffman *unico = (NodeHuffman *) extrair_min_heap(heap);
        NodeHuffman *pai = criar_no(0, unico->frequencia);
        pai->esq = unico;
        pai->dir = NULL;
        return pai;
    }

    // 4. laço principal do algoritmo de huffman, o que realmente monta a arvore na memoria
    while(tamanho_heap(heap) > 1) // realiza essa operação ate sobrar um unico no na heap que sera a raiz da arvore
    {
        // como retiramos 2 por vez mas adicionamos um novo no na heap logo dps entao esse while se torna O(n)

        // extrai os dois nós de menor frequencia da heap, apos cada extração o sift_down age para valdiar novamente a heap min
        NodeHuffman *esq = (NodeHuffman *) extrair_min_heap(heap);
        NodeHuffman *dir = (NodeHuffman *) extrair_min_heap(heap);

        // cria um nó pai sintetico composto pela soma das frequencias dos filhos
        NodeHuffman *pai = criar_no(0, esq->frequencia + dir->frequencia); // passei a referencia do nó sintetico como sendo o meu '*' e somei a frequencia dos filhos para passar pro pai
        // conectando o pai aos filhos
        pai->esq = esq;
        pai->dir = dir;

        // insere novamente o no pai na heap(sif_up age aqui procurando o local ideal para estar a partir do ultimo indice valido)
        inserir_heap(heap, pai);
        // ao reincerir o pai na heap nos temos um ponteiro agora que possui esquerda e direita ou seja fazendo 
        // esse processo repetidas vezes inevitavelmente ja estamos formando as conexões da nossa arvorecada vez que tiramos e reinserimos da nossa heap min
    }


    // 5. o ultimo nó que sobrou na nossa heap sera justamente a raiz da nossa arvore huffman
    NodeHuffman *raiz = (NodeHuffman *) extrair_min_heap(heap); // so existe um elemento na heap min neste momento

    // 6. desaloca a nossa heap para evitar memory leak
    destruir_heap(heap);

    return raiz;
}

void gerar_codigo_huffman(NodeHuffman *raiz, char *dicionario[256], char *caminho, int profundidade)
{
    if(raiz == NULL) return;

    // verificação que se estamos numa folha logo o no huffman atual nao possuo esquerda nem direita
    // desse modo verificamos uma folha idenpende do caractere de saida podendo ser qualquer byte do arquivo original, incluindo '*', '\', '#'
    if(raiz->esq == NULL && raiz->dir == NULL)
    {
        caminho[profundidade] = '\0';

        dicionario[(unsigned char)raiz->caractere] = strdup(caminho);
        // faço o cast para unsigned char apenas por questao de documentação para quem ler pois o caractere do tipo NodeHuffman ja é um unsigned char

        return;
    }

    // a partir daqui eu faço todos os caminhos possiveis da minha arvore e reescrevendo o caminho quando volto de uma folha

    // desce para esquerda acrescentando '0' no caminho
    caminho[profundidade] = '0';
    gerar_codigo_huffman(raiz->esq, dicionario, caminho, profundidade + 1);

    // desce para direita acrescentando '1' no caminho
    caminho[profundidade] = '1';
    gerar_codigo_huffman(raiz->dir, dicionario, caminho, profundidade + 1);
}

void liberar_dicionario(char *dicionario[256])
{
    for(int i = 0; i < 256; i++)
    {
        if(dicionario[i] != NULL)
        {
            free(dicionario[i]);
            dicionario[i] = NULL; // boa pratica para evitar ponteiro solto apos o free
        }
    }
}

// função que transforma uma arvore em uma sequencia linear de bytes, serializando nossa arvore
void serializar_arvore(NodeHuffman *raiz, unsigned char *buf, int *pos)
{
    if(raiz == NULL) return;

    // no interno: grava como '*'
    if(raiz->esq != NULL || raiz->dir != NULL)
    {
        buf[(*pos)++] = '*';
        serializar_arvore(raiz->esq, buf, pos);
        serializar_arvore(raiz->dir, buf, pos);
    }
    else
    {
        // no folha grava: '\' + simbolo de representação
        // se o simbolo for \ ou *, se faz necessario o escape de '\' antes para definir com sendo um byte real, para n criar confusão
        // com marcador de no interno na leitura
        buf[(*pos)++] = '\\';
        buf[(*pos)++] = raiz->caractere; // grava o byte indepente de qual for
    }
}

// função que pega uma serialização linear de arvore e transforma novamente em uma arvore de huffman atraves da recurssão
NodeHuffman *desserializar_arvore(const unsigned char *buf, int *pos, int tamanho)
{
    if(*pos >= tamanho) return NULL;

    unsigned char byte = buf[(*pos)++];

    if(byte == '*')
    {
        // o byte atual representa um no interno
        NodeHuffman *no = criar_no(0, 0);
        no->esq = desserializar_arvore(buf, pos, tamanho);
        no->dir = desserializar_arvore(buf, pos, tamanho);
        return no;
    }
    else if(byte == '\\')
    {
        // o byte atual é uma folha
        if(*pos >= tamanho) return NULL;
        unsigned char simbolo = buf[(*pos)++];
        return criar_no(simbolo, 0);
    }

    // caso ocorra um byte inesperado que n caia em nenhum dos casos anteriores sendo um arquivo corrompido
    return NULL;
}

// função para desalocar da memoria a arvore huffman
void destruir_arvore_huffman(NodeHuffman *raiz)
{
    if(raiz == NULL) return;
    
    // liberação da memoria em pos-ordem: primeiro os filhos depois o pai chamando recursivamente
    destruir_arvore_huffman(raiz->esq);
    destruir_arvore_huffman(raiz->dir);
    // pos-ordem pois ele chama ate a aesuqerda e so libera caso ele ande na direita e tbm encontre null se nao ele continua empilhando chamadas recursivas

    free(raiz);
}