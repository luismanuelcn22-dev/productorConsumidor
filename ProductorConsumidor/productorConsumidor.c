/**
 * @file productorConsumidor.c
 * @brief Implementación del problema Productor-Consumidor con múltiples 
 *        productores y consumidores (2 de cada uno) usando pthreads y semáforos.
 * @author Luis Manuel Cortés Nájera
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TAM_BUFFER 5      // Capacidad del buffer circular
#define TOTAL_ITEMS 20    // Total de elementos que producirá cada productor

// Buffer circular compartido
int buffer[TAM_BUFFER];
int in = 0;  // Índice para insertar
int out = 0; // Índice para retirar

// Semáforos POSIX y Mutex
sem_t sem_vacios;    // Cuenta los espacios vacíos en el buffer
sem_t sem_llenos;    // Cuenta los espacios llenos (ítems disponibles)
pthread_mutex_t mutex_buffer; // Exclusión mutua para modificar el buffer

// Función de los Hilos Productores
void* productor(void* arg) {
    int id_productor = *((int*) arg);

    for (int i = 1; i <= TOTAL_ITEMS; i++) {
        int item = (id_productor * 100) + i; // Generar un elemento único

        // Esperar a que haya un espacio vacío en el buffer
        sem_wait(&sem_vacios);

        // Entrar a la sección crítica (modificar el buffer)
        pthread_mutex_lock(&mutex_buffer);

        buffer[in] = item;
        printf("[Productor %d] Produjo el item %d en la posicion [%d]\n", id_productor, item, in);
        in = (in + 1) % TAM_BUFFER;

        pthread_mutex_unlock(&mutex_buffer);

        // Indicar que hay un nuevo ítem disponible (llenar un espacio)
        sem_post(&sem_llenos);

        // Pequeña pausa opcional para simular tiempo de producción
        usleep(100000); 
    }

    pthread_exit(NULL);
}

// Función de los Hilos Consumidores
void* consumidor(void* arg) {
    int id_consumidor = *((int*) arg);

    // Como hay 2 productores que producen TOTAL_ITEMS cada uno (40 ítems en total),
    // cada uno de los 2 consumidores consumirá 20 ítems.
    for (int i = 0; i < TOTAL_ITEMS; i++) {

        // Esperar a que haya al menos un ítem disponible en el buffer
        sem_wait(&sem_llenos);

        // Entrar a la sección crítica (retirar del buffer)
        pthread_mutex_lock(&mutex_buffer);

        int item = buffer[out];
        printf("--> [Consumidor %d] Consumio el item %d de la posicion [%d]\n", id_consumidor, item, out);
        out = (out + 1) % TAM_BUFFER;

        pthread_mutex_unlock(&mutex_buffer);

        // Indicar que hay un espacio vacío disponible
        sem_post(&sem_vacios);

        // Pequeña pausa opcional para simular tiempo de consumo
        usleep(150000);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t prod1, prod2, cons1, cons2;
    int id1 = 1, id2 = 2;

    // Inicializar semáforos
    // sem_vacios comienza en TAM_BUFFER (todos vacíos)
    if (sem_init(&sem_vacios, 0, TAM_BUFFER) != 0) {
        perror("Error al inicializar sem_vacios");
        exit(EXIT_FAILURE);
    }
    // sem_llenos comienza en 0 (ningún ítem al inicio)
    if (sem_init(&sem_llenos, 0, 0) != 0) {
        perror("Error al inicializar sem_llenos");
        exit(EXIT_FAILURE);
    }

    // Inicializar mutex
    if (pthread_mutex_init(&mutex_buffer, NULL) != 0) {
        perror("Error al inicializar mutex");
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Iniciando simulacion de Productor-Consumidor (2 Prod, 2 Cons)...\n\n");

    // Crear hilos productores
    pthread_create(&prod1, NULL, productor, &id1);
    pthread_create(&prod2, NULL, productor, &id2);

    // Crear hilos consumidores
    pthread_create(&cons1, NULL, consumidor, &id1);
    pthread_create(&cons2, NULL, consumidor, &id2);

    // Esperar a que todos los hilos terminen su ejecución
    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    // Destruir semáforos y mutex
    sem_destroy(&sem_vacios);
    sem_destroy(&sem_llenos);
    pthread_mutex_destroy(&mutex_buffer);

    printf("\n[INFO] Simulacion finalizada exitosamente.\n");
    return 0;
}