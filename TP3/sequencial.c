#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h> 
#include <sys/stat.h>

typedef struct Divisores{
    int divisivel;
    int expoente;
    struct Divisores *prox;
} Divisores;

//Cria um nó sempre que aparece um novo primo divisor
Divisores* CriaNos(int valor) {
    Divisores *i = calloc(1, sizeof(Divisores));
    if (i == NULL) {
        fprintf(stderr, "Falha na alocação de memória.\n");
        exit(EXIT_FAILURE);
    }
    i->divisivel = valor;
    i->expoente = 1;
    i->prox = NULL;
    return i;
}


//Liberar memória alocada.
void Liberar_Mem(Divisores *inicio) {
    Divisores *i = inicio;
    Divisores *prox = NULL;
    for (; i != NULL; i = prox) {
        prox = i->prox;
        free(i);
    }
}

//Faz a fatoração do número e calcula o produto dos expoentes dos primos adicionados em 1.
int Fatora_Prod(int num) {
    int count = 1, divisor = 2, expoente = 0;

    while (num > 1) {
        if (num % divisor == 0) {
            num = num / divisor;
            expoente++;
        } else {
            count *= (expoente + 1);
            expoente = 0;
            divisor++;
        }
    }

    count *= (expoente + 1);
    return count;
}

//Calcula os divisores.
int Qtd_Div(int num){
    int count = 1;
    for (int i = 1; i <= (num/2); i++) {
        if ((num % i) == 0) {
            count++;
        }
    }
    return count;
}

int main(int argc, char *argv[]){
    double time_spent = 0.0;
    char *arq_entrada = "entrada.txt";
    
    //Abertura do arquivo de entrada.
    FILE *entrada;
    if (!(entrada = fopen(arq_entrada, "r"))) {
        fprintf(stderr, "Não foi possível abrir o arquivo de entrada.\n");
        return 2;
    }

    //Abertura do arquivo de saída.
    FILE *saida;
    if (!(saida = fopen("saida_seq.txt", "w"))) {
        fprintf(stderr, "Não foi possível criar/abrir o arquivo de entrada.\n");
        return 3;
    }

    //Contagem de linhas do arquivo.
    int linhas = 0, num;
    while(fscanf(entrada, "%i", &num) != EOF) {
        linhas++;      
    }

    int nums[linhas];
    int div[linhas];
    rewind(entrada);
    //Leitura do arquivo de entrada.
    for (int i = 0; i < linhas; i++) {
        fscanf(entrada, "%d", &nums[i]);
    }

    clock_t begin = clock();
    //Calcula o número de divisores.
    for (int i = 0; i < linhas; i++){
        div[i] = Fatora_Prod(nums[i]);
        printf("%d - %d\n", nums[i], div[i]);
    }
    clock_t end = clock();

    //Salvando resultado em um arquivo de saída.
    for (int i=0; i<linhas; i++){    
        fprintf(saida, "%i\n", div[i]);
    }
        
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Tempo de execução -> Fatora_Prod: %f segundos", time_spent);

    fclose(entrada);
    fclose(saida);

    return 0;
}