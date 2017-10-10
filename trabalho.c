#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <windows.h>
#include <time.h>

typedef struct
{
    float **data;
} Matriz;

Matriz matrizTeste;
Matriz matrizTreino;
int algoritmo = 0;
int numeroDeColunas = 0;
int numeroDeLinhasDeTreino = 0;
int numeroDeLinhasDeTeste = 0;
int numeroDeThreads = 0;
int indiceMatrizTeste = 0;
float *menoresResultados;

FILE *AbrirArquivo(FILE *arquivo, char *caminho)
{
    arquivo = fopen(caminho, "r");
    if (arquivo == NULL)
    {
        printf("Nao foi possivel abrir o arquivo.\n");
    }
    return arquivo;
}

int ExtrairValorInteiro(char *nomeDoArquivo)
{
    int len = strlen(nomeDoArquivo), i, j = 0;
    char numero[len];
    for (i = 0; i < len; i++)
    {
        if (isdigit(nomeDoArquivo[i]))
        {
            strncpy(&numero[j], &nomeDoArquivo[i], 1);
            j++;
        }
    }
    numero[j] = '\0';
    return atoi(numero);
}

void PreencherColunas(char *linhaDoArquivo, int indiceDaLinhaCorrente, Matriz *matriz)
{
    char *celula;
    int coluna = 0;
    celula = strtok(linhaDoArquivo, ",");
    while (celula != NULL && coluna != numeroDeColunas)
    {
        matriz->data[indiceDaLinhaCorrente][coluna] = atof(celula);
        celula = strtok(NULL, ",");
        coluna++;
    }
}

float DistanciaEuclidiana(float *treino, float *teste)
{
    float somatoria = 0;
    int i;
    for (i = 0; i < numeroDeColunas; i++)
    {
        somatoria += pow((treino[i] - teste[i]), 2);
    }
    return sqrt(somatoria);
}

float DistanciaManhattan(float *treino, float *teste)
{
    float somatoria = 0;
    int i;
    for (i = 0; i < numeroDeColunas; i++)
    {
        somatoria += abs((treino[i] - teste[i]));
    }
    return somatoria;
}

DWORD WINAPI FuncaoThread(void *data)
{
    int id = (int)data, i;
    float resultado = 0;
    for (i = id; i < numeroDeLinhasDeTreino; i += numeroDeThreads)
    {
        resultado = DistanciaManhattan(matrizTreino.data[i], matrizTeste.data[indiceMatrizTeste]);
        if (resultado < menoresResultados[id]) {
            menoresResultados[id] = resultado;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    clock_t begin = clock();

    int opcao;
    extern char *optarg;

    FILE *arquivoTeste;
    FILE *arquivoTreino;
    char *linha = NULL;
    size_t tamanho = 0;
    ssize_t leitura;

    matrizTeste.data = (float **)malloc(3000 * sizeof(float *));
    matrizTreino.data = (float **)malloc(10000 * sizeof(float *));

    while ((opcao = getopt(argc, argv, "ht:n:p:")) != -1)
    {
        switch (opcao)
        {
        case 'h':
            printf("----------------------------------------------------------------------------------------------------\n");
            printf("\t\t\tsimplificada do algoritmo K-NN\n");
            printf("----------------------------------------------------------------------------------------------------\n");
            printf("Utilizacao: ./trabalho -t teste.data -n treino.data -p 1,2,3... (threads) -a algoritmo (1,2)\n");
            printf("Opcoes:\n");
            printf(" -Opcao -h: ajuda\n");
            printf(" -Opcao -t: caminho do arquivo de teste Ex: ./trabalho -t arquivo de teste.data\n");
            printf(" -Opcao -n: caminho do arquivo de treino Ex: ./trabalho -n arquivo de treino.data\n");
            printf(" -Opcao -p: numero de threads Ex: ./trabalho -p 1\n");
            printf(" -Opcao -a: algoritmo Ex: ./trabalho -a 1 (Euclidiana) ou 2 (Manhattan) \n");
            printf("----------------------------------------------------------------------------------------------------\n");
            break;
        case 't':
            numeroDeColunas = ExtrairValorInteiro(optarg);
            if (strstr(optarg, "test") != NULL)
            {
                arquivoTeste = AbrirArquivo(arquivoTeste, optarg);
                while ((leitura = getline(&linha, &tamanho, arquivoTeste)) != -1)
                {
                    matrizTeste.data[numeroDeLinhasDeTeste] = (float *)malloc(numeroDeColunas * sizeof(float));
                    PreencherColunas(linha, numeroDeLinhasDeTeste, &matrizTeste);
                    numeroDeLinhasDeTeste++;
                }
                fclose(arquivoTeste);
            }
            break;
        case 'n':
            numeroDeColunas = ExtrairValorInteiro(optarg);
            if (strstr(optarg, "train") != NULL)
            {
                arquivoTreino = AbrirArquivo(arquivoTreino, optarg);
                while ((leitura = getline(&linha, &tamanho, arquivoTreino)) != -1)
                {
                    matrizTreino.data[numeroDeLinhasDeTreino] = (float *)malloc(numeroDeColunas * sizeof(float));
                    PreencherColunas(linha, numeroDeLinhasDeTreino, &matrizTreino);
                    numeroDeLinhasDeTreino++;
                }
                fclose(arquivoTreino);
            }
            break;
        case 'p':
            numeroDeThreads = atoi(optarg);
            menoresResultados = (float *)malloc(numeroDeThreads * sizeof(float));
            break;
        default:
            return (0);
        }
    }

    printf("Numero de threads: %d\n", numeroDeThreads);
    int i;
    HANDLE vetorDeThread[numeroDeThreads];
    for (i = 0; i < numeroDeThreads; i++)
    {
        menoresResultados[i] = 0;
        vetorDeThread[i] = CreateThread(NULL, 0, FuncaoThread, (void *)i, 0, NULL);
    }

    for (i = 0; i < numeroDeThreads; i++)
    {
        CloseHandle(vetorDeThread[i]);
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Tempo de execucao: %f", time_spent);
    // 1 teste com todos os treinos
    // getchar: somente para pausar o console.
    getchar();
}