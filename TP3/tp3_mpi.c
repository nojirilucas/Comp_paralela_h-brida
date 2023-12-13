#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <pthread.h>

#define MESTRE 0
#define STD_TAG 0

typedef struct {
    int* nums_local;
    int* divisores_local;
    int start_index;
    int end_index;
} ThreadData;

int EhPrimo(int num) {
    if (num <= 1) {
        return 0;
    }
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) {
            return 0;
        }
    }
    return 1;
}

int Divisores(int num) {
    int count = 1;
    for (int i = 1; i <= sqrt(num); i++) {
        if (num % i == 0) {
            count += 2;
        }
    }
    if (sqrt(num) == (int)sqrt(num)) {
        count--;
    }
    return count;
}

void* processar_local(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    // Processar dados locais
    for (int i = 0; i < data->end_index - data->start_index; i++) {
        if (EhPrimo(data->nums_local[i])) {
            data->divisores_local[i] = 2;
        } else {
            data->divisores_local[i] = Divisores(data->nums_local[i]);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int process_rank, size_of_cluster, num_elementos_global, num_elementos_local;
    int *nums_local, *divisores_local;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size_of_cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);

    MPI_File file;
    MPI_File_open(MPI_COMM_WORLD, "entrada.txt", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);

    MPI_Offset filesize;
    MPI_File_get_size(file, &filesize);
    num_elementos_global = filesize / sizeof(int);

    num_elementos_local = num_elementos_global / size_of_cluster;
    nums_local = (int *)malloc(num_elementos_local * sizeof(int));
    divisores_local = (int *)malloc(num_elementos_local * sizeof(int));

    MPI_File_set_view(file, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    MPI_File_read_at_all(file, process_rank * num_elementos_local, nums_local, num_elementos_local, MPI_INT, MPI_STATUS_IGNORE);

    start_time = MPI_Wtime();

    // Criação e execução de threads para processamento local
    pthread_t* threads = (pthread_t*)malloc(size_of_cluster * sizeof(pthread_t));
    ThreadData* thread_data = (ThreadData*)malloc(size_of_cluster * sizeof(ThreadData));

    for (int i = 0; i < size_of_cluster; i++) {
        thread_data[i].start_index = i * num_elementos_local;
        thread_data[i].end_index = (i + 1) * num_elementos_local;
        thread_data[i].nums_local = (int*)malloc(num_elementos_local * sizeof(int));
        thread_data[i].divisores_local = (int*)malloc(num_elementos_local * sizeof(int));

        // Copiar dados originais para dados locais
        for (int j = 0; j < num_elementos_local; j++) {
            thread_data[i].nums_local[j] = nums_local[j + i * num_elementos_local];
        }

        pthread_create(&threads[i], NULL, processar_local, (void*)&thread_data[i]);
    }

    // Aguarda a conclusão das threads
    for (int i = 0; i < size_of_cluster; i++) {
        pthread_join(threads[i], NULL);
    }

    end_time = MPI_Wtime();

    MPI_File_close(&file);

    MPI_File out_file;
    MPI_File_open(MPI_COMM_WORLD, "saida.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &out_file);
    MPI_File_set_view(out_file, 0, MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    MPI_File_write_at_all(out_file, process_rank * num_elementos_local, divisores_local, num_elementos_local, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&out_file);

    // Liberar dados locais
    for (int i = 0; i < size_of_cluster; i++) {
        free(thread_data[i].nums_local);
        free(thread_data[i].divisores_local);
    }
    free(threads);
    free(thread_data);

    if (process_rank == MESTRE) {
        printf("Tempo de execução: %f segundos\n", end_time - start_time);
    }

    MPI_Finalize();

    return 0;
}
