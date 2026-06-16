#include <stdlib.h>
#include "bitio.h"

struct bit_file
{
    FILE *arquivo; // ponteiro para arquivo padrao do C
    unsigned char buffer; // caixa de 1 byte onde acumulamos 8 bits
    int contador_bits; // contamos quantos bits estao ocupados no buffer de 0 a 7
    char modo; // alterna entre leitura 'r' (read) e escrita 'w' (write)

};

// nessa função nos definimos nosso arquivo binario, como se estivessemos criando ele, definindo o modo que sera usado, que por enquanto o buffer esta vazio e se o contador de bits e 0 ou 8
// dependendo se estou lendo, 8 bits pois ja existem e eu leio 1 byte completo diretamente, ou escrevendo, 0 bits pois eles nao existe ainda.
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
    bf->buffer = 0; // como nao comecei a fazer nada ainda tudo inicializa como zero
    bf->modo = modo[0]; // salva se estou lendo 'r' ou escrevendo 'w'

    // se eu estou lendo meu contador de bits ele tem descrever pois obviamente eu leio bits que ja existem logo o contador precisa começar como 8 bits, lendo o primeiro byte imediatamente
    // caso contrario se eu estou escrevendo meu contador precisa começar como zero pois esses bits nao existem e eu os escreverei
    bf->contador_bits = (modo[0] == 'r') ? 8 : 0;

    return bf;
}

// função para escrever bit a bit o buffer, representação do byte em formação, um byte e cada vez que completa um byte joga ele para nosso arquivo
void write_bit(BitFile *bf, int bit)
{
    if(bf == NULL || bf->modo != 'w') return;

    // Se o bit for 1, precisamos "acender" o bit correto no vuffer usando deslocamente, bit_shift a esquerda, (<<)
    if(bit == 1)
    {
        // preenchemos do mais signifivatico para o menos por conveções computacionais que adotam esse metodo para representar um byte e tbm pela questao da leitura humana
        bf->buffer |= (1 << (7 - bf->contador_bits)); // caso tenha zero bits ele acende o bit numero 7, se tenho 1 acende o numero 6 e assim por diante ate ter 7 bits e acender o 0
        // considerando que o primeiro bit é o mais significativo, sendo o numero 7
        // |= é o equivalente a += sendo um OR bitwise que na pratica funciona como juntar todos os bits que aparecem em duas sequencias de bits ou o 1 sempre prevalece.
    }

    bf->contador_bits++;

    // caso tenhamos completado um byte, 8 bits, descarregamos ele no nosso arquivo, sendo a função fputc responsavel, e resetamos o buffer e o contador
    if(bf->contador_bits == 8)
    {
        fputc(bf->buffer, bf->arquivo);
        bf->buffer = 0; // limpa o buffer, logo todos os bits sao zero para o byte 0
        bf->contador_bits = 0; // zera o contador
    }
}

// função para ler bit a bit, lendo um buffer que guarda o byte, de um byte, retornando exatamente o bit atual do termo mais significativo ate o menos
int read_bit(BitFile *bf)
{
    if(bf == NULL || bf->modo != 'r') return -1;

    // Se já lemos 8 bits do buffer atual, 1 byte, precisamos pegar o proximo byte do arquivo
    if(bf->contador_bits == 8)
    {
        int caractere = fgetc(bf->arquivo);
        if(caractere == EOF)
        {
            return -1; // em caso de EOF retornar -1
        }
        bf->buffer = (unsigned char) caractere;
        bf->contador_bits = 0; // reinicia o contador para ler o proximo byte
    }

    // na primeira vez que entro nessa função o contador de bits é setado para 8 logo eu leio diretamente o primeiro byte que a função fgetc pega do arquivo
    // e passo esse caractere que é um int para a formatação de byte atraves de unsigned char

    // Extrai o bit da posição atual usando máscara de bits e deslocamente, mascara de bits é basicamente revelarmos 1 bit por vez do nosso byte garantindo resultado estritamente 0 ou 1
    // e o responsavel pela mascara de bit é usar & 1, onde atraves dele no deslocamos sendo essa operação (7 - bf->contador_bits) selecionando o bit de interesse para ser o 
    // bit menos significativo que fara a operação com o & 1 retornando 1 ou 0 caso seja igual ou diferente
    int bit = (bf->buffer >> (7 - bf->contador_bits)) & 1; // essa operação ocorre desse modo pois pegamos bit a bit do mais significativo ate o menos significativo
    bf->contador_bits++;

    return bit;
}

// se houver bits restantes apos o modo leitura insere eles no arquivo e dps fecha o arquivo e da free na estrutura do bf, sem perder o arquiv pois ele é um ponteiro
void close_bit_file(BitFile *bf)
{
    if(bf == NULL) return;

    // FLUSH: se o arquivo estava no modo escrita e sobrou algum bit "perdido" no buffer ele comepleta o byte com zeros e coloca no arquivo
    if(bf->modo == 'w' && bf->contador_bits > 0)
    {
        // automaticamente por definimos o buffer como zero ele ja possuio por inicialização zero em todas as suas posições nao usadas
        // logo basta gravar esses bits restantes para forma o byte final do arquivo, evitando perder esse bits finais
        fputc(bf->buffer, bf->arquivo);
    }

    fclose(bf->arquivo);
    free(bf);
}