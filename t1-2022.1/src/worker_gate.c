#include "worker_gate.h"
#include "config.h"
#include "globals.h"
#include <semaphore.h>
#include <stdlib.h>

worker_gate_t self_thread;
pthread_mutex_t tables_mutex;
sem_t tables_sem;
sem_t queue_sem;

void worker_gate_look_queue() {
    sem_wait(&queue_sem);
}

void worker_gate_remove_student() {
    // Pega o array de buffets
    buffet_t *buffets = globals_get_buffets();
    queue_t *queue = globals_get_queue();
    // Retira o primeiro estudante da fila
    student_t *student = queue_remove(queue);

    // Executa um while até encontrar o buffet que está vazio
    // NOTA: Essa função só será executada quando houver certeza que existe um buffet com lugares vazios,
    // então essa situação não se caracteriza como um spin-lock
    int buffet_found = 0;
    while (1) {
        if (buffet_found)
            break;

        int number_of_buffets = globals_get_number_of_buffets();
        for (int i = 0; i < number_of_buffets; i++) {
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
    sem_post(&student->student_sem); // libera o estudante para agir
}

void worker_gate_look_buffet() {
    sem_wait(&gate_sem);
}

void *worker_gate_run(void *arg) {
    int number_students;
    number_students = *((int *)arg);

    // Pega a variável
    int number_of_buffets = globals_get_number_of_buffets();
    int number_of_tables = globals_get_number_of_tables();
    int seats_per_table = globals_get_seats_per_table();

    // Inicializa mutex para mesas e o semáforo e com o valor total de posições disponíveis
    sem_init(&tables_sem, 0, number_of_tables * seats_per_table);
    pthread_mutex_init(&tables_mutex, NULL);
    // Inicializa um semáforo para controlar quantos alunos estão na fila
    sem_init(&queue_sem, 0, 0);
    // Inicializa o semáforo dos buffets e gate
    sem_init(&gate_sem, 0, number_of_buffets * 10);

    while (number_students > 0) {
        worker_gate_look_queue();
        worker_gate_look_buffet();
        worker_gate_remove_student();

        // Atualiza a variável global de estudantes que faltam na fila
        globals_set_students(--number_students);
    }

    pthread_exit(NULL);
}

void worker_gate_init(worker_gate_t *self) {
    int number_students = globals_get_students();
    self_thread = *self;
    pthread_create(&self->thread, NULL, worker_gate_run, &number_students);
}

void worker_gate_finalize(worker_gate_t *self) {
    // Destroi o semáforo e mutex das tables
    sem_destroy(&tables_sem);
    sem_destroy(&gate_sem);
    sem_destroy(&queue_sem);
    pthread_mutex_destroy(&tables_mutex);
    // Finaliza
    pthread_join(self->thread, NULL);
    // Finaliza a fila
    queue_t *queue = globals_get_queue();
    queue_finalize(queue);
    free(self);
}

// Colocar o estudante na fila do RU
void worker_gate_insert_queue_buffet(student_t *student) {
    // Pega a fila e manda o student para ela
    queue_t *queue = globals_get_queue();
    queue_insert(queue, student);
    sem_post(&queue_sem);
}