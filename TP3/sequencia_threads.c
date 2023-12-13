#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <pthread.h>

typedef struct Divisores
{
    int divisivel;
    int expoente;
    struct Divisores *prox;
} Divisores;

typedef struct ThreadData
{
    int *nums;
    int *div;
    int start;
    int end;
} ThreadData;

Divisores *CriaNos(int valor)
{
    Divisores *i = calloc(1, sizeof(Divisores));
    if (i == NULL)
    {
        fprintf(stderr, "Falha na alocação de memória.\n");
        exit(EXIT_FAILURE);
    }
    i->divisivel = valor;
    i->expoente = 1;
    i->prox = NULL;
    return i;
}

void Liberar_Mem(Divisores *inicio)
{
    Divisores *i = inicio;
    Divisores *prox = NULL;
    for (; i != NULL; i = prox)
    {
        prox = i->prox;
        free(i);
    }
}

int Fatora_Prod(int num)
{
    int count = 1, divisor = 2, expoente = 0;

    while (num > 1)
    {
        if (num % divisor == 0)
        {
            num = num / divisor;
            expoente++;
        }
        else
        {
            count *= (expoente + 1);
            expoente = 0;
            divisor++;
        }
    }

    count *= (expoente + 1);
    return count;
}

int Qtd_Div(int num)
{
    int count = 1;
    for (int i = 1; i <= (num / 2); i++)
    {
        if ((num % i) == 0)
        {
            count++;
        }
    }
    return count;
}

void *ProcessChunk(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    for (int i = data->start; i <= data->end; i++)
    {
        data->div[i] = Fatora_Prod(data->nums[i]);
        printf("%d - %d\n", data->nums[i], data->div[i]);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    double time_spent = 0.0;
    char *arq_entrada = "entrada.txt";

    FILE *entrada;
    if (!(entrada = fopen(arq_entrada, "r")))
    {
        fprintf(stderr, "Não foi possível abrir o arquivo de entrada.\n");
        return 2;
    }

    FILE *saida;
    if (!(saida = fopen("saida_seq.txt", "w")))
    {
        fprintf(stderr, "Não foi possível criar/abrir o arquivo de entrada.\n");
        return 3;
    }

    int linhas = 0, num;
    while (fscanf(entrada, "%i", &num) != EOF)
    {
        linhas++;
    }

    int nums[linhas];
    int div[linhas];
    rewind(entrada);
    for (int i = 0; i < linhas; i++)
    {
        fscanf(entrada, "%d", &nums[i]);
    }

    pthread_t threads[4]; // Ajusta o numero de threads necessarias
    ThreadData threadData[4]; // Ajusta o numero de threads necessarias

    clock_t begin = clock();

    int chunkSize = linhas / 4; // Ajusta o numero de threads necessarias

    for (int i = 0; i < 4; i++) // Ajusta o numero de threads necessarias
    {
        threadData[i].nums = nums;
        threadData[i].div = div;
        threadData[i].start = i * chunkSize;
        threadData[i].end = (i == 3) ? (linhas - 1) : (threadData[i].start + chunkSize - 1);

        pthread_create(&threads[i], NULL, ProcessChunk, (void *)&threadData[i]);
    }

    for (int i = 0; i < 4; i++) // Ajusta o numero de threads necessarias
    {
        pthread_join(threads[i], NULL);
    }

    clock_t end = clock();

    for (int i = 0; i < linhas; i++)
    {
        fprintf(saida, "%i\n", div[i]);
    }

    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Tempo de execução -> Fatora_Prod: %f segundos", time_spent);

    fclose(entrada);
    fclose(saida);

    return 0;
}
