#include "buffet.h"
#include "chef.h"
#include "config.h"
#include "globals.h"
#include <semaphore.h>
#include <stdlib.h>

// Semáforo único usado pelo worker_gate e pelos buffets
sem_t gate_sem;

void *buffet_run(void *arg) {
  buffet_t *self = (buffet_t *)arg;

  // Enquanto ainda existem alunos se servindo a thread continua viva
  while (has_students_serving(self) == FALSE) {
    _log_buffet(self);
  }

  // Destrói o semáforo de cada comida, cada comida é representada por um
  // semáforo e a quantidade de comida disponível é o valor interno do semáforo,
  // o qual é decrementado por cada aluno que consome o alimento.
  for (int i = 0; i < 5; i++) {
    sem_destroy(&self->_meal_sem[i]);

    // Destruindo array de mutexes das filas da direita e esquerda
    pthread_mutex_destroy(&self->queue_left_mutex[i]);
    pthread_mutex_destroy(&self->queue_right_mutex[i]);
  }
  pthread_exit(NULL);
}

// Verifica se uma thread buffet_t ainda está servindo alunos
int has_students_serving(buffet_t *self) {
  // Olha se ainda tem estudantes do lado de fora
  if (globals_get_students() > 0)
    return FALSE;

  // Verifica as posições do buffet específico
  for (int j = 0; j < 5; j++) {
    if (self->queue_left[j] != 0) {
      return FALSE;
    }
    if (self->queue_right[j] != 0) {
      return FALSE;
    }
  }

  return TRUE;
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
      /* Inicializa um mutex para cada posição das filas do buffet */
      pthread_mutex_init(&self[i].queue_left_mutex[j], NULL);
      pthread_mutex_init(&self[i].queue_right_mutex[j], NULL);
    }

    /* Inicializa com 0 os valores das posições no buffet */
    for (j = 0; j < 5; j++) {
      self[i].queue_left[j] = 0;
      self[i].queue_right[j] = 0;
    }

    pthread_create(&self[i].thread, NULL, buffet_run, &self[i]);
  }
}

//
void buffet_queue_insert(buffet_t *self, student_t *student) {
  // Verifica para qual lado o estudante irá na fila, essa função é chamada pelo
  // worker_gate após já ter sido garantido que existe lugar vazio nesse buffet.
  if (student->left_or_right ==
      'L') { /* Se o estudante vai para a fila esquerda */
    // Mutex que bloqueia a primeira posição do array, deixando somente um
    // estudante passar por vez
    pthread_mutex_lock(&self[student->_id_buffet].queue_left_mutex[0]);
    self[student->_id_buffet].queue_left[0] = student->_id;
  } else { /* Se o estudante vai para a fila direita */
    // Mesma lógica para a fila da direita
    pthread_mutex_lock(&self[student->_id_buffet].queue_right_mutex[0]);
    self[student->_id_buffet].queue_right[0] = student->_id;
  }

  student->_buffet_position = 0;
}

// Estudante tenta ir para a próxima posição no buffet
void buffet_next_step(buffet_t *self, student_t *student) {
  // Ponteiro para o buffet do estudante
  buffet_t *buffet_student = self + student->_id_buffet;

  /* Se estudante ainda precisa se servir de mais alguma coisa */
  if (student->_buffet_position + 1 < 5) {
    /* Está na fila esquerda? */
    if (student->left_or_right == 'L') {
      /* Caminha para a posição seguinte da fila do buffet.*/
      int position = student->_buffet_position;
      // Lock no Mutex da próxima posição para esperar o próximo estudante
      // liberá-la
      pthread_mutex_lock(&buffet_student->queue_left_mutex[position + 1]);

      buffet_student->queue_left[position] = 0;
      buffet_student->queue_left[position + 1] = student->_id;
      student->_buffet_position = student->_buffet_position + 1;

      // Unlock no Mutex da própria posição, visto que terminou de pegar a
      // comida
      pthread_mutex_unlock(&buffet_student->queue_left_mutex[position]);
    } else { /* Está na fila direita? */
      // Lógica análoga à da fila esquerda
      int position = student->_buffet_position;
      pthread_mutex_lock(&buffet_student->queue_right_mutex[position + 1]);

      buffet_student->queue_right[position] = 0;
      buffet_student->queue_right[position + 1] = student->_id;
      student->_buffet_position = student->_buffet_position + 1;

      pthread_mutex_unlock(&buffet_student->queue_right_mutex[position]);
    }
  } else {
    /* Se estudante não precisa mais de comida, então ele sai do buffet */
    if (student->left_or_right == 'L') {
      // Libera a posição em que estava, a ultima do buffet
      buffet_student->queue_left[4] = 0;
      pthread_mutex_unlock(&buffet_student->queue_left_mutex[4]);
    } else {
      // Libera a posição em que estava, a ultima do buffet
      buffet_student->queue_right[4] = 0;
      pthread_mutex_unlock(&buffet_student->queue_right_mutex[4]);
    }

    // Buffet_position == -1 -> fora do buffet
    student->_buffet_position = -1;
    // Libera um espaço no buffet para o gate mandar outro estudante
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

  // Pegando os valores de comida dos semáforos para o print
  int meals[5];
  for (int i = 0; i < 5; i++) {
    sem_getvalue(&self->_meal_sem[i], &meals[i]);
  }

  printf("\n\n\u250F\u2501 Queue left: [ %d %d %d %d %d ]\n", ids_left[0],
         ids_left[1], ids_left[2], ids_left[3], ids_left[4]);
  fflush(stdout);
  // Inteiros de comida foram alterados para semáforo para facilitar a
  // sincronização entre sa threads que acessam a comida
  printf("\u2523\u2501 BUFFET %d = [RICE: %d/40 BEANS:%d/40 PLUS:%d/40 "
         "PROTEIN:%d/40 SALAD:%d/40]\n",
         self->_id, meals[0], meals[1], meals[2], meals[3], meals[4]);
  // printf("\u2523\u2501 BUFFET %d = [RICE: %d/40 BEANS:%d/40 PLUS:%d/40
  // PROTEIN:%d/40 SALAD:%d/40]\n",
  //    self->_id, self->_meal[0], self->_meal[1], self->_meal[2],
  //    self->_meal[3], self->_meal[4]);
  fflush(stdout);
  printf("\u2517\u2501 Queue right: [ %d %d %d %d %d ]\n", ids_right[0],
         ids_right[1], ids_right[2], ids_right[3], ids_right[4]);
  fflush(stdout);
}