#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>

#define NUM_PISTAS 20 // 10 pistas para cada conjunto
#define NUM_CARROS 1000
#define CAPACIDADE_PONTE 10

// Estruturas para representar a pista e a ponte
typedef struct {
    int id;
    int num_carros;
    sem_t lock;
} Pista;

typedef struct {
    int num_carros;
    int trancada;
    sem_t lock;
} Ponte;

Pista* pistas;
Ponte* ponte1;
Ponte* ponte2;
int tempo_intervalo = 1000000;

// Função para inicializar as pistas e as pontes
void inicializar() {
    for (int i = 0; i < NUM_PISTAS; i++) {
        pistas[i].id = i;
        pistas[i].num_carros = 0;
        sem_init(&pistas[i].lock, 1, 1); // inicializar semáforos
    }
    ponte1->num_carros = 0;
    ponte1->trancada = 0;
    sem_init(&ponte1->lock, 1, 1);

    ponte2->num_carros = 0;
    ponte2->trancada = 0;
    sem_init(&ponte2->lock, 1, 1);
}

// Função para simular a passagem do carro pelas pistas e pelas pontes
void carro(int id) {
    // Passar pelo primeiro conjunto de pistas e pela primeira ponte
    for (int i = 0; i < NUM_PISTAS / 2; i++) {
        sem_wait(&pistas[i].lock);
        while (pistas[i].num_carros >= CAPACIDADE_PONTE) {
            sem_post(&pistas[i].lock);
            usleep(1000);
            sem_wait(&pistas[i].lock);
        }
        pistas[i].num_carros++;
        sem_post(&pistas[i].lock);
        if(i==5) {
            sem_wait(&ponte1->lock);
            ponte1->num_carros--;
            sem_post(&ponte1->lock);
        }
        usleep(tempo_intervalo); // Simula tempo para passar pela pista

        if (i == ((NUM_PISTAS / 2)/2) - 1) {
            sem_wait(&ponte1->lock);
            while (ponte1->num_carros >= CAPACIDADE_PONTE || ponte1->trancada == 1) {
                sem_post(&ponte1->lock);
                usleep(1000);
                sem_wait(&ponte1->lock);
            }
            ponte1->num_carros++;
            sem_post(&ponte1->lock);
            usleep(tempo_intervalo); // Simula tempo para passar pela ponte

            sem_wait(&pistas[i].lock);
            pistas[i].num_carros--;
            sem_post(&pistas[i].lock);
        } else {
            sem_wait(&pistas[i + 1].lock);
            while (pistas[i + 1].num_carros >= CAPACIDADE_PONTE) {
                sem_post(&pistas[i + 1].lock);
                usleep(1000);
                sem_wait(&pistas[i + 1].lock);
            }
            sem_wait(&pistas[i].lock);
            pistas[i].num_carros--;
            sem_post(&pistas[i].lock);
            sem_post(&pistas[i + 1].lock);
            usleep(tempo_intervalo);
        }
    }

    // Passar pelo segundo conjunto de pistas e pela segunda ponte
    for (int i = NUM_PISTAS / 2; i < NUM_PISTAS; i++) {
        sem_wait(&pistas[i].lock);
        while (pistas[i].num_carros >= CAPACIDADE_PONTE) {
            sem_post(&pistas[i].lock);
            usleep(1000);
            sem_wait(&pistas[i].lock);
        }
        pistas[i].num_carros++;
        sem_post(&pistas[i].lock);
        if(i==15) {
            sem_wait(&ponte2->lock);
            ponte2->num_carros--;
            sem_post(&ponte2->lock);
        }
        usleep(tempo_intervalo); // Simula tempo para passar pela pista

        if (i == (NUM_PISTAS / 2) + 4) {
            sem_wait(&ponte2->lock);
            while (ponte2->num_carros >= CAPACIDADE_PONTE || ponte2->trancada == 1) {
                sem_post(&ponte2->lock);
                usleep(1000);
                sem_wait(&ponte2->lock);
            }
            ponte2->num_carros++;
            sem_post(&ponte2->lock);
            usleep(tempo_intervalo); // Simula tempo para passar pela ponte

            sem_wait(&pistas[i].lock);
            pistas[i].num_carros--;
            sem_post(&pistas[i].lock);
        } else {
            sem_wait(&pistas[i + 1].lock);
            while (pistas[i + 1].num_carros >= CAPACIDADE_PONTE) {
                sem_post(&pistas[i + 1].lock);
                usleep(1000);
                sem_wait(&pistas[i + 1].lock);
            }
            sem_wait(&pistas[i].lock);
            pistas[i].num_carros--;
            sem_post(&pistas[i].lock);
            sem_post(&pistas[i + 1].lock);
            usleep(tempo_intervalo);
        }
    }
    exit(0);
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
            ponte1->trancada = 1;
            ponte2->trancada = 1;
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
            ponte1->trancada = 1; // Default para ponte fechada
            ponte2->trancada = 1;
            break;
    }
}

