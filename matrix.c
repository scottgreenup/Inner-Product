#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"

void matrix_init(struct matrix_t *self, uint32_t nrows, uint32_t ncols) {
    self->rows = (struct vector_t**)calloc(nrows, sizeof(struct vector_t*));

    for (uint32_t r = 0; r < nrows; r++) {
        self->rows[r] = (struct vector_t*)calloc(1, sizeof(struct vector_t));
        vector_init(self->rows[r], ncols);
    }

    self->nrows = nrows;
    self->ncols = ncols;
}

void matrix_free(struct matrix_t *self) {
    free(self->rows);
}

void matrix_print(const struct matrix_t *self) {
    for (uint32_t i = 0; i < self->nrows; i++) {
        vector_print(self->rows[i]);
    }
}
