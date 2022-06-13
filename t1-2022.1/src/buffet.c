#include "buffet.h"
#include "config.h"
#include "globals.h"
#include "worker_gate.h"
#include <semaphore.h>
#include <stdlib.h>

// Semáforo único usado pelo worker_gate e pelos buffets
sem_t gate_sem;
sem_t threads_sem;

void *buffet_run(void *arg) {
    buffet_t *self = (buffet_t *)arg;

    int *barreira = globals_get_barreira();
    barreira[self->_id] = 1;
    sem_wait(&threads_sem);

    /*  O buffet funciona enquanto houver alunos na fila externa ou se alimentando */
    while (all_students_served() == 0) {
        /* Cada buffet possui: Arroz, Feijão, Acompanhamento, Proteína e Salada */
        /* Máximo de porções por bacia (40 unidades). */
        _log_buffet(self);

        msleep(1000); /* Pode retirar este sleep quando implementar a solução! */
    }

    // Destrói o semáforo de cada comida
    for (int i = 0; i < 5; i++) {
        sem_destroy(&self->_meal_sem[i]);
    };

    pthread_exit(NULL);
}

void buffet_init(buffet_t *self, int number_of_buffets) {
    int i = 0, j = 0;
    globals_set_number_of_buffets(number_of_buffets);

    for (i = 0; i < number_of_buffets; i++) {
        /*A fila possui um ID*/
        self[i]._id = i;

        /* Inicia com 40 unidades de comida em cada bacia */
        for (j = 0; j < 5; j++) {
            sem_init(&self[i]._meal_sem[j], 0, 40);
            /* Cria um mutex para cada posição das filas do buffet */
            pthread_mutex_init(&self[i].queue_left_mutex[j], NULL);
            pthread_mutex_init(&self[i].queue_right_mutex[j], NULL);
        }

        for (j = 0; j < 5; j++) {
            /* A fila esquerda do buffet possui cinco posições. */
            self[i].queue_left[j] = 0;
            /* A fila esquerda do buffet possui cinco posições. */
            self[i].queue_right[j] = 0;
        }

        pthread_create(&self[i].thread, NULL, buffet_run, &self[i]);
    }
}

int buffet_queue_insert(buffet_t *self, student_t *student) {
    /* Se o estudante vai para a fila esquerda */
    if (student->left_or_right == 'L') {
        /* Verifica se a primeira posição está vaga */
        if (!self[student->_id_buffet].queue_left[0]) {
            pthread_mutex_lock(&self[student->_id_buffet].queue_left_mutex[0]);
            self[student->_id_buffet].queue_left[0] = student->_id;
            student->_buffet_position = 0;
            return TRUE;
        }
        return FALSE;
    } else { /* Se o estudante vai para a fila direita */
        if (!self[student->_id_buffet].queue_right[0]) {
            /* Verifica se a primeira posição está vaga */
            pthread_mutex_lock(&self[student->_id_buffet].queue_right_mutex[0]);
            self[student->_id_buffet].queue_right[0] = student->_id;
            student->_buffet_position = 0;
            return TRUE;
        }
        return FALSE;
    }
}

void buffet_next_step(buffet_t *self, student_t *student) {
    // Ponteiro para o buffet do estudante
    buffet_t *buffet_student = self + student->_id_buffet;

    /* Se estudante ainda precisa se servir de mais alguma coisa... */
    if (student->_buffet_position + 1 < 5) {
        // printf("Estudante %d avançando na fila, pos: %d\n", student->_id, student->_buffet_position);
        /* Está na fila esquerda? */
        if (student->left_or_right == 'L') {
            /* Caminha para a posição seguinte da fila do buffet.*/
            int position = student->_buffet_position;
            // Lock no Mutex da próxima posição para esperar o próximo estudante
            pthread_mutex_lock(&buffet_student->queue_left_mutex[position + 1]);

            // int *ids_left = buffet_student->queue_left;
            // printf("\n\n\u250F\u2501 Queue left Student: [ %d %d %d %d %d ]\n", ids_left[0], ids_left[1], ids_left[2], ids_left[3], ids_left[4]);
            buffet_student->queue_left[position] = 0;
            buffet_student->queue_left[position + 1] = student->_id;
            student->_buffet_position = student->_buffet_position + 1;

            // Unlock no Mutex da própria posição, visto que terminou de pegar a comida
            pthread_mutex_unlock(&buffet_student->queue_left_mutex[position]);
        } else /* Está na fila direita? */
        {      /* Caminha para a posição seguinte da fila do buffet.*/
            int position = student->_buffet_position;
            pthread_mutex_lock(&buffet_student->queue_right_mutex[position + 1]);

            buffet_student->queue_right[position] = 0;
            buffet_student->queue_right[position + 1] = student->_id;
            student->_buffet_position = student->_buffet_position + 1;

            pthread_mutex_unlock(&buffet_student->queue_right_mutex[position]);
        }
    } else {

        // printf("Estudante %d terminou de servir!\n", student->_id);

        /* Se estudante não precisa mais de comida, então ele sai do buffet */
        if (student->left_or_right == 'L') {
            buffet_student->queue_left[4] = 0;

            pthread_mutex_unlock(&buffet_student->queue_left_mutex[4]);
        } else {
            buffet_student->queue_right[4] = 0;

            pthread_mutex_unlock(&buffet_student->queue_right_mutex[4]);
        }

        student->_buffet_position = -1;
        // Libera um espaço
        sem_post(&gate_sem);
    }
}

/* --------------------------------------------------------- */
/* ATENÇÃO: Não será necessário modificar as funções abaixo! */
/* --------------------------------------------------------- */

void buffet_finalize(buffet_t *self, int number_of_buffets) {
    /* Espera as threads se encerrarem...*/
    for (int i = 0; i < number_of_buffets; i++) {
        pthread_join(self[i].thread, NULL);
    }

    /*Libera a memória.*/
    free(self);
}

void _log_buffet(buffet_t *self) {
    /* Prints do buffet */
    int *ids_left = self->queue_left;
    int *ids_right = self->queue_right;

    int meal0;
    int meal1;
    int meal2;
    int meal3;
    int meal4;
    sem_getvalue(&self->_meal_sem[0], &meal0);
    sem_getvalue(&self->_meal_sem[1], &meal1);
    sem_getvalue(&self->_meal_sem[2], &meal2);
    sem_getvalue(&self->_meal_sem[3], &meal3);
    sem_getvalue(&self->_meal_sem[4], &meal4);

    printf("\n\n\u250F\u2501 Queue left: [ %d %d %d %d %d ]\n", ids_left[0], ids_left[1], ids_left[2], ids_left[3], ids_left[4]);
    fflush(stdout);
    // Inteiros de comida foram alterados para semáforo para facilitar a sincronização entre as threads que acessam a comida
    printf("\u2523\u2501 BUFFET %d = [RICE: %d/40 BEANS:%d/40 PLUS:%d/40 PROTEIN:%d/40 SALAD:%d/40]\n",
           self->_id, meal0, meal1, meal2, meal3, meal4);
    // printf("\u2523\u2501 BUFFET %d = [RICE: %d/40 BEANS:%d/40 PLUS:%d/40 PROTEIN:%d/40 SALAD:%d/40]\n",
    //    self->_id, self->_meal[0], self->_meal[1], self->_meal[2], self->_meal[3], self->_meal[4]);
    fflush(stdout);
    printf("\u2517\u2501 Queue right: [ %d %d %d %d %d ]\n", ids_right[0], ids_right[1], ids_right[2], ids_right[3], ids_right[4]);
    fflush(stdout);
}