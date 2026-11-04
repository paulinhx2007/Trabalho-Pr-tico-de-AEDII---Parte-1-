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

Intercalação das partições utilizando **Árvore Binária de Vencedores**.

## Funções implementadas

### `intercalacao.h`

```c
void intercalacao_arvore_de_vencedores(char *nome_arquivo_saida, int num_p, Lista *nome_particoes);
```

### `intercalacao.c`

```c
/* Carrega em p->func o registro da posicao aux_p. NULL = particao esgotada */
static void carrega(TVet *p) {
    fseek(p->f, (long) p->aux_p * tamanho_registro(), SEEK_SET);
    p->func = le_funcionario(p->f);
}

/* Devolve o indice da particao com menor cod. func == NULL conta como "infinito" */
static int vencedor(int a, int b, TVet *v) {
    if (v[a].func == NULL) return b;
    if (v[b].func == NULL) return a;
    return (v[a].func->cod <= v[b].func->cod) ? a : b;
}

/* Monta a arvore de baixo para cima (P = numero de folhas) */
static void constroi(int arv[], TVet *v, int P) {
    for (int i = 0; i < P; i++)
        arv[P + i] = i;                 /* folhas guardam o indice da particao */
    for (int p = P - 1; p >= 1; p--)
        arv[p] = vencedor(arv[2 * p], arv[2 * p + 1], v);
}

/* Refaz so o caminho da folha i ate a raiz */
static void atualiza(int arv[], TVet *v, int P, int i) {
    int j = (P + i) / 2;
    while (j >= 1) {
        arv[j] = vencedor(arv[2 * j], arv[2 * j + 1], v);
        j = j / 2;
    }
}

void intercalacao_arvore_de_vencedores(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {
    int P = 1, erro = 0, escritos = 0;
    while (P < num_p) P *= 2;           /* completa ate potencia de 2 */

    TVet *v = calloc((size_t) P, sizeof(TVet));  /* folhas extras: func = NULL, f = NULL */
    int *arv = malloc(2 * (size_t) P * sizeof(int));
    FILE *out = fopen(nome_arquivo_saida, "wb");

    if (!v || !arv || !out) {
        printf("Erro ao alocar memoria ou criar %s.\n", nome_arquivo_saida);
        erro = 1;
    }

    /* Abre cada particao e carrega o primeiro registro */
    for (int i = 0; i < num_p && !erro; i++) {
        if (!nome_particoes) {
            printf("Faltam nomes de particoes.\n");
            erro = 1;
            break;
        }
        v[i].f = fopen(nome_particoes->nome, "rb");
        if (!v[i].f) {
            printf("Erro ao abrir %s.\n", nome_particoes->nome);
            erro = 1;
            break;
        }
        v[i].aux_p = 0;
        carrega(&v[i]);
        nome_particoes = nome_particoes->prox;
    }

    if (!erro) {
        constroi(arv, v, P);

        /* arv[1] = particao com o menor registro neste momento */
        while (v[arv[1]].func != NULL) {
            int i = arv[1];

            fseek(out, (long) escritos * tamanho_registro(), SEEK_SET);
            salva_funcionario(v[i].func, out);
            escritos++;

            free(v[i].func);
            v[i].aux_p++;
            carrega(&v[i]);             /* proximo registro da mesma particao */
            atualiza(arv, v, P, i);
        }
    }

    /* Limpeza */
    if (v) {
        for (int i = 0; i < P; i++) {
            if (v[i].func) free(v[i].func);
            if (v[i].f) fclose(v[i].f);
        }
    }
    if (out) fclose(out);
    free(v);
    free(arv);
}
```


Descrição das funções:

- carrega: lê da partição o registro da posição atual. Devolve NULL quando a partição acabou.
- vencedor: compara dois nós e devolve o índice da partição com o menor cod. Partição esgotada conta como "infinito".
- constroi: monta a árvore de baixo para cima, a partir do primeiro registro de cada partição.
- atualiza: depois de consumir um registro, refaz só o caminho da folha até a raiz.
- intercalacao_arvore_de_vencedores: abre as partições, executa o laço principal (grava o menor registro, lê o próximo da mesma partição, atualiza a árvore) e grava o arquivo final.

Uso: no menu do main.c, opção 9 (depois de gerar as partições com a opção 5 ou 6).

**-------------------------------------------------------------------------------------------------------------------------------------------**


Funções implementadas:


### intercalacao.c

**Estrutura auxiliar criada:**

```c
typedef struct {
    int particao;
    int codigo;
} TNoVencedor;
```

A estrutura `TNoVencedor` armazena o índice da partição e o código do funcionário utilizado nas comparações da árvore.

**Função auxiliar implementada:**

```c
TNoVencedor escolher_vencedor(TNoVencedor a, TNoVencedor b);
```

Compara dois candidatos e retorna aquele que possui o menor código. Essa função é utilizada tanto na construção quanto na atualização da árvore.

**Função principal implementada:**

```c
void intercalacao_arvore_de_vencedores(TPilha **pilha, int *vetTop, char *nome_arquivo_saida, int num_p);
```

Realiza a intercalação dos funcionários armazenados nas partições, utilizando uma Árvore Binária de Vencedores.

O algoritmo inicialmente prepara os candidatos de cada partição e constrói a árvore por meio de comparações entre os códigos dos funcionários.

A raiz da árvore identifica a partição que contém o menor funcionário disponível. Esse funcionário é gravado no arquivo de saída, e o topo da pilha correspondente é atualizado.

Em seguida, o algoritmo atualiza o caminho da folha modificada até a raiz, identificando o próximo vencedor. O procedimento continua até que todas as partições sejam processadas.

A implementação também considera quantidades de partições que não são potências de 2, utilizando folhas auxiliares com o valor `INT_MAX`.

### intercalacao.h

**Declaração da função principal:**

```c
void intercalacao_arvore_de_vencedores(TPilha **pilha, int *vetTop, char *nome_arquivo_saida, int num_p);
```

### Testes realizados

**Teste 1 — Seleção Natural**

- Base de 20 funcionários.
- Geração de 3 partições.
- Intercalação utilizando Árvore de Vencedores.
- Comparação com a Intercalação Básica.
- Resultado: arquivos binários idênticos.

**Teste 2 — Classificação Interna**

- Base de 20 funcionários.
- Geração de 10 partições, com 2 funcionários cada.
- Intercalação utilizando Árvore de Vencedores.
- Comparação com a Intercalação Básica.
- Resultado: arquivos binários idênticos.

Os dois testes foram aprovados, produzindo os 20 funcionários em ordem crescente de código.
