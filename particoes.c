
#include "funcionarios.h"
#include <stdlib.h>
#include <limits.h>

#include "particoes.h"

void classificacao_interna(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc) {
    rewind(arq); //posiciona cursor no inicio do arquivo


    int reg = 0;

    while (reg != nFunc) {
        //le o arquivo e coloca no vetor
        TFunc *v[M];
        int i = 0;
        while (!feof(arq)) {
            fseek(arq, (reg) * tamanho_registro(), SEEK_SET);
            v[i] = le_funcionario(arq);
       //     imprime_funcionario(v[i]);
            i++;
            reg++;
            if(i>=M) break;
        }

        //ajusta tamanho M caso arquivo de entrada tenha terminado antes do vetor
        if (i != M) {
            M = i;
        }

        //faz ordenacao
        for (int j = 1; j < M; j++) {
            TFunc *f = v[j];
            i = j - 1;
            while ((i >= 0) && (v[i]->cod > f->cod)) {
                v[i + 1] = v[i];
                i = i - 1;
            }
            v[i + 1] = f;
        }

        //cria arquivo de particao e faz gravacao
        char *nome_particao = nome_arquivos_saida->nome;
        nome_arquivos_saida = nome_arquivos_saida->prox;
        printf("\n%s\n", nome_particao);
        FILE *p;
        if ((p = fopen(nome_particao, "wb+")) == NULL) {
            printf("Erro criar arquivo de saida\n");
        } else {
            for (int i = 0; i < M; i++) {
                fseek(p, (i) * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[i], p);
                imprime_funcionario(v[i]);
            }
            fclose(p);
        }
        for(int jj = 0; jj<M; jj++)
            free(v[jj]);
    }
}

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
    
            while (!reservatorio_cheio && lidos < nFunc) {
                if (fseek(arq, (long) lidos * tamanho_registro(), SEEK_SET) != 0 ||
                    !(novo = le_funcionario(arq))) { erro = 1; break; }
                lidos++;
                if (novo->cod >= ultima_chave) { memoria[ativos++] = novo; break; }
                if (fseek(reservatorio, (long) guardados * tamanho_registro(), SEEK_SET) != 0) {
                    free(novo); erro = 1; break;
                }
                salva_funcionario(novo, reservatorio);
                free(novo); guardados++;
                if (guardados == n) reservatorio_cheio = 1;
            }
        }
        if (ferror(saida)) erro = 1;
        if (fclose(saida) != 0) erro = 1;
        if (erro) break;
        particoes++; nomes = nomes->prox;
        /* O reservatorio e carregado primeiro na proxima particao. */
        for (int i = 0; i < guardados && !erro; i++) {
            if (fseek(reservatorio, (long) i * tamanho_registro(), SEEK_SET) != 0 ||
                !(memoria[ativos] = le_funcionario(reservatorio))) { erro = 1; break; }
            ativos++;
        }
        guardados = 0;
        while (ativos < M && lidos < nFunc && !erro) {
            if (fseek(arq, (long) lidos * tamanho_registro(), SEEK_SET) != 0 ||
                !(memoria[ativos] = le_funcionario(arq))) { erro = 1; break; }
            ativos++; lidos++;
        }
    }
    for (int i = 0; i < ativos; i++) free(memoria[i]);
    free(memoria);
    fclose(reservatorio);
    if (erro) { printf("Erro ao gerar particoes.\n"); return -1; }
    printf("\n%d particao(oes) gerada(s).\n", particoes);
    return particoes;
}
