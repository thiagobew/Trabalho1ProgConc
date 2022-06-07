#include "worker_gate.h"
#include "config.h"
#include "globals.h"
#include <semaphore.h>
#include <stdlib.h>

worker_gate_t self;
extern sem_t gate_sem;
extern int quant_buffets;

void worker_gate_look_queue() {
    if (globals_get_queue()->_length == 0)
        worker_gate_finalize(&self);
}

void worker_gate_remove_student() {
    // Pega o array de buffets
    buffet_t *buffets = globals_get_buffets();
    queue_t *queue = globals_get_queue();
    // Retira o primeiro estudante da fila
    student_t *student = queue_remove(queue);
    int buffet_found = 0;

    // Executa um while até encontrar o buffet que está vazio
    // NOTA: Essa função só será executada quando houver certeza que existe um buffet com lugares vazios,
    // então essa situação não se caracteriza como um spin-lock
    while (1) {
        if (buffet_found)
            break;

        for (int i = 0; i < quant_buffets; i++) {
            if (buffets[i].queue_left[0] == 0) {
                student->left_or_right = 'L';
                student->_id_buffet = i;
                buffet_found = 1;
                break;
            }

            if (buffets[i].queue_right[0] == 0) {
                student->left_or_right = 'R';
                student->_id_buffet = i;
                buffet_found = 1;
                break;
            }
        }
    }

    // Insere o estudante no buffet encontrado
    buffet_queue_insert(buffets, student);
    pthread_mutex_unlock(&student->mutex); // libera o estudante para agir
}

void worker_gate_look_buffet() {
    sem_wait(&gate_sem);
}

void *worker_gate_run(void *arg) {
    int all_students_entered;
    int number_students;

    number_students = *((int *)arg);
    all_students_entered = number_students > 0 ? FALSE : TRUE;

    while (all_students_entered == FALSE) {
        worker_gate_look_queue();
        worker_gate_look_buffet();
        worker_gate_remove_student();
    }

    pthread_exit(NULL);
}

void worker_gate_init(worker_gate_t *self) {
    int number_students = globals_get_students();
    worker_gate_t self = *self;
    pthread_create(&self->thread, NULL, worker_gate_run, &number_students);
}

void worker_gate_finalize(worker_gate_t *self) {
    // Finaliza a fila
    queue_t *queue = globals_get_queue();
    queue_finalize(queue);
    pthread_join(self->thread, NULL);
    free(self);
}

// Colocar o estudante na fila do RU
void worker_gate_insert_queue_buffet(student_t *student) {
    // Pega a fila e manda o student para ela
    queue_t *queue = globals_get_queue();
    queue_insert(queue, student);
    // Dá um lock para esperar o estudante sair da fila
    pthread_mutex_lock(&student->mutex);
}