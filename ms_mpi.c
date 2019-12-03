#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#define SIZE 1024

void merge(int *vetor, int *aux, int comeco, int meio, int fim) {
	
	int k = comeco;
	int j = meio + 1;
	int i_vet; // percorre o vetor original
	int i_aux = comeco; // percorre o vetor auxiliar
	
	// bota os numeros em ordem no vetor auxiliar
	while((k<=meio) && (j<=fim)) {	
		if(vetor[k] <= vetor[j]) {
			aux[i_aux] = vetor[k];
			k++;
		}
		else {
			aux[i_aux] = vetor[j];
			j++;
		}
		i_aux++;
	}

	// se um deles acabar antes do outro
	if(meio < k) {
		for(i_vet = j; i_vet <= fim; i_vet++) {
			aux[i_aux] = vetor[i_vet];
			i_aux++;
		}	
	}	
	else {	
		for(i_vet = k; i_vet <= meio; i_vet++) {	
			aux[i_aux] = vetor[i_vet];
			i_aux++;	
		}		
	}
		
	for(i_vet = comeco; i_vet <= fim; i_vet++) {
			vetor[i_vet] = aux[i_vet];	
		}	
	}


void mergeSort(int *vetor, int *aux, int comeco, int fim) {
	int meio;
	if(comeco < fim) {	
		meio = (comeco + fim)/2;
		// chamada recursiva para a parte da esquerda do vetor
		mergeSort(vetor, aux, comeco, meio);
		// chamada recursiva para a parte da direita do vetor
		mergeSort(vetor, aux, (meio + 1), fim);
		// mergeando as duas partes na ordem certa
		merge(vetor, aux, comeco, meio, fim);
	}	
}

int main(int argc, char** argv) {
	
	int world_rank;
	int world_size;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	
	int *vetor = (int *)malloc(sizeof(int) * SIZE);
	int *vetor_ordenado = (int *)malloc(sizeof(int) * SIZE);	
	
	srand(time(NULL));
	printf("\nTAMANHO: %d\n", SIZE);
    printf("\nVETOR ORIGINAL:\n");
    for(int i=0;i<SIZE;i++){
        // numeros aleatorios entre 0 e 100
        vetor[i] = rand() % 100;
        printf("%d ",vetor[i]);
    }

    // quantos numeros do vetor ficam atribuidos a cada processo
	int numeros_por_processo = SIZE/world_size;
	printf("\nNumeros por processo: %d\n", numeros_por_processo);
	
	// buffer que vai receber a particao de numeros do processo
	int *particao_vetor = malloc(numeros_por_processo * sizeof(int));
	// o scatter envia diferentes dados para todos os processos
	MPI_Scatter(vetor, numeros_por_processo, MPI_INT, particao_vetor, numeros_por_processo, MPI_INT, 0, MPI_COMM_WORLD);
	
	// necessario para enviar
	int *aux = malloc(numeros_por_processo * sizeof(int));
	mergeSort(particao_vetor, aux, 0, (numeros_por_processo - 1));
		
	// so o mestre precisa do buffer de recebimento
	if(world_rank == 0){
		// contrario do scatter, reune dado de todos os processos no processo mestre
		MPI_Gather(particao_vetor, numeros_por_processo, MPI_INT, vetor_ordenado, numeros_por_processo, MPI_INT, 0, MPI_COMM_WORLD);
	}
	else{
		MPI_Gather(particao_vetor, numeros_por_processo, MPI_INT, NULL, numeros_por_processo, MPI_INT, 0, MPI_COMM_WORLD);
	}
	
	if(world_rank == 0) {	
		int *aux2 = malloc(SIZE * sizeof(int));
		mergeSort(vetor_ordenado, aux2, 0, (SIZE-1));
		
		printf("\nVETOR ORDENADO\n");
		for(int i = 0; i < SIZE; i++) {
			printf("%d ", vetor_ordenado[i]);
		}			
		free(aux2);	
	}
	
	free(vetor);
	free(particao_vetor);
	free(vetor_ordenado);
	free(aux);	
	MPI_Finalize();
}


