#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "globals.h"
#include "student.h"
#include "table.h"
#include "worker_gate.h"

void *student_run(void *arg) {
    student_t *self = (student_t *)arg;
    table_t *tables = globals_get_table();

    // Criação do mutex para controlar as ações do estudante
    sem_init(&self->student_sem, 0, 0);

    worker_gate_insert_queue_buffet(self);
    msleep(5000);
    sem_wait(&self->student_sem);
    student_serve(self);
    student_seat(self, tables);
    student_leave(self, tables);

    // Destroi o mutex
    sem_destroy(&self->student_sem);
    pthread_exit(NULL);
};

void student_seat(student_t *self, table_t *table) {
    printf("Estudante %d sentando\n", self->_id);
    sem_wait(&tables_sem); // Semáforo com quantos lugares disponíveis tem
    // Mutex para somente 1 estudante procurar lugar por vez
    pthread_mutex_lock(&tables_mutex);

    config_t *configs = globals_get_config();
    // Procura e pega um lugar disponível nas mesas
    table_t *tables = globals_get_table();
    for (int i = 0; i < configs->tables; i++) {
        if (tables[i]._empty_seats > 0) {
            // salva o id da mesa em que o estudante está
            self->_id_table = i;
            tables[i]._empty_seats--;
            break;
        }
    }

    pthread_mutex_unlock(&tables_mutex);
}

void student_serve(student_t *self) {
    // printf("Estudante %d se servindo no Buffet %d\n", self->_id, self->_id_buffet);
    buffet_t *buffets = globals_get_buffets();
    buffet_t buffet = buffets[self->_id_buffet];

    while (self->_buffet_position != -1) {
        if (self->_wishes[self->_buffet_position] == 1) {
            // Pega a comida dando wait no semáforo que representa a comida
            sem_wait(&buffet._meal_sem[self->_buffet_position]);
        }
        // Anda na fila
        buffet_next_step(buffets, self);
    }
}

void student_leave(student_t *self, table_t *table) {
    // Lock para poder acessar o array de tables
    pthread_mutex_lock(&tables_mutex);

    // Libera um lugar na mesa que estava
    table[self->_id_table]._empty_seats++;

    pthread_mutex_unlock(&tables_mutex);
    sem_post(&tables_sem); // Libera o semáforo de lugares disponíveis
    printf("Estudante %d saindo\n", self->_id);
}

/* --------------------------------------------------------- */
/* ATENÇÃO: Não será necessário modificar as funções abaixo! */
/* --------------------------------------------------------- */

student_t *student_init() {
    student_t *student = malloc(sizeof(student_t));
    student->_id = rand() % 1000; // TODO
    student->_buffet_position = -1;
    int none = TRUE;
    for (int j = 0; j <= 4; j++) {
        student->_wishes[j] = _student_choice();
        if (student->_wishes[j] == 1)
            none = FALSE;
    }

    if (none == FALSE) {
        /* O estudante só deseja proteína */
        student->_wishes[3] = 1;
    }

    return student;
};

void student_finalize(student_t *self) {
    free(self);
};

pthread_t students_come_to_lunch(int number_students) {
    pthread_t lets_go;
    pthread_create(&lets_go, NULL, _all_they_come, &number_students);
    return lets_go;
}

/**
 * @brief Função (privada) que inicializa as threads dos alunos.
 *
 * @param arg
 * @return void*
 */
void *_all_they_come(void *arg) {
    int number_students = *((int *)arg);

    student_t *students[number_students];

    for (int i = 0; i < number_students; i++) {
        students[i] = student_init(); /* Estudante é iniciado, recebe um ID e escolhe o que vai comer*/
    }

    for (int i = 0; i < number_students; i++) {
        pthread_create(&students[i]->thread, NULL, student_run, students[i]); /*  Cria as threads  */
    }

    for (int i = 0; i < number_students; i++) {
        pthread_join(students[i]->thread, NULL); /*  Aguarda o término das threads   */
    }

    for (int i = 0; i < number_students; i++) {
        student_finalize(students[i]); /*  Libera a memória de cada estudante  */
    }

    pthread_exit(NULL);
}

/**
 * @brief Função que retorna as escolhas dos alunos, aleatoriamente (50% para cada opção)
 *        retornando 1 (escolhido) 0 (não escolhido). É possível que um aluno não goste de nenhuma opção
 *         de comida. Nesse caso, considere que ele ainda passa pela fila, como todos aqueles que vão comer.
 * @return int
 */
int _student_choice() {
    float prob = (float)rand() / RAND_MAX;
    return prob > 0.51 ? 1 : 0;
}