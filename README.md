# Trabalho-Pratico-de-AEDII---Parte-1-
Objetivo: Desenvolver o algoritmo de Geração de Partições utilizando o método de Seleção Natural e Intercalação utilizando Árvores Binárias de Vencedores

## Geração de Partições

Funções implementadas:

- main.c
- particoes.c:

```c
int selecao_natural(FILE *arq, Lista *nomes, int M, int nFunc, int n) {
    TFunc **memoria;
    FILE *reservatorio;
    int ativos = 0, lidos = 0, guardados = 0, particoes = 0;
    int erro = 0;

    if (!arq || !nomes || M <= 0 || n <= 0 || n > M || nFunc <= 0) {
        printf("Parametros invalidos ou base vazia.\n");
        return -1;
    }

    memoria = calloc((size_t) M, sizeof(*memoria));
    reservatorio = tmpfile();
    if (!memoria || !reservatorio) {
        printf("Erro ao criar memoria ou reservatorio.\n");
        free(memoria);
        if (reservatorio) fclose(reservatorio);
        return -1;
    }

    rewind(arq);
    while (ativos < M && lidos < nFunc) {
        if (fseek(arq, (long) lidos * tamanho_registro(), SEEK_SET) != 0 ||
            !(memoria[ativos] = le_funcionario(arq))) {
            erro = 1;
            break;
        }
        ativos++;
        lidos++;
    }

    while (!erro && ativos > 0) {
        FILE *saida;
        int escritos = 0, reservatorio_cheio = 0;

        if (!nomes) {
            printf("Faltam nomes para as particoes.\n");
            erro = 1;
            break;
        }

        saida = fopen(nomes->nome, "wb+");
        if (!saida) {
            printf("Erro ao criar %s.\n", nomes->nome);
            erro = 1;
            break;
        }

        printf("\nParticao %s:\n", nomes->nome);
        while (ativos > 0 && !erro) {
            int menor = 0, ultima_chave;
            TFunc *novo = NULL;

            for (int i = 1; i < ativos; i++)
                if (memoria[i]->cod < memoria[menor]->cod) menor = i;

            ultima_chave = memoria[menor]->cod;

            if (fseek(saida, (long) escritos * tamanho_registro(), SEEK_SET) != 0) {
                erro = 1;
                break;
            }

            salva_funcionario(memoria[menor], saida);
            imprime_funcionario(memoria[menor]);
            escritos++;

            free(memoria[menor]);
            memoria[menor] = memoria[--ativos];
            memoria[ativos] = NULL;

            while (!reservatorio_cheio && lidos < nFunc) {
                if (fseek(arq, (long) lidos * tamanho_registro(), SEEK_SET) != 0 ||
                    !(novo = le_funcionario(arq))) {
                    erro = 1;
                    break;
                }
                lidos++;

                if (novo->cod >= ultima_chave) {
                    memoria[ativos++] = novo;
                    break;
                }

                if (fseek(reservatorio, (long) guardados * tamanho_registro(), SEEK_SET) != 0) {
                    free(novo);
                    erro = 1;
                    break;
                }

                salva_funcionario(novo, reservatorio);
                free(novo);
                guardados++;

                if (guardados == n) reservatorio_cheio = 1;
            }
        }

        if (ferror(saida)) erro = 1;
        if (fclose(saida) != 0) erro = 1;
        if (erro) break;

        particoes++;
        nomes = nomes->prox;

        /* O reservatorio e carregado primeiro na proxima particao. */
        for (int i = 0; i < guardados && !erro; i++) {
            if (fseek(reservatorio, (long) i * tamanho_registro(), SEEK_SET) != 0 ||
                !(memoria[ativos] = le_funcionario(reservatorio))) {
                erro = 1;
                break;
            }
            ativos++;
        }

        guardados = 0;

        while (ativos < M && lidos < nFunc && !erro) {
            if (fseek(arq, (long) lidos * tamanho_registro(), SEEK_SET) != 0 ||
                !(memoria[ativos] = le_funcionario(arq))) {
                erro = 1;
                break;
            }
            ativos++;
            lidos++;
        }
    }

    for (int i = 0; i < ativos; i++) free(memoria[i]);
    free(memoria);
    fclose(reservatorio);

    if (erro) {
        printf("Erro ao gerar particoes.\n");
        return -1;
    }

    printf("\n%d particao(oes) gerada(s).\n", particoes);
    return particoes;
}
```

- particoes.h:

```c
int selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n);
```

## Intercalação

Funções implementadas:
