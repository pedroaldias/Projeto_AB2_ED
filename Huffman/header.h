#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include "huffman_tree.h"
#include "bitio.h"

/**
 * @brief Estrutura opaca do cabeçalho do arquivo .huff.
 *
 * Armazena os bits de lixo, o tamanho da árvore serializada
 * e o buffer com os bytes da árvore em pré-ordem.
 * Formato: [3 bits lixo][13 bits tamanho árvore][bytes árvore]
 */
typedef struct header Header; // struct opaca para so ter acesso as propriedades de heade dentro do header.c

/**
 * @brief Cria o cabeçalho serializando a árvore e registrando o lixo.
 *
 * @param raiz  Raiz da árvore de Huffman já construída.
 * @param lixo  Quantidade de bits de lixo no último byte dos dados (0-7).
 * @return Header* Ponteiro para o cabeçalho criado, ou NULL em caso de erro.
 */
Header *criar_header(NodeHuffman *raiz, int lixo); // função para criar um header de uma arvore huffman e a quantidade de bits lixo no ulitmo byte de dados(que foram usados para completar)

/**
 * @brief Escreve o cabeçalho no início do arquivo compactado.
 *
 * Grava 2 bytes de metadados seguidos pelos bytes da árvore serializada.
 * Usa big-endian para garantir interoperabilidade entre arquiteturas.
 *
 * @param h  Ponteiro para o cabeçalho.
 * @param f  Arquivo de destino aberto em modo binário ("wb").
 */
void escrever_header(Header *h, BitFile *bf); // escreve um header dentro de um arquivo que sera nosso arquivo de bytes compactado

/**
 * @brief Lê o cabeçalho de um arquivo compactado e reconstrói a árvore.
 *
 * @param f        Arquivo compactado aberto em modo binário ("rb").
 * @param lixo_out Ponteiro onde será gravada a quantidade de bits de lixo.
 * @return NodeHuffman* Raiz da árvore reconstruída, ou NULL em caso de erro.
 */
NodeHuffman *ler_header(BitFile *bf, int *lixo_out, int *tam_arvore_out); // captura todas as informações do header de um arquivo compactado seja o tamanho do lixo, tamanho da arvore e sua serialização, reconstruindo ela

/**
 * @brief Libera a memória alocada para o cabeçalho.
 *
 * @param h Ponteiro para o cabeçalho a ser destruído.
 */
void destruir_header(Header *h); // função para desalocar o header da memoria

#endif