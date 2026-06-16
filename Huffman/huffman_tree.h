#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include "heap.h"

/**
 * @brief Estrutura que representa um nó da Árvore de Huffman.
 * * Contém o caractere correspondente (relevante apenas para nós folha),
 * a frequência acumulada daquele caractere ou subárvore, e os ponteiros
 * para os filhos esquerdo e direito.
 */
typedef struct node_huffman NodeHuffman; // struct opaca para só ser possivel acessar as propriedades da minha struct em huffman_tree.c 

struct node_huffman
{
    // usamos unsigned char pois dessa maneira eu tenho um espaço que consegue suporta uma sequencai de 8 bits do 0 ao 255,
    // e todos os tipos de dados para uma maquina sao apenas sequencias de bytes(8 bits) logo desse modo eu consigo suportar qualquer tipo de dado com unsigned char
    // guarda byte na prática e nao somente chars e tbm em bytes onde o bit mais a esquerda é 1 se nao for unsigned ele sera negativo, oq causa problema ao usar na tabela de frequencia
    unsigned char caractere; 
    int frequencia; // frequencia que o byte em especifico apareceu
    NodeHuffman *esq;
    NodeHuffman *dir;
};

/**
 * @brief Função de comparação customizada para nós de Huffman.
 * * Esta função serve de "Juiz" para a Heap genérica. Ela converte os ponteiros
 * genéricos (void*) para o tipo NodeHuffman* e compara suas frequências.
 * * @param a Ponteiro genérico para o primeiro nó de Huffman.
 * @param b Ponteiro genérico para o segundo nó de Huffman.
 * @return int Retorna um valor negativo (< 0) se a frequência de 'a' for menor que a de 'b'.
 * Retorna um valor positivo (> 0) se a frequência de 'a' for maior que a de 'b'.
 * Retorna zero (0) se as frequências forem iguais.
 */
int comparar_nos_huffman(void *a, void *b); // novamente assim como a de comparar com heap caso o primeira seja maior retorna >0 caso contrario <0 e se forem iguais sera 0

/**
 * @brief Constrói a Árvore de Huffman a partir de uma tabela de frequências.
 * * A função cria nós folha para todos os caracteres presentes, insere-os em uma
 * Min-Heap, e executa o processo recursivo de extrair os dois mínimos, fundi-los
 * em um nó pai sintético e reinseri-lo até que reste apenas a raiz da árvore.
 * * @param frequencias Array de tamanho 256 contendo a contagem de cada caractere ASCII.
 * @return NodeHuffman* Retorna o ponteiro para a raiz da Árvore de Huffman gerada,
 * ou NULL caso não haja caracteres para codificar.
 */
NodeHuffman *construir_a_arvore(const int frequencias[]); // a partir de uma tabela de frequencia constroi a arvore de huffman antes inserindo em uma minheap e dps passa para arvore huffman
// frequencai possui tamanho para suporta o ascii completo, 256.

/**
 * @brief Gera recursivamente os códigos binários de Huffman para cada byte.
 *
 * Percorre a árvore de Huffman em pré-ordem. A cada nó, acumula '0' ao
 * ir para a esquerda e '1' ao ir para a direita. Ao atingir uma folha,
 * salva uma cópia da string acumulada no dicionário na posição do byte
 * correspondente.
 *
 * @param raiz      Ponteiro para a raiz (ou sub-raiz) da árvore de Huffman.
 * @param dicionario Array de 256 ponteiros onde cada posição [i] receberá
 *                   a string do código binário do byte i (ex: "1001").
 *                   As posições de bytes ausentes permanecem NULL.
 * @param caminho   Buffer temporário que acumula o caminho percorrido.
 *                   Deve ser alocado pelo chamador com pelo menos 257 bytes.
 * @param profundidade Nível atual na árvore, usado como índice no caminho.
 *                     Deve ser 0 na chamada inicial.
 */
void gerar_codigo_huffman(NodeHuffman *raiz, char *dicionario[256], char *caminho, int profundidade); // função que gera o codigo huffman de cada folha de byte

/**
 * @brief Libera a memória de todas as strings alocadas no dicionário.
 *
 * Percorre as 256 posições do dicionário e libera cada string que foi
 * alocada por gerar_codigo_huffman. Após a chamada, todos os ponteiros
 * do array ficam NULL.
 *
 * @param dicionario Array de 256 ponteiros para strings de código binário.
 */
void liberar_dicionario(char *dicionario[256]); // função para apagar da memoria o dicionario que é um array de strings com espaço para 256 strings do indice 0 ao 255

/**
 * @brief Serializa a árvore de Huffman em pré-ordem para um buffer de bytes.
 *
 * Nós internos geram o byte '*'. Nós folha geram o byte '\' seguido do
 * símbolo. Se o símbolo for '*' ou '\', ele é escapado com '\' antes.
 *
 * @param raiz     Ponteiro para a raiz da árvore.
 * @param buf      Buffer de saída onde os bytes serão escritos.
 * @param pos      Ponteiro para o índice atual no buffer (atualizado pela função).
 */
void serializar_arvore(NodeHuffman *raiz, unsigned char *buf, int *pos);
// serializando a arvore em pré-ordem com a especificaçoes requisitadas convertendo minha estrutura de dados em uma sequencia linear de bytes

/**
 * @brief Reconstrói a árvore de Huffman a partir de um buffer serializado.
 *
 * Lê o buffer seguindo a mesma convenção de serializar_arvore.
 * '*' indica nó interno; '\' indica que o próximo byte é o símbolo da folha.
 *
 * @param buf      Buffer de entrada com os bytes da árvore serializada.
 * @param pos      Ponteiro para o índice atual no buffer (atualizado pela função).
 * @param tamanho  Tamanho total do buffer em bytes.
 * @return NodeHuffman* Raiz da árvore reconstruída, ou NULL se o buffer for inválido.
 */
NodeHuffman *desserializar_arvore(const unsigned char *buf, int *pos, int tamanho); // desserializa a arvore pegando a escrita linear e transformando em uma arvore huffman de volta

void destruir_arvore_huffman(NodeHuffman *raiz); // destroi a arvore de huffman da memoria

#endif