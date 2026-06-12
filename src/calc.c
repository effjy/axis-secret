#include "calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

static void skip_whitespace(const char **p) {
    while (**p && isspace((unsigned char)**p)) {
        (*p)++;
    }
}

static double parse_expression(const char **p, int *success, char *err_msg, size_t err_msg_len);
static double parse_term(const char **p, int *success, char *err_msg, size_t err_msg_len);
static double parse_factor(const char **p, int *success, char *err_msg, size_t err_msg_len);
static double parse_primary(const char **p, int *success, char *err_msg, size_t err_msg_len);

static double parse_expression(const char **p, int *success, char *err_msg, size_t err_msg_len) {
    if (!*success) return 0.0;
    
    double val = parse_term(p, success, err_msg, err_msg_len);
    if (!*success) return 0.0;
    
    while (1) {
        skip_whitespace(p);
        char op = **p;
        if (op == '+' || op == '-') {
            (*p)++;
            double next_val = parse_term(p, success, err_msg, err_msg_len);
            if (!*success) return 0.0;
            if (op == '+') {
                val += next_val;
            } else {
                val -= next_val;
            }
        } else {
            break;
        }
    }
    return val;
}

static double parse_term(const char **p, int *success, char *err_msg, size_t err_msg_len) {
    if (!*success) return 0.0;
    
    double val = parse_factor(p, success, err_msg, err_msg_len);
    if (!*success) return 0.0;
    
    while (1) {
        skip_whitespace(p);
        char op = **p;
        if (op == '*' || op == '/') {
            (*p)++;
            double next_val = parse_factor(p, success, err_msg, err_msg_len);
            if (!*success) return 0.0;
            if (op == '*') {
                val *= next_val;
            } else {
                if (next_val == 0.0) {
                    *success = 0;
                    snprintf(err_msg, err_msg_len, "Division by zero");
                    return 0.0;
                }
                val /= next_val;
            }
        } else {
            break;
        }
    }
    return val;
}

static double parse_factor(const char **p, int *success, char *err_msg, size_t err_msg_len) {
    if (!*success) return 0.0;
    
    double val = parse_primary(p, success, err_msg, err_msg_len);
    if (!*success) return 0.0;
    
    skip_whitespace(p);
    if (**p == '%') {
        val /= 100.0;
        (*p)++;
    }
    
    return val;
}

static double parse_primary(const char **p, int *success, char *err_msg, size_t err_msg_len) {
    if (!*success) return 0.0;
    
    skip_whitespace(p);
    char c = **p;
    
    if (c == '-') {
        (*p)++;
        return -parse_primary(p, success, err_msg, err_msg_len);
    }
    if (c == '+') {
        (*p)++;
        return parse_primary(p, success, err_msg, err_msg_len);
    }
    
    if (c == '(') {
        (*p)++;
        double val = parse_expression(p, success, err_msg, err_msg_len);
        if (!*success) return 0.0;
        
        skip_whitespace(p);
        if (**p != ')') {
            *success = 0;
            snprintf(err_msg, err_msg_len, "Missing closing parenthesis");
            return 0.0;
        }
        (*p)++;
        return val;
    }
    
    if (isdigit((unsigned char)c) || c == '.') {
        char *endptr;
        double val = strtod(*p, &endptr);
        if (*p == endptr) {
            *success = 0;
            snprintf(err_msg, err_msg_len, "Invalid number format");
            return 0.0;
        }
        *p = endptr;
        return val;
    }
    
    *success = 0;
    if (c == '\0') {
        snprintf(err_msg, err_msg_len, "Unexpected end of expression");
    } else {
        snprintf(err_msg, err_msg_len, "Unexpected character '%c'", c);
    }
    return 0.0;
}

double evaluate_expression(const char *expr, int *success, char *err_msg, size_t err_msg_len) {
    const char *p = expr;
    *success = 1;
    err_msg[0] = '\0';
    
    skip_whitespace(&p);
    if (*p == '\0') {
        *success = 0;
        snprintf(err_msg, err_msg_len, "Empty expression");
        return 0.0;
    }
    
    double val = parse_expression(&p, success, err_msg, err_msg_len);
    if (!*success) return 0.0;
    
    skip_whitespace(&p);
    if (*p != '\0') {
        *success = 0;
        snprintf(err_msg, err_msg_len, "Unexpected character '%c' after expression", *p);
        return 0.0;
    }
    
    return val;
}
