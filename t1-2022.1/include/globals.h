#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "buffet.h"
#include "config.h"
#include "queue.h"
#include "table.h"

/**
 * @brief Inicia uma fila (de modo global)
 *
 * @param queue
 */
extern void globals_set_queue(queue_t *queue);

/**
 * @brief Retorna uma fila (de modo global)
 *
 * @return queue_t*
 */
extern queue_t *globals_get_queue();

/**
 * @brief Insere o número de alunos (de modo global)
 *
 */
extern void globals_set_students(int number);

/**
 * @brief Retorna o número de alunos (de modo global)
 *
 * @return int
 */

extern int globals_get_students();

/**
 * @brief Inicia um array de mesas (de modo global).
 *
 * @param t
 */
extern void globals_set_table(table_t *t);

/**
 * @brief Inicia um objeto de config (de modo global).
 *
 * @param t
 */
extern void globals_set_config(config_t *t);

/**
 * @brief Inicia um objeto de config (de modo global).
 *
 * @param t
 */
extern config_t *globals_get_config();

/**
 * @brief Retorna um array de mesas (de modo global)
 *
 * @return table_t*
 */
extern table_t *globals_get_table();

/**
 * @brief Finaliza todas as variáveis globais.
 *
 */
extern void globals_finalize();

/**
 * @brief Inicia um array de buffets (de modo global)
 *
 */
extern void globals_set_buffets(buffet_t *buffets_ref);

/**
 * @brief Retorna um array de buffets (de modo global)
 *
 * @return buffet_t*
 */
extern buffet_t *globals_get_buffets();

/**
 * @brief Seta o numero de mesas (de modo global)
 *
 */
extern void globals_set_number_of_tables(int number_tables);

/**
 * @brief Retorna a quantidade de mesas (de modo global)
 *
 * @return buffet_t*
 */
extern int globals_get_number_of_tables();

/**
 * @brief Seta o numero de buffets (de modo global)
 *
 */
extern void globals_set_number_of_buffets(int number_of_buffets);

/**
 * @brief Retorna a quantidade de buffets (de modo global)
 *
 * @return buffet_t*
 */
extern int globals_get_number_of_buffets();

#endif