#include "globals.h"
#include "config.h"
#include <stdlib.h>

queue_t *students_queue = NULL;
table_t *table = NULL;
buffet_t *buffets_ref = NULL;
sem_t buffets_start_sem;

int students_number = 0;
int number_of_buffets = 0;
int number_of_tables = 0;
int seats_per_table = 0;

int globals_get_number_of_tables() {
    return number_of_tables;
}

void globals_set_seats_per_table(int number) {
    seats_per_table = number;
}

int globals_get_seats_per_table() {
    return seats_per_table;
}

void globals_set_number_of_tables(int quant_tables) {
    number_of_tables = quant_tables;
}

void globals_set_number_of_buffets(int number) {
    number_of_buffets = number;
}

int globals_get_number_of_buffets() {
    return number_of_buffets;
}

void globals_set_queue(queue_t *queue) {
    students_queue = queue;
}

queue_t *globals_get_queue() {
    return students_queue;
}

void globals_set_table(table_t *t) {
    table = t;
}

table_t *globals_get_table() {
    return table;
}

void globals_set_students(int number) {
    students_number = number;
}

int globals_get_students() {
    return students_number;
}

void globals_set_buffets(buffet_t *buffets) {
    buffets_ref = buffets;
}

buffet_t *globals_get_buffets() {
    return buffets_ref;
}

void wait_structures_to_start() {
    while (table == NULL || students_queue == NULL || buffets_ref == NULL) {
    }
    return;
}

// Verifica se todos os estudantes já se serviram
int all_students_served() {
    queue_t *queue = globals_get_queue();
    if (!(queue == NULL)) {
        if (queue->_length > 0) {
            return 0;
        }
    }

    buffet_t *buffets = globals_get_buffets();
    if (buffets == NULL)
        return 0;
    int number_of_buffets = globals_get_number_of_buffets();
    for (int i = 0; i < number_of_buffets; i++) {
        for (int j = 0; j < 5; j++) {
            if (buffets == NULL)
                continue;
            if (buffets[i].queue_left[j] != 0)
                return 0;
            if (buffets[i].queue_right[j] != 0)
                return 0;
        }
    }
    return 1;
}

/**
 * @brief Finaliza todas as variáveis globais que ainda não foram liberadas.
 *  Se criar alguma variável global que faça uso de mallocs, lembre-se sempre de usar o free dentro
 * dessa função.
 */
void globals_finalize() {
    sem_destroy(&buffets_start_sem);
    free(table);
}