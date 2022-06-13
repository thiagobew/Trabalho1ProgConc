#include "worker_gate.h"
#include "config.h"
#include "globals.h"
#include <semaphore.h>
#include <stdlib.h>

worker_gate_t self_thread;
pthread_mutex_t tables_mutex;
sem_t tables_sem;
sem_t threads_sem;

void worker_gate_look_queue() {}

void worker_gate_remove_student() {
  // Pega o array de buffets
  buffet_t *buffets = globals_get_buffets();
  queue_t *queue = globals_get_queue();
  // Retira o primeiro estudante da fila
  student_t *student = queue_remove(queue);
  int buffet_found = 0;

  // Executa um while até encontrar o buffet que está vazio
  // NOTA: Essa função só será executada quando houver certeza que existe um
  // buffet com lugares vazios, então essa situação não se caracteriza como um
  // spin-lock

  while (1) {
    if (buffet_found)
      break;

    int number_of_buffets = globals_get_number_of_buffets();

    // Procura pelo primeiro buffet com a primeira posição livre (em qualquer
    // uma das duas filas)
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
  // Solta o semáforo dentor do estudante que o estava prendendo
  sem_post(&student->student_sem);
}

void worker_gate_look_buffet() { sem_wait(&gate_sem); }

void *worker_gate_run(void *arg) {
  int number_of_buffets = globals_get_number_of_buffets();
  // Inicializando barreira
  int quant_threads = number_of_buffets + 2;
  //  globals_set_barreira(quant_threads);
  //  Buffets + Chef + Worker gate threads
  sem_init(&threads_sem, 0, 0);

  int all_students_entered;
  int number_students;
  number_students = *((int *)arg);
  all_students_entered = number_students > 0 ? FALSE : TRUE;

  pthread_mutex_init(&tables_mutex, NULL);
  // Inicializa o semáforo dos buffets e gate
  sem_init(&gate_sem, 0, number_of_buffets * 10);

  for (int i = 0; i < quant_threads; i++)
    sem_post(&threads_sem);

  while (all_students_entered == FALSE) {
    worker_gate_look_queue();
    worker_gate_look_buffet();
    worker_gate_remove_student();

    number_students--;
    all_students_entered = number_students > 0 ? FALSE : TRUE;
  }

  pthread_exit(NULL);
}

void worker_gate_init(worker_gate_t *self) {
  int number_students = globals_get_students();
  self_thread = *self;
  pthread_create(&self->thread, NULL, worker_gate_run, &number_students);
}

void worker_gate_finalize(worker_gate_t *self) {
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
}