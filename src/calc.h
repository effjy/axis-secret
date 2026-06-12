#ifndef CALC_H
#define CALC_H

#include <stddef.h>

/**
 * Evaluates a mathematical expression string.
 * @param expr The expression to evaluate (e.g., "3 + 5 * (10 - 2)")
 * @param success Output parameter indicating if evaluation was successful (1) or failed (0)
 * @param err_msg Output buffer for error message if success is 0
 * @param err_msg_len Size of the error message buffer
 * @return The result of the evaluation, or 0.0 on error
 */
double evaluate_expression(const char *expr, int *success, char *err_msg, size_t err_msg_len);

#endif /* CALC_H */
