#ifndef BITIO_H
#define BITIO_H

#include <stdio.h>

/**
 * @brief Estrutura opaca para controle de leitura e escrita de bits em arquivos.
 * * Encapsula o arquivo real, o buffer de 1 byte (8 bits) e o contador
 * de bits para que o usuário não precise manipular máscaras binárias manualmente.
 */
typedef struct bit_file BitFile; // struct opaca assim so tem acesso as propriedades da struct no arquivo bitio.c

/**
 * @brief Abre um arquivo para leitura ou escrita em nível de bits.
 * * @param nome_arquivo Caminho do arquivo que será aberto.
 * @param modo String determinando o modo ("rb" para leitura biária, "wb" para escrita binária).
 * @return BitFile* Ponteiro para a estrutura de controle criada, ou NULL em caso de erro.
 */
BitFile* open_bit_file(const char *nome_arquivo, const char *modo); // abre um arquivo para ler os bits dentro ou para escrever bits nesse arquivo

/**
 * @brief Escreve um único bit (0 ou 1) no arquivo.
 * * O bit é armazenado temporariamente em um buffer interno. Quando o buffer
 * acumula 8 bits, ele é gravado fisicamente no arquivo como 1 byte.
 * * @param bf Ponteiro para a estrutura de controle do arquivo de bits.
 * @param bit O valor do bit a ser escrito (obrigatoriamente 0 ou 1).
 */
void write_bit(BitFile *bf, int bit); //escreve um unico bit por vez e acumula em um buffer que suporta 8 bits, quando esse buffer esta cheio ele é gravado no arquivo como 1 byte fisicamente

/**
 * @brief Lê um único bit do arquivo.
 * * Se o buffer interno estiver vazio, a função lê o próximo byte inteiro do arquivo
 * e vai devolvendo os bits um a um a cada chamada.
 * * @param bf Ponteiro para a estrutura de controle do arquivo de bits.
 * @return int Retorna o bit lido (0 ou 1), ou -1 se atingir o fim do arquivo (EOF).
 */
int read_bit(BitFile *bf); // Lê um unico bit por vez do arquivo, se o buffer interno esiver vazio ele le o proximo byte completo e devolve os bits 1 a 1 em cada chamada

/**
 * @brief Fecha o arquivo de bits e libera a memória alocada.
 * * IMPORTANTE: No modo de escrita, esta função realiza o "flush", garantindo que
 * se restarem bits incompletos no buffer (ex: apenas 3 bits), eles sejam completados
 * com zeros até formar 1 byte e salvos no arquivo antes de fechar.
 * * @param bf Ponteiro para a estrutura que será fechada e desalocada.
 */
void close_bit_file(BitFile *bf); // fecha o arquivo de bit e libera memoria alocada evitando memory leak, contudo no modo de escrita essa função tbm completa bytes com zeros caso necessite

#endif