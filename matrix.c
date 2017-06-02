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

void matrix_copy(struct matrix_t *self, const struct matrix_t *from) {
    matrix_init(self, from->nrows, from->ncols);

    for (uint32_t r = 0; r < self->nrows; r++) {
        for (uint32_t c = 0; c < self->ncols; c++) {
            self->rows[r]->elements[c] = from->rows[r]->elements[c];
        }
    }
}

void matrix_print(const struct matrix_t *self) {

    if (self->nrows == 0) {
        fprintf(stdout, "[ empty ] \n");
        return;
    }

    fprintf(stdout, "[\n");
    for (uint32_t i = 0; i < self->nrows; i++) {
        char buf[2 + 9 * self->ncols + 5];
        vector_sprint(self->rows[i], buf);
        fprintf(stdout, "  %s,\n", buf);

    }
    fprintf(stdout, "]\n");
}
