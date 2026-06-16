#ifndef leitura_SMT
#define leitura_SMT
#include "leituraSAT.h"

typedef enum operador 
{
    MENOR_IGUAL,
    MAIOR_IGUAL,  
    IGUAL,        
    MENOR,        
    MAIOR,
    DIFERENTE
} operador;

// estrutura de uma (ine)equação linear ax + b op c 
typedef struct equacao
{
    int a;
    int b;
    operador op;
    int c;
} equacao;

typedef struct problemaSMT
{
    equacao *equacoes;
    formula *logicaSAT;
} problemaSMT;

problemaSMT* leArquivoSMT(const char *filename);

#endif