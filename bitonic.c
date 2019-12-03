#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define sobe 0
#define desce 1
#define MIN(x, y) (((x) > (y)) ? (y) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

void bitonicSortPar(int comeco, int tamanho, int *vet, int direcao);
void bitonicSortSeq(int comeco, int tamanho, int *vet, int direcao);

int tam_particao;

void troca(int *x, int *y){ //troca dois elementos do vetor
    int temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

void bitonicSortSeq(int comeco, int tamanho, int *vet, int direcao){
    int i;

    if (tamanho == 1) //se o vetor só tem um elemento, então já está ordenado
        return;

    if (tamanho % 2 !=0 ){ //algoritmo é feito para potências de 2
        printf("Tamanho não é multiplo de 2\n");
        exit(0);
    }

    int k = tamanho / 2;

    //divisao bitonica
    for (i = comeco; i < comeco + k; i++){
        if (direcao == sobe) { //forma parte crescente da sequencia bitonica
            if (vet[i] > vet[i + k])
                troca(&vet[i], &vet[i + k]);
        }
        else { //forma parte decrescente da sequencia bitonica
            if (vet[i] < vet[i + k])
                troca(&vet[i], &vet[i + k]);
        }
    }

    bitonicSortSeq(comeco, k, vet, direcao); //chama para a primeira metade do vetor
    bitonicSortSeq(comeco + k, k, vet, direcao); //chama para a segunda metade do vetor
}

void bitonicSortPar(int comeco, int tamanho, int *vet, int direcao){
    int i;
    if (tamanho == 1) //se o vetor só tem um elemento, então já está ordenado
        return;

    if (tamanho % 2 !=0 ){ //algoritmo é feito para potências de 2
        printf("Tamanho da particao não é múltiplo de 2\n");
        exit(0);
    }

    int k = tamanho / 2; //metade do numero de elementos do vetor

    //divisao bitonica
    #pragma omp parallel for shared(vet, direcao, comeco, k) private(i)
    for (i = comeco; i < comeco + k; i++){
        if (direcao == sobe){
            if (vet[i] > vet[i + k])
                troca(&vet[i], &vet[i + k]);
        }
        else{
            if (vet[i] < vet[i + k])
                troca(&vet[i], &vet[i + k]);
        }
    }

    if (k > tam_particao){
        // tam_particao é o tamanho da partição, que é dado por n/numThreads
        bitonicSortPar(comeco, k, vet, direcao);
        bitonicSortPar(comeco + k, k, vet, direcao);
    }

    return;
}

int main(){
    int n, i, j, valor_inicial, valor_final, direcao, *vet, numThreads, id;

    n = 1024; //tamanho do vetor
    valor_inicial = 1; //intervalo para gerar número aleatorios
    valor_final = 200;
    vet = (int *) malloc (n * sizeof(int));

    srand(time(NULL));
    for (i = 0; i < n; i++){ //gerando vetor com numeros aleatorios
        vet[i] = (rand() % (valor_inicial - valor_final)) + (valor_inicial+1);
    }

    numThreads =  omp_get_max_threads();

    printf("Rodando com %d threads\n", numThreads);

    // Conferindo se o tamanho do vetor é menor que o número de threads
    if ( n < numThreads * 2 ){
        printf("O tamanho do vetor é menor que número de threads * 2.\n");
        exit(0);
    }

    // define o tamanho da particao que cada thread vai ficar
    tam_particao = n / numThreads;

    // gera bitonic sequence, parte 1
    for (i = 2; i <= tam_particao; i = 2 * i){
        #pragma omp parallel for shared(vet, i) private(direcao, j)
        for (j = 0; j < n; j += i){
            if ((j / i) % 2 == 0)
                direcao = sobe; //a sequencia deve ser crescente
            else
                direcao = desce; //a sequencia deve ser decrescente
            bitonicSortSeq(j, i, vet, direcao);
        }
    }

    // gera bitonic sequence, parte 2
    for (i = 2; i <= numThreads; i = 2 * i){
        for (j = 0; j < numThreads; j += i){
            if ((j / i) % 2 == 0)
                direcao = sobe;
            else
                direcao = desce;
            bitonicSortPar(j*tam_particao, i*tam_particao, vet, direcao);
        }
        #pragma omp parallel for shared(j)
        for (j = 0; j < numThreads; j++){
            if (j < i)
                direcao = sobe;
            else
                direcao = desce;
            bitonicSortSeq(j*tam_particao, tam_particao, vet, direcao);
        }
    }

    printf("Vetor ordenado:\n");
    for(i = 0; i < n; i++){
        printf("%d ", vet[i]);
    }
    printf("\n");

    free(vet);
}

