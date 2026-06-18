#ifndef COMPRESS_H
#define COMPRESS_H

/**
 * @brief Estrutura opaca para o controlador de compressão.
 * * Centraliza os caminhos dos arquivos de entrada e saída, além de gerenciar
 * o estado interno do processo de compressão.
 */
typedef struct compressor Compressor; // struct opaca para so ter acesso as propriedades no compress.c

/**
 * @brief Cria e inicializa um objeto compressor.
 * * @param arquivo_origem Caminho do arquivo que será compactado (ex: "imagem.bmp").
 * @param arquivo_destino Caminho do arquivo compactado final (ex: "imagem.huff").
 * @return Compressor* Ponteiro para a estrutura alocada, ou NULL em caso de erro.
 */
Compressor *criar_compressor(const char *arquivo_origem, const char *arquivo_destino); // cria e inicializa o objeto compressor com o arquivo original e onde ira depositar o comprimido

/**
 * @brief Executa o algoritmo de compressão de Huffman completo.
 * * Esta função realiza os seguintes passos:
 * 1. Abre o arquivo de origem e conta a frequência dos bytes.
 * 2. Monta a Heap e a Árvore de Huffman.
 * 3. Cria os novos códigos binários para cada byte.
 * 4. Abre o arquivo de destino e grava o cabeçalho (Header).
 * 5. Traduz o conteúdo do arquivo original para bits e grava no destino.
 * * @param c Ponteiro para o controlador de compressão.
 * @return int Retorna 1 em caso de sucesso ou 0 se ocorrer algum erro (arquivo não encontrado, etc).
 */
int executar_compressao(Compressor *c); // função que realiza toda a ordem cronologica da compressão e utiiza dos nossos demais arquivos

/**
 * @brief Libera a memória alocada para o controlador de compressão.
 * * @param c Ponteiro para a estrutura que será desalocada.
 */
void destruir_compressor(Compressor *c); // destroi o compressor sem destruir o dados utilizados

#endif