#include "chef.h"
#include "config.h"
#include "globals.h"
#include <semaphore.h>
#include <stdlib.h>

// Verifica se todos os estudantes já se serviram
int all_students_served() {
    queue_t *queue = globals_get_queue();
    if (!(queue == NULL)) {
        if (queue->_length > 0) {
            return 0;
        }
    }

    buffet_t *buffets = globals_get_buffets();
    int number_of_buffets = globals_get_number_of_buffets();
    for (int i = 0; i < number_of_buffets; i++) {
        for (int j = 0; j < 5; j++) {
            if (buffets[i].queue_left[j] != 0)
                return 0;
            if (buffets[i].queue_right[j] != 0)
                return 0;
        }
    }
    return 1;
}

void *chef_run() {
    msleep(5000);
    while (all_students_served() == 0) {
        chef_check_food();
    }

    pthread_exit(NULL);
}

void chef_put_food(sem_t *meal_sem) {
    for (int i = 0; i < 40; i++) {
        sem_post(meal_sem);
    }
}

void chef_check_food() {
    buffet_t *buffets = globals_get_buffets();

    int number_of_buffets = globals_get_number_of_buffets();
    for (int i = 0; i < number_of_buffets; i++) {
        for (int j = 0; j < 5; j++) {

            int food_amount;
            sem_t *meal_sem = &buffets[i]._meal_sem[j];
            sem_getvalue(meal_sem, &food_amount);

            if (food_amount == 0) {
                printf("Colocando comida no buffet %d\n", buffets[i]._id);
                chef_put_food(meal_sem);
            }
        }
    }
}

/* --------------------------------------------------------- */
/* ATENÇÃO: Não será necessário modificar as funções abaixo! */
/* --------------------------------------------------------- */

void chef_init(chef_t *self) {
    pthread_create(&self->thread, NULL, chef_run, NULL);
}

void chef_finalize(chef_t *self) {
    pthread_join(self->thread, NULL);
    free(self);
}