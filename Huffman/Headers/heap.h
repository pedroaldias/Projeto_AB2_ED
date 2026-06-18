#ifndef HEAP_H
#define HEAP_H

// proriedades da heap min que usamos é que o menor elemento esta no topo e obrigatoriamente os elementos posteriores são maiores ou iguais,
// no caso de uma heap para o huffman isso ocorre caso as frequencias sejam iguais em caracteres diferentes, que o pai e no pode ser pai de 0,1 ou 2 filhos
/**
 * @brief Definição opaca da estrutura da Heap.
 * * Garante o encapsulamento dos dados, impedindo o acesso direto
 * aos membros internos fora do arquivo heap.c.
 */
typedef struct heap Heap; // se eu quiser acessar fora do heap.c eu nao consigo ter controle das propriedades

/**
 * @brief Definição do ponteiro de função para comparação de elementos.
 * * @param a Ponteiro genérico (void*) para o primeiro elemento.
 * @param b Ponteiro genérico (void*) para o segundo elemento.
 * @return int Retorna um valor negativo (< 0) se 'a' tiver menor frequencia que 'b'.
 * Retorna um valor positivo (> 0) se 'a' tiver maior frequencia que 'b'.
 * Retorna zero (0) se ambos tiverem a mesma prioridade.
 */
typedef int (*comp_fn)(void *a, void *b); // exatamente como strcmp que retorna 0 se forem iguais -1 se b for maior e 1 se a for maior

/**
 * @brief Cria uma nova estrutura de Heap na memória de forma dinâmica.
 * * @param capacidade A quantidade máxima de elementos que a heap pode armazenar.
 * @param comparar O ponteiro para a função que ditará o critério de ordenação (prioridade).
 * @return Heap* Ponteiro para a Heap criada, ou NULL em caso de falha de alocação.
 */
Heap* criar_heap(int capacidade, comp_fn comparar); 
// retorna um ponteiro para heap criada e ele ja defini o tamanho maxima da heap e aloca ela dinamicamente e recebe um ponteiro para função que dita a ordenação ou prioridade da heap

/**
 * @brief Insere um novo elemento genérico na Heap.
 * * Após colocar o elemento na última posição livre, a função realiza
 * internamente o procedimento de subida (sift_up) para reorganizar a estrutura.
 * * @param h Ponteiro para a Heap onde o dado será inserido.
 * @param dado Ponteiro genérico (void*) representando o dado a ser armazenado.
 */
void inserir_heap(Heap *h, void* dado); 
// insere um dado dentro da nossa heap a partir do ultimo elemento e realiza o sift_up ate encontrar um local adequado para ser pai ou estar na heap mantendo a propriedade da heap

/**
 * @brief Remove e retorna o elemento de maior prioridade (o topo/mínimo) da Heap.
 * * Substitui a raiz pelo último elemento e realiza internamente o procedimento
 * de descida (sift_down) para restaurar a integridade da Heap.
 * * @param h Ponteiro para a Heap de onde o elemento será extraído.
 * @return void* Ponteiro genérico para o elemento removido, ou NULL se a heap estiver vazia.
 */
void *extrair_min_heap(Heap *h); 
// extrai o elemento de menor frequencia da nossa heap e apos isso pega o ultimo elemento da heap,
// torna ele a raiz e realiza o sift_down ate que apos uma sucessaõ de trocar a heap esteja de volta as suas propriedades

/**
 * @brief Consulta a quantidade atual de elementos armazenados na Heap.
 * * @param h Ponteiro para a Heap consultada.
 * @return int O número de elementos presentes na Heap.
 */
int tamanho_heap(Heap *h); // retorna a quantidade atual de elementos na nossa heap

/**
 * @brief Libera toda a memória alocada dinamicamente para a estrutura da Heap.
 * * Atenção: Esta função limpa o array de ponteiros da estrutura interna da Heap, 
 * mas NÃO desaloca a memória dos dados individuais que foram inseridos nela.
 * * @param h Ponteiro para a Heap que será destruída.
 */
void destruir_heap(Heap *h); 
// desaloca toda a memoria alocada dinamicamente na ESTRUTURA da heap sem apagar os dados pois isso apagaria oq esta na nossa arvore huffman, limpando apenas o array de ponteiros

#endif