#ifndef __WORKER_GATE_H__
#define __WORKER_GATE_H__

#include "student.h"
#include <pthread.h>
#include <semaphore.h>

typedef struct worker_gate {

    pthread_t thread; // A thread do funcionário que fica na catraca.

} worker_gate_t;

extern sem_t gate_sem;
extern sem_t tables_sem;
extern pthread_mutex_t tables_mutex;

/**
 * @brief Inicia o funcionário que fica na catraca.
 *
 * @param self
 */
extern void worker_gate_init(worker_gate_t *self);

/**
 * @brief Finaliza a thread do funcionário que fica na catraca.
 *
 * @param self
 */
extern void worker_gate_finalize(worker_gate_t *self);

/**
 * @brief Thread do funcionário que fica na catraca.
 *
 * @return void*
 */
extern void *worker_gate_run();

/**
 * @brief Funcionário direciona o estudante para o buffet.
 *
 */
void worker_gate_insert_student_in_buffet(student_t *student);

/**
 * @brief Funcionário coloca o estudante na fila de fora.
 *
 */
extern void worker_gate_insert_student_in_queue(student_t *student);

/**
 * @brief Funcionário espera que tenha alguem na fila.
 *
 */
void worker_gate_look_queue();

/**
 * @brief Funcionário espera que tenha algum lugar disponível nos buffets para inserir estudante.
 *
 */
void worker_gate_look_buffet();

/**
 * @brief Funcionário remove um estudante da fila e o retorna.
 *
 */
student_t *worker_gate_remove_student();

#endif