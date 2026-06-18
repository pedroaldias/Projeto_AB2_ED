#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Headers/compress.h"
#include "../Headers/huffman_tree.h"
#include "../Headers/bitio.h"
#include "../Headers/header.h"

struct compressor
{
    char *origem;  // nome do arquivo de entrada
    char *destino; // nome do arquivo de saida
};

// função para criar o compressor e copiar salvando o nome do arquivo de origem e o destino
Compressor *criar_compressor(const char *arquivo_origem, const char *arquivo_destino)
{
    if(arquivo_origem == NULL || arquivo_destino == NULL) return NULL;

    Compressor *c = (Compressor *)malloc(sizeof(Compressor));
    if(c == NULL) return NULL; // medida de segurança

    // duplica as strings dos nomes dos arquivos para garantir o encapsulamento
    // copiando a strings alterações feitas com a string original nao alteram em nada o compressor
    c->origem = strdup(arquivo_origem);
    c->destino = strdup(arquivo_destino);

    return c;
}

// realiza todo o processo central de comprimir o arquivo de origem e depositar no arquivo destino incluindo contar as frequencais dos bits, criar a arvore huffman e o dicionario dos codigos
// cria a grava no arquivo de bytes o header, volta para o começo do arquivo original e grava os bytes em bit a bit no arquivo de byte
// fecha o arquivo original e liberta todas as estruturas usadas da memoria
int executar_compressao(Compressor *c)
{
    if(c == NULL) return 0;

    // PASSO 1: Contar as frequencias dos bytes no arquivo original
    FILE *f_origem = fopen(c->origem, "rb");
    if(f_origem == NULL) return 0;

    int frequencias[256] = {0};
    int caractere;
    while((caractere = fgetc(f_origem)) != EOF) // passa por todos os bytes do arquivo e soma +1 na usa frequencia
    {
        frequencias[caractere]++;
    }

    // PASSO 2: Criar a árvore de huffman e o dicionário dos códigos
    // aqui usamos as funções implementadas no huffman_tree.c
    NodeHuffman *arvore_huffman = construir_a_arvore(frequencias);
    if(arvore_huffman == NULL)
    {
        fclose(f_origem);
        return 0;
    }

    // Cria uma tabela de string para armazenar o código de cada um dos 256 bytes
    char caminho[257];
    char *dicionario[256] = {NULL}; //array de tamanho 256 onde dentro possui ponteiros do tipo char
    gerar_codigo_huffman(arvore_huffman, dicionario, caminho, 0); 
    // passo o caminho como parametro pois ele tem espaço para o maior possivel de 256 bits + \0 e ele é reescrito durante o caminho da arvore atraves da volta das chamadas recursivas

    // PASSO 3: Calculo do total de bits, considerando as frequencias, e a quantidade de bits lixo no ulitmo byte
    long long total_bits = 0;
    for(int i = 0; i < 256; i++)
    {
        if(dicionario[i] != NULL)
        {
            // eu vou adicionar ao total de bits a frequencai que ele aparece vezes o numero de bits do seu codigo huffman
            total_bits += (long long)frequencias[i] * strlen(dicionario[i]);
        }
    }

    // aqui eu pego o total de bits e faço o resto por 8, obtendo a quantidade de bits finais uteis no ultimo bit graça ao codigo huffman n ter completado o ultimo byte
    // para obter o quanto de bits lixo o ultimo byte preciso eu diminuo por 8 e caso o ultimo byte esta completo essa conta daria 8, o que é um problema
    // pois se eu nao preciso completar nada pelo total de bits preencher todos os bytes deveria ser 0 o numero de lixo e não 8, por isso o resto 8 dnv
    int lixo = (8 - (int)(total_bits % 8)) % 8; 

    // PASSO 4: abrir o arquivo destino em modo "wb", criar e gravar o header no arquivo de destino
    BitFile *f_destino = open_bit_file(c->destino, "wb"); // abrimos nosso arquivo de bytes como file e dps fechamos para abrir como BitFile
    if(f_destino == NULL)
    {
        // limpeza de emergencia pois n foi possivel criar o arquivo de bytes
        liberar_dicionario(dicionario);
        destruir_arvore_huffman(arvore_huffman);
        fclose(f_origem);
        return 0;
    }

    Header *h = criar_header(arvore_huffman, lixo); // cria o nosso header
    escrever_header(h, f_destino); // escreve no nosso arquivo de bytes o header
    destruir_header(h); // agora que ja escrevemos o header no nosso arquivo de bytes podemos apagar ele da memoria

    // PASSO 4: Voltar o ponteiro do arquivo original para o seu início
    rewind(f_origem);

    // PASSO 5: Ler o arquivo original byte a byte e gravar no arquivo de bytes os bits correspondentes
    while((caractere = fgetc(f_origem)) != EOF)
    {
        char *codigo = dicionario[caractere];

        // Percorre a string de 0s e 1s do codigo bit a bit e grava no arquivo de bytes
        for(int i = 0; codigo[i] != '\0'; i++)
        {
            if(codigo[i] == '1')
            {
                write_bit(f_destino, 1);
            }
            else
            {
                write_bit(f_destino, 0);
            }
        }
    }

    // PASSO 6: fechar os arquivos e liberar toda memória utilizada
    close_bit_file(f_destino); // O close_bit_file faz o flush automatico nos ultimos bits caso sobre algum bit no buffer para ser introduzido no arquivo de byte
    fclose(f_origem);
    liberar_dicionario(dicionario);
    destruir_arvore_huffman(arvore_huffman);

    return 1; // compressao realizada com sucesso
}

// função para destruir da memoria a estrutura do compressor podendo destruir as propriedades pq eu nao tenho uma copia do endereço eu duplico a string no compressor
void destruir_compressor(Compressor *c)
{
    if(c != NULL)
    {
        free(c->origem);
        free(c->destino);
        free(c);
    }
}