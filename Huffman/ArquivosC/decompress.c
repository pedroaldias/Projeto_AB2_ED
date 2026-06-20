#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Headers/decompress.h"
#include "../Headers/huffman_tree.h"
#include "../Headers/bitio.h"
#include "../Headers/header.h"

struct decompressor
{
    char *origem;
    char *destino;
};

// função para criar um onejto decompressor recebendo o arquivo comprimido na origem e o destino final vai ter o resultado da descompressao
Decompressor *criar_decompressor(const char *arquivo_origem, const char *arquivo_destino)
{
    if(arquivo_origem == NULL || arquivo_destino == NULL) return NULL;

    Decompressor *d = (Decompressor *)malloc(sizeof(Decompressor));
    if(d == NULL) return NULL;

    // Duplica as strings dos nomes dos arquivos evitando que ao mexer no ponteiro posteriormente cause esse nessa função pois é uma duplicata da string inteira e nao uma copia do endereço
    d->origem = strdup(arquivo_origem);
    d->destino = strdup(arquivo_destino); 

    return d;
}

// função que realiza todo o processo central e controle de descomprimir um arquivo compactado e jogar o resultado da descompressão em um arquivo destino atraves de ferramentas ja implementadas
// assim abirindo arquivo compactado fazendo a leitura de seu cabeçalho e recuperando as frequencais dos bytes unicos, extraindo ponteiro da header do arquivo compactado,
// reconstruindo a arvore huffman atraves das frequencias lidas, abrindo o arquivo de destino e tmb contando qual a quantidade de bytes a serem escritos nesse arquivo, decodificar o codigo
// de cada byte unico atraves da navegação na nossa arvore huffman e depositando o codigo ao achar uma folha, por fim limpar toda memoria alocada e fechar o arquivo origem
int executar_decompressao(Decompressor *d)
{
    if(d == NULL) return 0;

    // PASSO 1: Abrir o arquivo compactado para leitura dos bits passando como o modo de leitura na função open_bit_file
    BitFile *f_origem = open_bit_file(d->origem, "rb");
    if(f_origem == NULL) return 0;

    // PASSO 2: Ler o cabeçalho e recuperando a arvore huffman e obtendo o numero de bits lixo
    int lixo = 0;
    int tam_arvore = 0;
    // eu leio o header que esta no arquivo de byte e nessa função eu retorno um ponteiro do tipo arvore huffman,
    // criando uma arvore huffman a partir da leitura do arquivo de bytes e obtenho o numero de bits lixo
    NodeHuffman *raiz_huffman = ler_header(f_origem, &lixo, &tam_arvore);
    if(raiz_huffman == NULL) // arquivo corrompido fechar o arquivo e cancela o processo
    {
        close_bit_file(f_origem);
        return 0;
    }

    // apos ler o reader e recuperar a arvore, o BitFIle ja esta posicionado no primeiro bit dos dados
    // calcula o numero de bits uteis para saber o tamanho total do arquivo
    FILE *f_tam = fopen(d->origem, "rb");
    fseek(f_tam, 0, SEEK_END);
    long tam_total_bytes = ftell(f_tam);
    fclose(f_tam);

    // agora possuindo o tamanho da arvore precisamos saber quantos butes o header ocupa para calcular a quantidadede bytes_dados
    // header = 2 bytes fixos + tamanho_arvore bytes
    // assim descobrimos o numero de bytes para os dados do arquivo e o numero de bits uteis
    int tam_header_bytes = 2 + tam_arvore;
    long bytes_dados = tam_total_bytes - tam_header_bytes;
    long long bits_uteis = ((bytes_dados * 8) - lixo);
    if(bytes_dados <= 0 || bits_uteis <= 0)
    {
        // protege contra arquivo vazio ou corrompido
        destruir_arvore_huffman(raiz_huffman);
        close_bit_file(f_origem);
        return 0;
    }

    // PASSO 4: Abrir o arquivo de destino para gravar os bytes do arquivo de origem (wb, criando ou sobreescrevendo no arquivo destino os bytes do compactado)
    FILE *f_destino = fopen(d->destino, "wb");
    if(f_destino == NULL) // medida de proteção contra arquivos corrompidos
    {
        destruir_arvore_huffman(raiz_huffman);
        close_bit_file(f_origem);
        return 0;
    }

    // PASSO 5: decodificar os codigos dos bytes atraves de caminhar na arvore e nos sabemos quando devemos parar comparando o numero de bytes escritos com o total
    NodeHuffman *atual = raiz_huffman; // cria uma copia da raiz para caminhar sem alterar a raiz
    long long bits_lidos = 0;

    // Só se mantem no loop se ainda houver bytes para decodificar
    while(bits_lidos < bits_uteis)
    {
        int bit = read_bit(f_origem); // lê um byte do arquivo compactado no passo de bit a bit dele
        if(bit == -1)
        {
            // medida de segurança caso o arquivo acabe antes de terminarmos de ler todos os bytes previstos (arquivos corrompidos)
            break;
        }

        bits_lidos++; // contando de bit em bit

        // Se o bit for 0, vai para esquerda se for 1 vai para direita caminhando na arvore ate encontrar uma folha e nao um no interno
        if(atual->dir == NULL) atual = atual->esq; // força esquerda se a direita nao existe, solucionando o caso especial de um unico byte distinto
        else if(bit == 1)
        {
            atual = atual->dir;
        }
        else // bit == 0
        {
            atual = atual->esq;
        }

        // Se encontramos um nó folha quer dizer que achamos um byte original do texto antes da compactação
        if(atual->esq == NULL && atual->dir == NULL) // essa linha pode causar segmetaion fault dependendo de como é tratado arquivos com um unico byte distinto
        {
            fputc(atual->caractere, f_destino); // coloca o caractere do no folha no arquivo de destino apos descompatar
            atual = raiz_huffman; // voltamos para raiz percorrendo outro caminho
        }
    }

    // PASSO 7: limpeza geral e fechamento dos arquivos usados
    fclose(f_destino);
    destruir_arvore_huffman(raiz_huffman);
    close_bit_file(f_origem);
    
    // se conseguimos escrever todos os bytes previsto logo descompactamos com sucesso
    return (bits_lidos == bits_uteis);
}

// realiza a limpeza na memoria da estrutura do objeto descompactador sem apagar os dados usados
void destruir_decompressor(Decompressor *d)
{
    if(d != NULL)
    {
        free(d->origem);
        free(d->destino);
        free(d);
    }
}