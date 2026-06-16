#ifndef DECOMPRESS_H
#define DECOMPRESS_H

/**
 * @brief Estrutura opaca para o controlador de descompactação.
 * * Gerencia os caminhos dos arquivos compactados (origem) e do arquivo que 
 * será restaurado (destino).
 */
typedef struct decompressor Decompressor; // struct opaca para so ter cesso as propriedades no descompress.c

/**
 * @brief Cria e inicializa um objeto decompressor.
 * * @param arquivo_origem Caminho do arquivo compactado (ex: "imagem.huff").
 * @param arquivo_destino Caminho do arquivo onde o original será restaurado (ex: "imagem_restaurada.bmp").
 * @return Decompressor* Ponteiro para a estrutura alocada, ou NULL em caso de erro.
 */
Decompressor *criar_decompressor(const char *arquivo_origem, const char *arquivo_destino); // cria o objeto descompressor possuindo o arquivo comprimido no de origem e destino pos descompressão

/**
 * @brief Executa o algoritmo de descompactação de Huffman completo.
 * * Esta função realiza os seguintes passos:
 * 1. Abre o arquivo compactado através do BitFile.
 * 2. Lê o cabeçalho (Header) e recupera a tabela de frequências.
 * 3. Reconstrói a Árvore de Huffman idêntica à original.
 * 4. Abre o arquivo de destino para escrita comum.
 * 5. Navega na árvore bit a bit lido do BitFile até encontrar as folhas, 
 * gravando os bytes originais até reconstruir o arquivo completo.
 * * @param d Ponteiro para o controlador de descompactação.
 * @return int Retorna 1 em caso de sucesso ou 0 se ocorrer algum erro (arquivo corrompido, etc).
 */
int executar_decompressao(Decompressor *d); // controla todo o processo cronologico da descompressao utilizando funções ja implementadas

/**
 * @brief Libera a memória alocada para o controlador de descompactação.
 * * @param d Ponteiro para a estrutura que será desalocada.
 */
void destruir_decompressor(Decompressor *d); // destroi o decompressor sem destruir os dados utilizados

#endif