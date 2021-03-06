#ifndef __buffet_H__
#define __buffet_H__

#include <pthread.h>
#include <semaphore.h>

#include "queue.h"

typedef struct buffet {
    int _id;
    sem_t _meal_sem[5]; // Semáforo para fazer o controle da comida (inicializado com 40)

    int queue_left[5];
    int queue_right[5];

    pthread_t thread; /* Thread do buffet   */
    /* Mutexes de cada posição no buffet */
    pthread_mutex_t queue_left_mutex[5];
    pthread_mutex_t queue_right_mutex[5];
} buffet_t;

/**
 * @brief Thread do buffet.
 *
 * @return void*
 */
extern void *buffet_run();

/**
 * @brief Inicia o buffet
 *
 */
extern void buffet_init(buffet_t *self, int number_of_buffets);

/**
 * @brief Encerra o buffet
 *
 */
extern void buffet_finalize(buffet_t *self, int number_of_buffets);

/**
 * @brief Vai para a próxima posição da fila do buffet
 *
 * @param self
 * @param student
 */
extern void buffet_next_step(buffet_t *self, student_t *student);

/**
 * @brief Atualiza os dados do estudante e do buffet em relação ao buffet-destino.
 *
 */
extern void buffet_queue_insert(buffet_t *self, student_t *student);

/**
 * @brief Retorna TRUE quando a fila de fora está vazia e o respectivo buffet já está vazio.
 *        Retorna FALSE, caso contrário.
 *
 */
int has_students_serving(buffet_t *self);

/**
 * @brief Referências para funções privadas ao arquivo.
 *
 * @param self
 */
void _log_buffet(buffet_t *self);

#endif