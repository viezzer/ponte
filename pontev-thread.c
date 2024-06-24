#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_PISTAS 20 // 10 pistas para cada conjunto
#define NUM_CARROS 1000
#define CAPACIDADE_PONTE 10

// Estruturas para representar a pista e a ponte
typedef struct {
    int id;
    int num_carros;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Pista;

typedef struct {
    int num_carros;
    int trancada;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Ponte;

Pista pistas[NUM_PISTAS];
Ponte ponte1, ponte2;
int tempo_intervalo = 1000000;

// Função para inicializar as pistas e as pontes
void inicializar() {
    for (int i = 0; i < NUM_PISTAS; i++) {
        pistas[i].id = i;
        pistas[i].num_carros = 0;
        pthread_mutex_init(&pistas[i].lock, NULL);
        pthread_cond_init(&pistas[i].cond, NULL);
    }
    ponte1.num_carros = 0;
    ponte1.trancada = 0;
    pthread_mutex_init(&ponte1.lock, NULL);
    pthread_cond_init(&ponte1.cond, NULL);
    
    ponte2.num_carros = 0;
    ponte2.trancada = 0;
    pthread_mutex_init(&ponte2.lock, NULL);
    pthread_cond_init(&ponte2.cond, NULL);
}

// Função para simular a passagem do carro pelas pistas e pelas pontes
void* carro(void* arg) {
    int id = *((int*)arg);

    // Passar pelo primeiro conjunto de pistas e pela primeira ponte
    for (int i = 0; i < NUM_PISTAS / 2; i++) {
        pthread_mutex_lock(&pistas[i].lock);
        while (pistas[i].num_carros >= CAPACIDADE_PONTE) {
            pthread_cond_wait(&pistas[i].cond, &pistas[i].lock);
        }
        pistas[i].num_carros++;
        pthread_mutex_unlock(&pistas[i].lock);
        if(i==5) {
            pthread_mutex_lock(&ponte1.lock);
            ponte1.num_carros--;
            pthread_cond_signal(&ponte1.cond);
            pthread_mutex_unlock(&ponte1.lock);
        }
        usleep(tempo_intervalo); // Simula tempo para passar pela pista

        // Tentar avançar para a próxima pista (se houver)
        int proxima_pista = i + 1;
        if (proxima_pista < NUM_PISTAS / 2) {
            // Entrada na ponte (última pista antes da ponte)
            if(i == ((NUM_PISTAS / 2)/2) - 1) {
                pthread_mutex_lock(&ponte1.lock);
                while (ponte1.num_carros >= CAPACIDADE_PONTE || ponte1.trancada == 1) {
                    pthread_cond_wait(&ponte1.cond, &ponte1.lock);
                }
                ponte1.num_carros++;
                pthread_mutex_unlock(&ponte1.lock);
                usleep(tempo_intervalo); // Simula tempo para passar pela ponte

                // Saída da pista atual
                pthread_mutex_lock(&pistas[i].lock);
                pistas[i].num_carros--;
                pthread_cond_signal(&pistas[i].cond);
                pthread_mutex_unlock(&pistas[i].lock);
            } else {
                pthread_mutex_lock(&pistas[proxima_pista].lock);
                while (pistas[proxima_pista].num_carros >= CAPACIDADE_PONTE) {
                    pthread_cond_wait(&pistas[proxima_pista].cond, &pistas[proxima_pista].lock);
                }
                // Saída da pista atual
                pthread_mutex_lock(&pistas[i].lock);
                pistas[i].num_carros--;
                pthread_cond_signal(&pistas[i].cond);
                pthread_mutex_unlock(&pistas[i].lock);
                pthread_mutex_unlock(&pistas[proxima_pista].lock);
                usleep(tempo_intervalo);
            }
        }
    }

    // Passar pelo segundo conjunto de pistas e pela segunda ponte
    for (int i = NUM_PISTAS / 2; i < NUM_PISTAS; i++) {
        pthread_mutex_lock(&pistas[i].lock);
        while (pistas[i].num_carros >= CAPACIDADE_PONTE) {
            pthread_cond_wait(&pistas[i].cond, &pistas[i].lock);
        }
        pistas[i].num_carros++;
        pthread_mutex_unlock(&pistas[i].lock);
        if(i==10) {
            pthread_mutex_lock(&pistas[9].lock);
            pistas[9].num_carros--;
            pthread_cond_signal(&pistas[9].cond);
            pthread_mutex_unlock(&pistas[9].lock);
        }
        if(i==15) {
            pthread_mutex_lock(&ponte2.lock);
            ponte2.num_carros--;
            pthread_cond_signal(&ponte2.cond);
            pthread_mutex_unlock(&ponte2.lock);
        }
        usleep(tempo_intervalo); // Simula tempo para passar pela pista

        // Tentar avançar para a próxima pista (se houver)
        int proxima_pista = i + 1;
        if (proxima_pista < NUM_PISTAS) {
            // Entrada na ponte (última pista antes da ponte)
            if(i == (NUM_PISTAS / 2) + 4) {
                pthread_mutex_lock(&ponte2.lock);
                while (ponte2.num_carros >= CAPACIDADE_PONTE || ponte2.trancada == 1) {
                    pthread_cond_wait(&ponte2.cond, &ponte2.lock);
                }
                ponte2.num_carros++;
                pthread_mutex_unlock(&ponte2.lock);
                usleep(tempo_intervalo); // Simula tempo para passar pela ponte

                // Saída da pista atual
                pthread_mutex_lock(&pistas[i].lock);
                pistas[i].num_carros--;
                pthread_cond_signal(&pistas[i].cond);
                pthread_mutex_unlock(&pistas[i].lock);
            } else {
                pthread_mutex_lock(&pistas[proxima_pista].lock);
                while (pistas[proxima_pista].num_carros >= CAPACIDADE_PONTE) {
                    pthread_cond_wait(&pistas[proxima_pista].cond, &pistas[proxima_pista].lock);
                }
                // Saída da pista atual
                pthread_mutex_lock(&pistas[i].lock);
                pistas[i].num_carros--;
                pthread_cond_signal(&pistas[i].cond);
                pthread_mutex_unlock(&pistas[i].lock);
                pthread_mutex_unlock(&pistas[proxima_pista].lock);
                usleep(tempo_intervalo);
            }
        }
    }

    free(arg);
    pthread_exit(NULL);
}

// Função para exibir o menu e definir o intervalo de tempo
void menu() {
    int escolha;
    printf("Escolha o fluxo de veículos por minuto na ponte:\n");
    printf("1. Ponte fechada\n");
    printf("2. 30 carros por minuto (2s de intervalo entre a geração de cada carro)\n");
    printf("3. 60 carros por minuto (1s de intervalo)\n");
    printf("4. 90 carros por minuto (0.67s de intervalo)\n");
    printf("Digite sua escolha (1-4): ");
    scanf("%d", &escolha);

    switch (escolha) {
        case 1:
            ponte1.trancada = 1;
            ponte2.trancada = 1;
            break;
        case 2:
            tempo_intervalo = 2000000; // 2 segundos
            break;
        case 3:
            tempo_intervalo = 1000000; // 1 segundo
            break;
        case 4:
            tempo_intervalo = 667000; // 0.67 segundos
            break;
        default:
            printf("Escolha inválida! Definindo para ponte fechada.\n");
            ponte1.trancada = 1; // Default para ponte fechada
            ponte2.trancada = 1;
            break;
    }
}

// Função para imprimir o estado do congestionamento periodicamente
void* imprimir_estado(void* arg) {
    while (1) {
        sleep(1); // Intervalo de tempo para imprimir o estado
        printf("------------------------------------------------------------------\n");
        for (int i = 0; i < NUM_PISTAS; i++) {
            pthread_mutex_lock(&pistas[i].lock);
            printf("Pista %d - %d\n", i, pistas[i].num_carros);
            pthread_mutex_unlock(&pistas[i].lock);
            if(i == 4) {
                pthread_mutex_lock(&ponte1.lock);
                printf("Ponte 1 - %d\n", ponte1.num_carros);
                pthread_mutex_unlock(&ponte1.lock);
            }
            if(i == 14) {
                pthread_mutex_lock(&ponte2.lock);
                printf("Ponte 2 - %d\n", ponte2.num_carros);
                pthread_mutex_unlock(&ponte2.lock);
            }
        }
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_CARROS];
    pthread_t thread_estado;
    inicializar();
    menu();

    // Criar thread para imprimir o estado do congestionamento
    pthread_create(&thread_estado, NULL, imprimir_estado, NULL);
    printf("-> Gerando carros!\n");
    for (int i = 0; i < NUM_CARROS; i++) {
        int* id = malloc(sizeof(int));
        *id = i;
        pthread_create(&threads[i], NULL, carro, (void*)id);
        usleep(tempo_intervalo); // Intervalo entre a criação de cada carro
    }
    printf("-> Todos carros gerados!\n");

    for (int i = 0; i < NUM_CARROS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Terminar a thread de estado
    pthread_cancel(thread_estado);
    pthread_join(thread_estado, NULL);

    return 0;
}
