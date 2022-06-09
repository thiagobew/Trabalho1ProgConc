#include "worker_gate.h"
#include "config.h"
#include "globals.h"
#include <semaphore.h>
#include <stdlib.h>

worker_gate_t self_thread;
pthread_mutex_t tables_mutex;
sem_t tables_sem;
config_t config;

void worker_gate_look_queue() {
    printf("Queue length: %d\n", globals_get_queue()->_length);
    if (globals_get_queue()->_length == 0)
        worker_gate_finalize(&self_thread);
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

        for (int i = 0; i < config.buffets; i++) {
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
    printf("Estudante Liberado ID: %d ID Buffet: %d", student->_id, student->_id_buffet);
    sem_post(&student->student_sem); // libera o estudante para agir
}

void worker_gate_look_buffet() {
    sem_wait(&gate_sem);
}

void *worker_gate_run(void *arg) {
    int all_students_entered;
    int number_students;

    number_students = *((int *)arg);
    all_students_entered = number_students > 0 ? FALSE : TRUE;

    pthread_mutex_init(&tables_mutex, NULL);

    // Inicializa o semáforo com o valor total de posições disponíveis
    sem_init(&tables_sem, 0, config.tables * config.seat_per_table);
    // Inicializa o semáforo dos buffets e gate
    sem_init(&gate_sem, 0, config.buffets * 10);

    msleep(5000);
    while (all_students_entered == FALSE) {
        worker_gate_look_queue();
        worker_gate_look_buffet();
        worker_gate_remove_student();
    }

    pthread_exit(NULL);
}

void worker_gate_init(worker_gate_t *self) {
    int number_students = globals_get_students();
    self_thread = *self;
    pthread_create(&self->thread, NULL, worker_gate_run, &number_students);
}

void worker_gate_finalize(worker_gate_t *self) {
    // Finaliza a fila
    queue_t *queue = globals_get_queue();
    queue_finalize(queue);
    // Destroi o semáforo e mutex das tables
    sem_destroy(&tables_sem);
    pthread_mutex_destroy(&tables_mutex);
    // Finaliza
    pthread_join(self->thread, NULL);
    free(self);
}

// Colocar o estudante na fila do RU
void worker_gate_insert_queue_buffet(student_t *student) {
    // Pega a fila e manda o student para ela
    queue_t *queue = globals_get_queue();
    queue_insert(queue, student);
}