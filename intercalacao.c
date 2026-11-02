#include "intercalacao.h"
#include "funcionarios.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
typedef struct {
    int particao;
    int codigo;
} TNoVencedor;
TNoVencedor escolher_vencedor(TNoVencedor a, TNoVencedor b) {
    if (a.codigo <= b.codigo) {
        return a;
    }

    return b;
}

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

void intercalacao_arvore_de_vencedores(TPilha **pilha, int *vetTop, char *nome_arquivo_saida, int num_p){
    //implementar primeira parte do trabalho
    // Intercalacao utilizando Arvore Binaria de Vencedores

    // Etapa 1: preparar os candidatos de cada particao
    if (num_p <= 0) { //Verifica se existe pelo menos uma partição para intercalar. Se não existir, a função termina.
    return;
}

    int num_folhas = 1;

    while (num_folhas < num_p) {
    num_folhas *= 2;
}
    TNoVencedor arvore[2 * num_folhas]; //se tivermos quatro partições (num_p = 4), reservaremos oito posições.
        for (int i = num_p; i < num_folhas; i++) {
            arvore[num_folhas + i].particao = -1; //particao = -1: indica que a folha não pertence a nenhuma partição real.
            arvore[num_folhas + i].codigo = INT_MAX; //codigo = INT_MAX: faz com que essa folha seja tratada como vazia nas comparações.
    }

        for (int i = 0; i < num_p; i++) { //o for vai percorrer todas as partições
            arvore[num_folhas + i].particao = i; //o num_p + i identifica a folha correspondente à partição.
            arvore[num_folhas + i].codigo = INT_MAX; //.codigo = INT_MAX inicializa a folha com um valor provisório, indicando que ainda não carregamos um candidato válido.
         //consulta o funcionário que está no topo de cada pilha.
         if (pilha[i] != NULL && vetTop[i] >= 0) { //pilha[i] acessa a pilha da partição i. //vetTop[i] indica a posição do topo dessa pilha.
            TFunc *func = pilha[i][vetTop[i]].info; //.info acessa o funcionário armazenado.

            if (func != NULL) {
                arvore[num_folhas + i].codigo = func->cod; //func->cod obtém o código que será comparado na árvore.
            }
        }
    }
    // Etapa 2: construir a arvore de vencedores
        for (int i = num_folhas - 1; i >= 1; i--) {
            arvore[i] = escolher_vencedor(
                arvore[2 * i],
                arvore[2 * i + 1]
            );
}

    // Etapa 3: selecionar e gravar o vencedor
        FILE *saida = fopen(nome_arquivo_saida, "wb"); //FILE *saida declara um ponteiro para o arquivo.//fopen(nome_arquivo_saida, "wb") abre o arquivo para escrita binária. // MAS ATENÇÃO: o modo "wb" cria o arquivo se ele não existir, mas apaga seu conteúdo anterior caso já exista.

    if (saida == NULL) { //verifica se houve algum erro ao abrir o arquivo.
        printf("Erro ao abrir o arquivo de saida.\n");
        return; //encerra a função caso não seja possível continuar.
    }

    while (arvore[1].codigo != INT_MAX) { //arvore[1] representa a raiz da árvore, onde fica o vencedor da comparação. //codigo != INT_MAX indica que ainda existe um candidato a processar.
    int pos_vencedor = arvore[1].particao;  //guarda o número da partição que contém o funcionário vencedor.

    TFunc *vencedor = pilha[pos_vencedor][vetTop[pos_vencedor]].info; //recupera o registro completo do funcionário vencedor, utilizando a partição identificada pela árvore.

    salva_funcionario(vencedor, saida); //utiliza salva_funcionario(), que já existe no projeto, para gravar esse funcionário no arquivo binário de saída.
    vetTop[pos_vencedor]--;

    int pos_folha = num_folhas + pos_vencedor;

    arvore[pos_folha].codigo = INT_MAX;

    TFunc *proximo = NULL;

    if (vetTop[pos_vencedor] >= 0) {
        proximo = pilha[pos_vencedor][vetTop[pos_vencedor]].info;
    }
    if (proximo != NULL) {
    arvore[pos_folha].codigo = proximo->cod;
}


    int pai = pos_folha / 2;  //pos_folha / 2 encontra o pai da folha atualizada.

    while (pai >= 1) {
        arvore[pai] = escolher_vencedor( //escolher_vencedor() compara novamente os dois filhos.
            arvore[2 * pai],
            arvore[2 * pai + 1]
        );

        pai = pai / 2;  //sobe um nível na árvore.
}

    } // Fim do while principal

    // Etapa 4: atualizar a arvore ate finalizar
    fclose(saida);
}

