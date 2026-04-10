#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_ITENS 10

typedef struct {
    int peso;
    int valor;
} Item;

typedef struct {
    int genes[N_ITENS];
    int fitness;
} Individuo;

// Dados do problema
Item pesos_e_valores[N_ITENS] = {
    {4, 30}, {8, 10}, {8, 30}, {25, 75}, {2, 10},
    {50, 100}, {6, 300}, {12, 50}, {100, 400}, {8, 300}
};

int peso_maximo = 100;

int calcular_fitness(int genes[]) {
    int peso_total = 0;
    int valor_total = 0;

    for (int i = 0; i < N_ITENS; i++) {
        if (genes[i] == 1) {
            peso_total += pesos_e_valores[i].peso;
            valor_total += pesos_e_valores[i].valor;
        }
    }

    if (peso_total > peso_maximo) {
        return -1; // Peso excedido
    }
    return valor_total;
}

void criar_individuo(Individuo *ind) {
    for (int i = 0; i < N_ITENS; i++) {
        ind->genes[i] = rand() % 2;
    }
    ind->fitness = calcular_fitness(ind->genes);
}

double calcular_media_fitness(Individuo populacao[], int tamanho_populacao) {
    double soma = 0;
    int validos = 0;
    for (int i = 0; i < tamanho_populacao; i++) {
        if (populacao[i].fitness >= 0) {
            soma += populacao[i].fitness;
            validos++;
        }
    }
    return (validos == 0) ? 0 : soma / validos;
}

// Lógica de sorteio para a roleta
int sortear(Individuo pais[], int n_pais, int fitness_total, int indice_a_ignorar) {
    if (fitness_total <= 0) return rand() % n_pais;

    int temp_total = fitness_total;
    if (indice_a_ignorar != -1) {
        temp_total -= pais[indice_a_ignorar].fitness;
    }

    float valor_sorteado = (float)rand() / (float)RAND_MAX;
    float acumulado = 0;

    for (int i = 0; i < n_pais; i++) {
        if (i == indice_a_ignorar) continue;
        
        acumulado += (float)pais[i].fitness / temp_total;
        if (acumulado >= valor_sorteado) {
            return i;
        }
    }
    return n_pais - 1;
}

void evoluir(Individuo populacao[], int tamanho_populacao, float taxa_mutacao) {
    Individuo pais[tamanho_populacao];
    int n_pais = 0;
    int fitness_total_pais = 0;

    for (int i = 0; i < tamanho_populacao; i++) {
        if (populacao[i].fitness >= 0) {
            pais[n_pais] = populacao[i];
            fitness_total_pais += pais[n_pais].fitness;
            n_pais++;
        }
    }

    // Se ninguém sobreviveu, reinicia a população (garantia de segurança)
    if (n_pais < 2) {
        for (int i = 0; i < tamanho_populacao; i++) criar_individuo(&populacao[i]);
        return;
    }

    Individuo novos_filhos[tamanho_populacao];

    // REPRODUÇÃO (Crossover)
    for (int i = 0; i < tamanho_populacao; i++) {
        int idx_pai = sortear(pais, n_pais, fitness_total_pais, -1);
        int idx_mae = sortear(pais, n_pais, fitness_total_pais, idx_pai);

        int meio = N_ITENS / 2;
        for (int j = 0; j < N_ITENS; j++) {
            if (j < meio) novos_filhos[i].genes[j] = pais[idx_pai].genes[j];
            else novos_filhos[i].genes[j] = pais[idx_mae].genes[j];
        }

        // MUTAÇÃO
        if (((float)rand() / (float)RAND_MAX) < taxa_mutacao) {
            int pos = rand() % N_ITENS;
            novos_filhos[i].genes[pos] = !novos_filhos[i].genes[pos];
        }
        
        novos_filhos[i].fitness = calcular_fitness(novos_filhos[i].genes);
    }

    // Atualiza a população original
    for (int i = 0; i < tamanho_populacao; i++) {
        populacao[i] = novos_filhos[i];
    }
}

int main() {
    srand(time(NULL));

    int N_CROMOSSOMOS;
    int GERACOES;
    float TAXA_MUTACAO;
    
    printf("Digite o tamanho da populacao: ");
    scanf("%d", &N_CROMOSSOMOS);

    if (N_CROMOSSOMOS <= 0) {
        printf("Tamanho invalido!\n");
        return 1;
    }
    
    printf("Digite a taxa de mutacao: ");
    scanf("%f", &TAXA_MUTACAO);

    if (TAXA_MUTACAO < 0.0 || TAXA_MUTACAO > 1.0) {
        printf("Taxa de mutacao deve estar entre 0 e 1!\n");
        return 1;
    }
    
    printf("Digite o numero de geracoes: ");
    scanf("%d", &GERACOES);

    if (GERACOES <= 0) {
        printf("Numero de geracoes deve ser maior que 0!\n");
        return 1;
    }
    
    Individuo populacao[N_CROMOSSOMOS];
    for (int i = 0; i < N_CROMOSSOMOS; i++) {
        criar_individuo(&populacao[i]);
    }

    printf("Geracao: 0 | Media de valor na mochila: %.2f\n", calcular_media_fitness(populacao, N_CROMOSSOMOS));

    for (int g = 1; g <= GERACOES; g++) {
        evoluir(populacao, N_CROMOSSOMOS, TAXA_MUTACAO);
        printf("Geracao: %d | Media de valor na mochila: %.2f\n", g, calcular_media_fitness(populacao, N_CROMOSSOMOS));
    }

    printf("\nPeso maximo: %dg\n\nItens disponiveis:\n", peso_maximo);
    for (int i = 0; i < N_ITENS; i++) {
        printf("Item %d: %dg | R$%d\n", i + 1, pesos_e_valores[i].peso, pesos_e_valores[i].valor);
    }

    printf("\nExemplos de boas solucoes (Genes):\n");
    int impressos = 0;
    for (int i = 0; i < N_CROMOSSOMOS && impressos < 5; i++) {
        if (populacao[i].fitness >= 0) {
            printf("[");
            for (int j = 0; j < N_ITENS; j++) printf(" %d ", populacao[i].genes[j]);
            printf("] - Valor: %d\n", populacao[i].fitness);
            impressos++;
        }
    }

    return 0;
}
