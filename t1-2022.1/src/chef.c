#include "chef.h"
#include "config.h"
#include "globals.h"
#include <semaphore.h>
#include <stdlib.h>

void *chef_run() {
    wait_buffets_initialize(); // Espera ocupada
    while (all_students_served() == FALSE) {
        chef_check_food();
    }

    pthread_exit(NULL);
}

// Verifica se algum alimento acabou e chama a função para repor
void chef_check_food() {
    buffet_t *buffets = globals_get_buffets();
    int number_of_buffets = globals_get_number_of_buffets();

    for (int i = 0; i < number_of_buffets; i++) {
        for (int j = 0; j < 5; j++) {
            int food_amount;
            sem_t *meal_sem = &buffets[i]._meal_sem[j];
            sem_getvalue(meal_sem, &food_amount);

            if (food_amount == 0) {
                chef_put_food(meal_sem);
            }
        }
    }
}

// Espera ocupada até os buffets serem inicializados
void wait_buffets_initialize() {
    /* Motivo da espera ocupada >> */
    /*
        Visto que nossa implementação da função all_students_served precisava analisar todos
        os buffets e que a inicialização deles e do chef partem diretamente da main, não conseguimos
        encontrar nenhuma outra solução que não necessitasse alterar a main, tentamos colocar um semáforo
        para forçar uma sincronização com o chef e as threads de buffet, mas como essas estruturas de
        sincronização precisam ser iniciadas a sincronização não ocorria, exemplo: Se o init ficasse no chef
        os buffets davam um sem_post em um semáforo ainda não iniciado e o chef nunca saía do sem_wait, visto que
        é uma sincronização que apenas espera as outras threads serem iniciadas no começo do programa não é muito custoso
    */

    buffet_t *buffets = globals_get_buffets();
    while (buffets == NULL)
        buffets = globals_get_buffets();
}

// Repõe a comida, dando 40 post no semáforo que representa a comida
void chef_put_food(sem_t *meal_sem) {
    // O sem_post ocorre 40 vezes pois o máximo de comida que cabe é 40 unidades
    for (int i = 0; i < 40; i++) {
        sem_post(meal_sem);
    }
}

// Verifica se todos os estudantes já se serviram
int all_students_served() {
    // Vê se ainda existem alunos na fila externa
    if (globals_get_students() > 0)
        return FALSE;

    buffet_t *buffets = globals_get_buffets();
    int number_of_buffets = globals_get_number_of_buffets();
    // Passa por todas as posições de todos os buffets verificando se existe alunos lá
    for (int i = 0; i < number_of_buffets; i++) {
        for (int j = 0; j < 5; j++) {
            if (buffets[i].queue_left[j] != 0)
                return FALSE;
            if (buffets[i].queue_right[j] != 0)
                return FALSE;
        }
    }
    return TRUE;
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