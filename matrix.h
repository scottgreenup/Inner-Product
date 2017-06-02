
#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "vector.h"

struct matrix_t {
    struct vector_t **rows;
    uint32_t ncols;
    uint32_t nrows;
};

void matrix_init(struct matrix_t *self, uint32_t nrows, uint32_t ncols);
void matrix_free(struct matrix_t *self);
void matrix_copy(struct matrix_t *self, const struct matrix_t *from);

void matrix_print(const struct matrix_t *self);

#endif
