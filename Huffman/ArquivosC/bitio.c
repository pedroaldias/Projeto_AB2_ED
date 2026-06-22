#include <stdlib.h>
#include "../Headers/bitio.h"

struct bit_file
{
    FILE *arquivo; // ponteiro para arquivo padrao do C
    unsigned char buffer; // caixa de 1 byte onde acumulamos 8 bits
    int contador_bits; // contamos quantos bits estao ocupados no buffer de 0 a 7
    char modo; // alterna entre leitura 'r' (read) e escrita 'w' (write)

};

// abre o arquivo binario e inicializa o buffer; contador comeca em 8 (leitura) ou 0 (escrita)
BitFile *open_bit_file(const char *nome_arquivo, const char *modo)
{
    FILE *f = fopen(nome_arquivo, modo);
    if(f == NULL) return NULL;

    BitFile *bf = (BitFile *)malloc(sizeof(BitFile));
    if(bf == NULL)
    {
        fclose(f);
        return NULL;
    }

    bf->arquivo = f;
    bf->buffer = 0; // tudo inicializa zerado
    bf->modo = modo[0]; // salva se estou lendo 'r' ou escrevendo 'w'

    // se lendo, comeca em 8 para forçar a leitura do primeiro byte; se escrevendo, comeca em 0
    bf->contador_bits = (modo[0] == 'r') ? 8 : 0;

    return bf;
}

// envolve um FILE* ja aberto em um BitFile sem abrir um novo arquivo
// necessario para reusar o mesmo FILE* apos gravar o cabecalho em modo byte puro
BitFile *wrap_bit_file(FILE *f, const char *modo)
{
    if(f == NULL) return NULL;

    BitFile *bf = (BitFile *)malloc(sizeof(BitFile));
    if(bf == NULL) return NULL;

    bf->arquivo = f;
    bf->buffer = 0;
    bf->modo = modo[0];
    bf->contador_bits = (modo[0] == 'r') ? 8 : 0;

    return bf;
}

// retorna o FILE* interno do BitFile para permitir leitura em modo byte puro
// usado pelo ler_header para ler o cabecalho com fgetc antes de iniciar a leitura bit a bit
FILE *get_arquivo(BitFile *bf)
{
    if(bf == NULL) return NULL;
    return bf->arquivo;
}

// escreve um bit no buffer; ao completar 8 bits, descarrega o byte no arquivo
void write_bit(BitFile *bf, int bit)
{
    if(bf == NULL || bf->modo != 'w') return;

    // se o bit for 1, acende a posição correspondente no buffer via deslocamento
    if(bit == 1)
    {
        // preenche do bit mais significativo (7) para o menos significativo (0)
        bf->buffer |= (1 << (7 - bf->contador_bits));
    }

    bf->contador_bits++;

    // byte completo: grava no arquivo e reseta buffer e contador
    if(bf->contador_bits == 8)
    {
        fputc(bf->buffer, bf->arquivo);
        bf->buffer = 0;
        bf->contador_bits = 0;
    }
}

// lê um bit por vez; quando o buffer esgota, busca o próximo byte do arquivo
int read_bit(BitFile *bf)
{
    if(bf == NULL || bf->modo != 'r') return -1;

    // buffer esgotado (8 bits lidos): le o proximo byte do arquivo
    if(bf->contador_bits == 8)
    {
        int caractere = fgetc(bf->arquivo);
        if(caractere == EOF)
        {
            return -1; // fim do arquivo
        }
        bf->buffer = (unsigned char) caractere;
        bf->contador_bits = 0;
    }

    // extrai o bit atual via mascara (&1) apos deslocar a posicao de interesse para o bit menos significativo
    int bit = (bf->buffer >> (7 - bf->contador_bits)) & 1;
    bf->contador_bits++;

    return bit;
}

// comeplta os bits restantes (se houver) e fecha o arquivo, liberando o bf
void close_bit_file(BitFile *bf)
{
    if(bf == NULL) return;

    // se sobrou bit no buffer em modo escrita, completa o byte com zeros e grava
    if(bf->modo == 'w' && bf->contador_bits > 0)
    {
        fputc(bf->buffer, bf->arquivo);
    }

    fclose(bf->arquivo);
    free(bf);
}