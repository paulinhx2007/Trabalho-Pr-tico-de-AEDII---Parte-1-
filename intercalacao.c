#include "intercalacao.h"
#include "funcionarios.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {

    int fim = 0; //variavel que controla fim do procedimento
    FILE *out; //declara ponteiro para arquivo

    //abre arquivo de saida para escrita
    if ((out = fopen(nome_arquivo_saida, "wb")) == NULL) {
        printf("Erro ao abrir arquivo de sa?da\n");
    } else {
        //cria vetor de particoes
        TVet v[num_p];

        //abre arquivos das particoes, colocando variavel de arquivo no campo f do vetor
        //e primeiro funcionario do arquivo no campo func do vetor
        for (int i=0; i < num_p; i++) {
            v[i].f = fopen(nome_particoes->nome, "rb");
            v[i].aux_p = 0;
            if (v[i].f != NULL) {
                fseek(v[i].f, v[i].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[i].f);
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posi??o do vetor
                    v[i].func = funcionario(INT_MAX, "","","",0);
                }
                else {
                    //conseguiu ler funcionario, coloca na posi??o atual do vetor
                    v[i].func = f;
                }
            }
            else {
                fim = 1;
            }
            nome_particoes = nome_particoes->prox;
        }

        int aux = 0;
        while (!(fim)) { //conseguiu abrir todos os arquivos
            int menor = INT_MAX;
            int pos_menor;
            //encontra o funcionario com menor chave no vetor
            for(int i = 0; i < num_p; i++){
                if(v[i].func->cod < menor){
                    menor = v[i].func->cod;
                    pos_menor = i;
                }
            }
            if (menor == INT_MAX) {
                fim = 1; //terminou processamento
            }
            else {
                //salva funcionario no arquivo de saída
                fseek(out, aux * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[pos_menor].func, out);
                //atualiza posição pos_menor do vetor com pr?ximo funcionario do arquivo
                v[pos_menor].aux_p++;
                fseek(v[pos_menor].f, v[pos_menor].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[pos_menor].f);
                aux++;
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posiçao do vetor
                    v[pos_menor].func = funcionario(INT_MAX, "", "", "",0.0);
                }
                else {
                    v[pos_menor].func = f;
                }

            }
        }

        //fecha arquivos das partiÇões de entrada
        for(int i = 0; i < num_p; i++){
            fclose(v[i].f);
        //    free(v[i].func);
        }
        //fecha arquivo de saída
        fclose(out);
    }
}

/* ---------- Arvore binaria de vencedores ---------- */

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
        arv[P + i] = i;                          /* folhas guardam o indice da particao */
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
    while (P < num_p) P *= 2;                    /* completa ate potencia de 2 */

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
            carrega(&v[i]);                      /* proximo registro da mesma particao */
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
