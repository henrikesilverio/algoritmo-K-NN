#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <mpi.h>

float **matrizTeste;
float **matrizTreino;

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

void PreencherColunas(char *linhaDoArquivo, int indiceDaLinhaCorrente, int numeroDeColunas, float **matriz)
{
    char *celula;
    int coluna = 0;
    celula = strtok(linhaDoArquivo, ",");
    while (celula != NULL && coluna != numeroDeColunas)
    {
        matriz[indiceDaLinhaCorrente][coluna] = atof(celula);
        celula = strtok(NULL, ",");
        coluna++;
    }
}

float DistanciaManhattan(float *treino, float *teste, int numeroDeColunas)
{
    float somatoria = 0;
    int i;
    for (i = 0; i < numeroDeColunas; i++)
    {
        somatoria += abs((treino[i] - teste[i]));
    }
    return somatoria;
}

int main(int argc, char **argv)
{
    int i, j, numeroDoProcesso, numeroDeProcessos;
    int numeroDeLinhasDeTreino = 0;
    int numeroDeLinhasDeTeste = 0;
    int tagNumeroDeColunas = 1, tagLinha = 2;
    int numeroDeColunas = ExtrairValorInteiro(argv[1]);

    matrizTeste = (float **)malloc(3000 * sizeof(float *));
    matrizTreino = (float **)malloc(10000 * sizeof(float *));

    FILE *arquivoTeste;
    FILE *arquivoTreino;
    char *linha = NULL;
    size_t tamanho = 0;
    ssize_t leitura;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &numeroDoProcesso);
    MPI_Comm_size(MPI_COMM_WORLD, &numeroDeProcessos);

    arquivoTeste = AbrirArquivo(arquivoTeste, argv[1]);
    arquivoTreino = AbrirArquivo(arquivoTreino, argv[2]);

    while ((leitura = getline(&linha, &tamanho, arquivoTeste)) != -1)
    {
        if (numeroDoProcesso == 0)
        {
            matrizTeste[numeroDeLinhasDeTeste] = (float *)malloc(numeroDeColunas * sizeof(float));
            PreencherColunas(linha, numeroDeLinhasDeTeste, numeroDeColunas, matrizTeste);
            for (i = 0; i < numeroDeProcessos; i++)
            {
                //MPI_Send(&numeroDeColunas, 1, MPI_INT, i, tagNumeroDeColunas, MPI_COMM_WORLD);
                MPI_Send(matrizTeste[numeroDeLinhasDeTeste], numeroDeColunas, MPI_FLOAT, i, tagLinha, MPI_COMM_WORLD);
                // Envia uma mensagem para processo “1”
            }
            numeroDeLinhasDeTeste++;
        }
        else
        {
            //MPI_Recv(&numeroDeColunas, 1, MPI_INT, 0, tagNumeroDeColunas, MPI_COMM_WORLD, &status);
            float linha[numeroDeColunas];
            MPI_Recv(linha, numeroDeColunas, MPI_FLOAT, 0, tagLinha, MPI_COMM_WORLD, &status);
            // Recebe uma mensagem provinda do processo “0”
            //printf("Linha %f processo %d\n", linha[0], numeroDoProcesso);
        }
    }
    MPI_Finalize();
    return 0;
}