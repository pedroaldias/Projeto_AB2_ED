    #include <stdio.h>
    #include <stdlib.h>
    #include "../Headers/header.h"
    #include "../Headers/huffman_tree.h"
    #include "../Headers/bitio.h"

    /*
    * Formato do arquivo .huff
    *
    * Byte 0 e 1 (16 bits):
    *   bit 15 ao 13 = quantidade de bits de lixo no último byte (0-7)
    *   bit 12 ao 0  = tamanho da árvore serializada em bytes (0-8191)
    *
    * Byte 2 até (2 + tamanho_arvore - 1):
    *   bytes da árvore em pré-ordem conforme serializar_arvore()
    *
    * Bytes restantes:
    *   dados comprimidos, o último byte pode ter bits de lixo no final
    */

    /* tamanho máximo do buffer de serialização da árvore:
    * pior caso: 256 folhas, cada folha = 2 bytes ('\' + símbolo)
    * cada nó interno = 1 byte ('*'), máximo 255 nós internos, 255 pois é o caminho maximo que um no interno pode ter ate uma folha
    * total máximo: 256*2 + 255 = 767 bytes — usamos 1024 com folga */
    #define TAM_MAX_ARVORE 1024

    struct header
    {
        int lixo;                                    // quantidade de bits lixo no ultimo byte
        int tamanho_arvore;                          // tamanho de bytes totais, sendo para o caractere de simbolo com \ e A por exemplo contando como dois caracteres, da arvore serializada(linear)
        unsigned char arvore_buffer[TAM_MAX_ARVORE]; // bytes da arvore serializados guardado em forma de pre-ordem, Ex: *\a\b, guardando as representaçoes dos simbolos dos bytes e sua sequencia
    };

    // cria um header e serializa a arvore linearmente obtendo o tamanho da arvore em relaçao a quantidade total de bytes representados nela, essa função precisa que passe o numero de lixo na arvore
    Header *criar_header(NodeHuffman *raiz, int lixo)
    {
        if(raiz == NULL) return NULL;

        Header *h = (Header *)malloc(sizeof(Header));
        if(h == NULL) return NULL;

        h->lixo = lixo;

        // serializa a arvore no buffer interno mapeando-a linearmente
        int pos = 0;
        serializar_arvore(raiz, h->arvore_buffer, &pos);
        h->tamanho_arvore = pos;

        return h;
    }

    // função a ser a primeira chamda na compactação, onde escrevemos nosso header no arquivo compactado onde inicialmente separado os bits de lixo e os bits de arvore e
    // juntamos em uma variavel e inserimos inicialmente essas informações, em ordem, no header compactado logo dps gravamos todos os bytes da arvore de maneira serializada
    void escrever_header(Header *h, BitFile *bf)
    {
        if(h == NULL || bf == NULL) return;

        // usamos operações bit wise or e and com mascara de bit para capturar exatamente o numero binario de 16 bits que desejamos
        // faz um bit shift no inteiro de lixo 13 casas para a esquerda e retornar para word o resultado da comparação bit a bit com 0b111,
        // isolando o resultado desejado nas 3 primeiras potencias mais significativa dos dois bytes de word
        // apos isso realzia um bit wise or com o resultado da operação AND bit a bit do tamanho da arvore com 0b1111111111111
        // desse modo capturamos todos os 1's e 0's e juntamos no unsigned short(2 bytes) de word juntando os dois pedaços pelo bitwise or
        // o bit shift em h->lixo nao é nos bits de h->lixo é pra posicionar os bits dele nos 3 primeiros de word
        unsigned short word = ((unsigned short)(h->lixo & 0b111) << 13) | ((unsigned short)(h->tamanho_arvore & 0b1111111111111)); // unsigned chart tem um espaço para 2 bytes(16 bits)

        // escrevemos os 16 bits diretamente no nosso aquivo de bytes compactado do mais significativo para o menos
        for(int i = 15; i >= 0; i--)
        {
            write_bit(bf, (word >> i) & 1); // escrevemo bit a bit do mais significativo pro menos mandando sem 0 ou 1 pela mascara de bit com 1
        }

        // escrevemos os bytes da nossa arvore serializada bit a bit atraves do write_bit no nosso arquivo de bytes compactado
        for(int i = 0; i < h->tamanho_arvore; i++)
        {
            for(int bit = 7; bit >= 0; bit--)
            {
                write_bit(bf, (h->arvore_buffer[i] >> bit) & 1); // escrevemos bit a bit cada byte, de todos os simbolos, que representa nossa arvore serializada
            }
        }
    }

    // função que sera a primeira aser chamada na descompactação onde eu consigo a partir dela lendo os 2 bytes iniciais pega o numero de bits lixo e o tamanho da arvore e logo apos
    // eu leio todos os bytes que representam a arvore serializada em pre-ordem e reconstruo minha arvore a partir de um buffer que recebeu essa serialização
    NodeHuffman *ler_header(BitFile *bf, int *lixo_out, int *tam_arvore_out)
    {
        if(bf == NULL) return NULL; // medida de segurança contra arquivos corrompidos

        // lê os 16 bits iniciais do nosso header que contem o numero de lixos e o tamanho da arvore
        unsigned short word = 0;
        for(int i = 15; i >= 0; i--)
        {
            int b = read_bit(bf);
            if(b == -1) return NULL; // medida de segurança contra arquivos corrompidos
            word |= ((unsigned short)b << i); // colocamos os bits lidos do arquivo de byte do mais significativo pro menos
        }

        *lixo_out = (word >> 13) & 0b111; // posiciona os 3 bits mais significativo, com o numero de bits lixo, no 3 menos significativos dos 16 para comparar bit wise and com 111
        int tamanho_arvore = (word) & 0b1111111111111; // comparar bit wise and com os valor dos 13 bits menos significativo, isolando o valor do tamanho da arvore
        if(tamanho_arvore == 0) return NULL;
        *tam_arvore_out = tamanho_arvore; // obtenho e armazeno o tamanho da arvore


        // Lendo os bytes da arvore serializados, seus simbolos em bytes unicos
        unsigned char *buf = (unsigned char *)malloc(tamanho_arvore*sizeof(unsigned char)); // aloca espaço suficiente para ler todos os simbolos da arvore em bytes
        if(buf == NULL) return NULL;

        for(int i = 0; i < tamanho_arvore; i++)
        {
            unsigned char byte = 0;
            for(int bit = 7; bit >= 0; bit--)
            {
                int b = read_bit(bf);
                if(b == -1)
                {
                    free(buf);
                    return NULL; // medida de segurança contra arquivos corrompidos
                } 
                byte |= ((unsigned char)b << bit); // colocamos os bits lidos do mais significativo para o menos e depois armazenamos o byte do simbolo no buf[i]
            }
            buf[i] = byte;
        }

        // reconstruindo a arvore a partir do buffer que representa ela serializada linearmente em pre-ordem
        int pos = 0;
        NodeHuffman *raiz = desserializar_arvore(buf, &pos, tamanho_arvore);
        free(buf); // agora que ja reconstrui a arvore nao preciso mais dela

        return raiz;
    }

    // função para apagar apenas a estrutura do header sem apagar os dados usados
    void destruir_header(Header *h)
    {
        if(h != NULL) free(h);
    }