// Função para imprimir o estado do congestionamento periodicamente
void imprimir_estado() {
    while (1) {
        sleep(1); // Intervalo de tempo para imprimir o estado
        printf("------------------------------------------------------------------\n");
        for (int i = 0; i < NUM_PISTAS; i++) {
            sem_wait(&pistas[i].lock);
            printf("Pista %d - %d\n", i, pistas[i].num_carros);
            sem_post(&pistas[i].lock);
            if (i == 4) {
                sem_wait(&ponte1->lock);
                printf("Ponte 01 - %d\n", ponte1->num_carros);
                sem_post(&ponte1->lock);
            }
            if (i == 14) {
                sem_wait(&ponte2->lock);
                printf("Ponte 02 - %d\n", ponte2->num_carros);
                sem_post(&ponte2->lock);
            }
        }
    }
}

int main() {
    int shmid_pistas, shmid_ponte1, shmid_ponte2;

    // Alocar memória compartilhada para as pistas e pontes
    shmid_pistas = shmget(IPC_PRIVATE, NUM_PISTAS * sizeof(Pista), IPC_CREAT | 0666);
    shmid_ponte1 = shmget(IPC_PRIVATE, sizeof(Ponte), IPC_CREAT | 0666);
    shmid_ponte2 = shmget(IPC_PRIVATE, sizeof(Ponte), IPC_CREAT | 0666);

    if (shmid_pistas == -1 || shmid_ponte1 == -1 || shmid_ponte2 == -1) {
        perror("Erro ao criar memória compartilhada");
        exit(1);
    }

    pistas = (Pista*)shmat(shmid_pistas, NULL, 0);
    ponte1 = (Ponte*)shmat(shmid_ponte1, NULL, 0);
    ponte2 = (Ponte*)shmat(shmid_ponte2, NULL, 0);

    if (pistas == (void*)-1 || ponte1 == (void*)-1 || ponte2 == (void*)-1) {
        perror("Erro ao associar memória compartilhada");
        exit(1);
    }

    inicializar();
    menu();

    // Criar um processo para imprimir o estado do congestionamento
    if (fork() == 0) {
        imprimir_estado();
        exit(0);
    }

    printf("-> Gerando carros!\n");
    for (int i = 0; i < NUM_CARROS; i++) {
        if (fork() == 0) {
            carro(i);
        }
        usleep(tempo_intervalo); // Intervalo entre a criação de cada carro
    }
    printf("-> Todos carros gerados!\n");

    // Aguardar todos os processos filhos terminarem
    for (int i = 0; i < NUM_CARROS; i++) {
        wait(NULL);
    }

    // Desassociar e desalocar memória compartilhada
    shmdt(pistas);
    shmdt(ponte1);
    shmdt(ponte2);
    shmctl(shmid_pistas, IPC_RMID, NULL);
    shmctl(shmid_ponte1, IPC_RMID, NULL);
    shmctl(shmid_ponte2, IPC_RMID, NULL);

    return 0;
}
