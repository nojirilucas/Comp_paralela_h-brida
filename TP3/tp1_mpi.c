#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define MESTRE 0
#define STD_TAG 0

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

    MPI_File_set_view(file, process_rank * num_elementos_local * sizeof(int), MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    MPI_File_read_shared(file, nums_local, num_elementos_local, MPI_INT, MPI_STATUS_IGNORE);

    start_time = MPI_Wtime();

    for (int i = 0; i < num_elementos_local; i++) {
        if (EhPrimo(nums_local[i])) {
            divisores_local[i] = 2;
        } else {
            divisores_local[i] = Divisores(nums_local[i]);
        }
    }

    end_time = MPI_Wtime();

    MPI_File_close(&file);

    MPI_File out_file;
    MPI_File_open(MPI_COMM_WORLD, "saida.txt", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &out_file);
    MPI_File_set_view(out_file, process_rank * num_elementos_local * sizeof(int), MPI_INT, MPI_INT, "native", MPI_INFO_NULL);
    MPI_File_write_shared(out_file, divisores_local, num_elementos_local, MPI_INT, MPI_STATUS_IGNORE);
    MPI_File_close(&out_file);

    free(nums_local);
    free(divisores_local);

    if (process_rank == MESTRE) {
        printf("Tempo de execução: %f segundos\n", end_time - start_time);
    }

    MPI_Finalize();

    return 0;
}
