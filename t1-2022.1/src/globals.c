#include "globals.h"
#include "config.h"
#include <stdlib.h>

queue_t *students_queue = NULL;
table_t *table = NULL;
buffet_t *buffets_ref = NULL;

int students_number = 0;
int number_of_buffets = 0;
int number_of_tables = 0;
int seats_per_table = 0;

int globals_get_number_of_tables() { return number_of_tables; }

void globals_set_seats_per_table(int number) {
    seats_per_table = number;
}

int globals_get_seats_per_table() {
    return seats_per_table;
}

void globals_set_number_of_tables(int quant_tables) {
    number_of_tables = quant_tables;
}

void globals_set_number_of_buffets(int number) { number_of_buffets = number; }

int globals_get_number_of_buffets() { return number_of_buffets; }

void globals_set_queue(queue_t *queue) { students_queue = queue; }

queue_t *globals_get_queue() { return students_queue; }

void globals_set_table(table_t *t) { table = t; }

table_t *globals_get_table() { return table; }

void globals_set_students(int number) { students_number = number; }

int globals_get_students() { return students_number; }

/**
 * @brief Finaliza todas as variáveis globais que ainda não foram liberadas.
 *  Se criar alguma variável global que faça uso de mallocs, lembre-se sempre de
 * usar o free dentro dessa função.
 */
void globals_finalize() { free(table); }