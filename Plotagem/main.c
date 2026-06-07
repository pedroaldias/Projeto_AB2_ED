#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * ! Execução: gcc -std=c99 main.c -o main
 */

#define TAMANHO_MAXIMO 5005

typedef struct heap {
	int tamanho_heap;
	int dados[TAMANHO_MAXIMO];
} heap;

/**
 * Obtém o índice do pai de um elemento.
 * @param i Índice do elemento.
 * @return Índice do elemento pai.
 */
int obter_indice_pai(int i) {
	return i/2;
}

/**
 * Obtém o índice do filho à esquerda de um elemento.
 * @param i Índice do elemento.
 * @return Índice do filho à esquerda.
 */
int obter_indice_filho_esquerda(int i) {
	return 2*i;
}

/**
 * Obtém o índice do filho à direita de um elemento.
 * @param i Índice do elemento.
 * @return Índice do filho à direita.
 */
int obter_indice_filho_direita(int i) {
	return 2*i + 1;
}

/**
 * Função que troca os valores entre um elemento e seu pai.
 * @param end_elemento endereço do 'elemento'.
 * @param end_pai endereço do pai do 'elemento'.
 * @return void
 */
void trocar_valores(int *end_elemento, int *end_pai) {
	int aux = *end_elemento;
	*end_elemento = *end_pai;
	*end_pai = aux;
}

/**
 * Adiciona um novo elemento na heap e reorganiza a heap para manter as regras da heap:
 * * O pai sempre é maior que os filhos.
 * @param heap ponteiro para uma heap (Heap).
 * @param elemento novo elemento a ser adicionado.
 * @param comparacoes endereço para um int que armazena a quantidade de comparações.
 * @return void
 */
void enqueue(heap *heap, int elemento, int *comparacoes) {
	if (heap->tamanho_heap >= TAMANHO_MAXIMO) {
		printf("Heap Overflow");
	} else {
		// Adiciona o elemento na última posição da heap;
		heap->dados[++heap->tamanho_heap] = elemento;

		int indice_elemento = heap->tamanho_heap;
		int indice_pai = obter_indice_pai(indice_elemento);

        // verifica se o índice não é menor que o índice inicial e se o elemento é maior que seu pai
		while(indice_pai >= 1 &&  heap->dados[indice_elemento] > heap->dados[indice_pai]) {
			(*comparacoes)++;

			trocar_valores(&heap->dados[indice_elemento], &heap->dados[indice_pai]); // swap

            indice_elemento = indice_pai;
            indice_pai = obter_indice_pai(indice_elemento);
		}
	}
}

/**
 * Organiza a heap para manter as regras da heap:
 * * O pai sempre é maior que os filhos.
 * @param heap ponteiro para uma heap (Heap).
 * @param indice índice do elemento a ser realocado na heap.
 * @param comparacoes endereço para um int que armazena a quantidade de comparações.
 * @return void
 */
void max_heapify(heap *heap, int indice, int *comparacoes){
    int indice_esq = obter_indice_filho_esquerda(indice);
    int indice_dir = obter_indice_filho_direita(indice);
	int maior = indice_esq;

    if(indice_esq > heap->tamanho_heap){
        return;
    }

    if(indice_dir > heap->tamanho_heap || heap->dados[indice_esq] > heap->dados[indice_dir]) {
		(*comparacoes)++;

		maior = indice_esq;
    }else{
		maior = indice_dir;
	}

	// se o 'maior' é maior que o 'pai'
	if(heap->dados[maior] > heap->dados[indice]) {
		(*comparacoes)++;

		trocar_valores(&heap->dados[maior], &heap->dados[indice]);

		max_heapify(heap, maior, comparacoes);
	}
}

/**
 * Retira o primeiro elemento (maior prioridade) da heap.
 * @param heap ponteiro para uma heap (Heap).
 * @param comparacoes endereço para um int que armazena a quantidade de comparações.
 * @return valor do elemento.
 */
int dequeue(heap *heap, int *comparacoes){
    int elemento = heap->dados[1];

    heap->dados[1] = heap->dados[heap->tamanho_heap];
    heap->tamanho_heap--;

    max_heapify(heap, 1, comparacoes);

    return elemento;
}

// FILA SIMPLES ------------------------------------------------------------------------------------------
typedef struct fila {
    int tamanho;
    int dados[TAMANHO_MAXIMO];
} fila;

/**
 * Insere um elemento no final do array desordenado.
 * Custo: O(1)
 */
void enqueue_fila(fila *fila, int elemento) {
    if (fila->tamanho >= TAMANHO_MAXIMO) {
        printf("Fila Overflow");
    } else {
        fila->dados[++fila->tamanho] = elemento;
    }
}

/**
 * Remove e retorna o maior elemento usando busca linear.
 * @param fila ponteiro pra uma fila.
 * @param comparacoes endereço para um int que armazena a quantidade de comparações.
 */
int dequeue_fila(fila *fila, int *comparacoes) {
    if (fila->tamanho == 0) {
        printf("Fila Underflow");
        return -1;
    }

    int indice_maior = 1;
    int maior_valor = fila->dados[1];

    for (int i = 2; i <= fila->tamanho; i++) {
        (*comparacoes)++;
        
        if (fila->dados[i] > maior_valor) {
            maior_valor = fila->dados[i];
            indice_maior = i;
        }
    }

    //pega o último elemento da fila e o coloca na posição do elemento que acabou de ser removido.
    fila->dados[indice_maior] = fila->dados[fila->tamanho];
    fila->tamanho--;

    return maior_valor;
}


int main() {
    // Gerador de números aleatórios
    srand(time(NULL));
    heap my_heap;
    my_heap.tamanho_heap = 0;
    fila my_fila;
    my_fila.tamanho = 0;

    // Cria o arquivo para exportar para o Matlab
    FILE *arquivo_csv = fopen("dados_comparacao.csv", "w");
    if (arquivo_csv == NULL) {
        printf("Erro ao criar arquivo CSV.\n");
        return 1;
    }
    
    // Cabeçalho do CSV
    fprintf(arquivo_csv, "Elemento,Comparacoes_Heap,Comparacoes_Fila\n");

    int sorteios = 5000;

    for (int i = 0; i < sorteios; i++) {
        int numero_sorteado = rand() % 5001; // Número aleatório entre 0 e 5000
        
        int comparacoes_heap = 0;
        int comparacoes_fila = 0;

        // INSERÇÃO
        enqueue(&my_heap, numero_sorteado, &comparacoes_heap);
        enqueue_fila(&my_fila, numero_sorteado);

        if (i > 0 && i % 100 == 0) {
            dequeue(&my_heap, &comparacoes_heap);
            dequeue_fila(&my_fila, &comparacoes_fila);

            // Escreve a linha no CSV
            fprintf(arquivo_csv, "%d,%d,%d\n", i, comparacoes_heap, comparacoes_fila);
        }
    }

    fclose(arquivo_csv);

    return 0;
}