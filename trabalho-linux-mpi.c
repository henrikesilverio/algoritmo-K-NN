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

int PreencherMatriz(int numeroDeColunas, char *caminho, float **matriz)
{
    FILE *arquivo = AbrirArquivo(arquivo, caminho);
    int numeroDeLinhasDeTeste = 0;
    char *linha = NULL;
    size_t tamanho = 0;
    ssize_t leitura;

    while ((leitura = getline(&linha, &tamanho, arquivo)) != -1)
    {
        matriz[numeroDeLinhasDeTeste] = (float *)malloc(numeroDeColunas * sizeof(float));
        PreencherColunas(linha, numeroDeLinhasDeTeste, numeroDeColunas, matriz);
        numeroDeLinhasDeTeste++;
    }
    return numeroDeLinhasDeTeste;
}

float Abs(float valor)
{
    if (valor < 0)
    {
        return valor * -1;
    }
    else
    {
        return valor;
    }
}

float DistanciaManhattan(float *treino, float *teste, int numeroDeColunas)
{
    float somatoria = 0;
    int i;
    for (i = 0; i < numeroDeColunas; i++)
    {
        somatoria += Abs((treino[i] - teste[i]));
    }
    return somatoria;
}

float Calcula(float **matriz, float *linha, int numeroDeColunas, int inicio, int fim, int passo)
{
    float resultado = 0, retorno = 0;
    int i;
    for (i = inicio; i < fim; i += passo)
    {
        retorno = DistanciaManhattan(matriz[i], linha, numeroDeColunas);
        if (resultado < retorno)
        {
            resultado = retorno;
        }
    }
    return resultado;
}

int main(int argc, char **argv)
{
    int i, numeroDoProcesso, numeroDeProcessos;
    int numeroDeLinhasDeTreino = 0;
    int numeroDeLinhasDeTeste = 0;
    int indiceCorrente = 0;
    int tagIndice = 1, tagMax = 2, tagLinha = 3, tagResultado = 4;
    int numeroDeColunas = ExtrairValorInteiro(argv[1]);
    float maiorValor = 0;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &numeroDoProcesso);
    MPI_Comm_size(MPI_COMM_WORLD, &numeroDeProcessos);

    while (1)
    {
        if (numeroDoProcesso == 0)
        {
            if (matrizTeste == NULL)
            {
                matrizTeste = (float **)malloc(3000 * sizeof(float *));
                numeroDeLinhasDeTeste = PreencherMatriz(numeroDeColunas, argv[1], matrizTeste);
            }

            for (i = 0; i < numeroDeProcessos; i++)
            {
                MPI_Send(&indiceCorrente, 1, MPI_INT, i, tagIndice, MPI_COMM_WORLD);
                MPI_Send(&numeroDeLinhasDeTeste, 1, MPI_INT, i, tagMax, MPI_COMM_WORLD);
                MPI_Send(matrizTeste[indiceCorrente], numeroDeColunas, MPI_FLOAT, i, tagLinha, MPI_COMM_WORLD);
            }

            if (matrizTreino == NULL)
            {
                matrizTreino = (float **)malloc(10000 * sizeof(float *));
                numeroDeLinhasDeTreino = PreencherMatriz(numeroDeColunas, argv[2], matrizTreino);
            }
            maiorValor = Calcula(matrizTreino, matrizTeste[indiceCorrente], numeroDeColunas, numeroDoProcesso, numeroDeLinhasDeTreino, numeroDeProcessos);

            for (i = 1; i < numeroDeProcessos; i++) 
            {
                float resultado = 0;
                MPI_Recv(&resultado, 1, MPI_FLOAT, i, tagResultado, MPI_COMM_WORLD, &status);
                if (resultado > maiorValor) 
                {
                    maiorValor = resultado;
                }
            }

            indiceCorrente++;
            if (indiceCorrente == numeroDeLinhasDeTeste)
            {
                free(matrizTeste);
                free(matrizTreino);
                break;
            }
        }
        else
        {
            int indice, max;
            float resultado = 0;
            MPI_Recv(&indice, 1, MPI_INT, 0, tagIndice, MPI_COMM_WORLD, &status);
            MPI_Recv(&max, 1, MPI_INT, 0, tagMax, MPI_COMM_WORLD, &status);
            float linha[numeroDeColunas];
            MPI_Recv(linha, numeroDeColunas, MPI_FLOAT, 0, tagLinha, MPI_COMM_WORLD, &status);
            if (matrizTreino == NULL)
            {
                matrizTreino = (float **)malloc(10000 * sizeof(float *));
                numeroDeLinhasDeTreino = PreencherMatriz(numeroDeColunas, argv[2], matrizTreino);
            }
            resultado = Calcula(matrizTreino, linha, numeroDeColunas, numeroDoProcesso, numeroDeLinhasDeTreino, numeroDeProcessos);
            MPI_Send(&resultado, 1, MPI_FLOAT, 0, tagResultado, MPI_COMM_WORLD);

            if ((indice + 1) == max)
            {
                free(matrizTreino);
                break;
            }
        }
    }
    MPI_Finalize();
    return 0;
}