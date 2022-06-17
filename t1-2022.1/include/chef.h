#ifndef __chef_H__
#define __chef_H__

#include <pthread.h>
#include <semaphore.h>

typedef struct chef {
    pthread_t thread;
} chef_t;

/**
 * @brief Inicializa a thread do chef.
 *
 * @param  none
 */
extern void chef_init(chef_t *self);

/**
 * @brief Finaliza a thread do chef.
 *
 * @param self
 */
extern void chef_finalize(chef_t *self);

/**
 * @brief Função de thread do chef.
 *
 * @param  none
 */
extern void *chef_run();

/**
 * @brief Chefe coloca comida no buffet.
 *
 * @param  none
 */
extern void chef_put_food();

/**
 * @brief Chefe checa comida no buffet.
 *
 * @param  none
 */
extern void chef_check_food();

/**
 * @brief Retorna TRUE se os buffets e a fila estiverem vazios, e FALSE do contrário.
 *
 * @param  none
 */
int all_students_served();

/**
 * @brief Checa e espera as variáveis de buffet_t serem setadas.
 *
 * @param  none
 */
void wait_buffets_initialize();

#endif