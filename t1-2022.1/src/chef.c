#include "chef.h"
#include "config.h"
#include "globals.h"
#include "worker_gate.h"
#include <semaphore.h>
#include <stdlib.h>

pthread_mutex_t tables_mutex;
sem_t tables_sem;
sem_t threads_sem;

void *chef_run() {

  printf("Esperando\n");
  sem_wait(&threads_sem);
  printf("Passou\n");

  // Pegando variáveis globais para inicialização de semáforos
  int number_of_tables = globals_get_number_of_tables();
  int seats_per_table = globals_get_table()->_max_seats;

  // Inicializa o semáforo com o valor total de posições disponíveis
  sem_init(&tables_sem, 0, number_of_tables * seats_per_table);

  while (all_students_served() == 0) {
    chef_check_food();
  }

  // Destroi o semáforo e mutex das tables
  sem_destroy(&tables_sem);
  pthread_mutex_destroy(&tables_mutex);
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