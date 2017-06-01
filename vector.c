
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

void vector_init(struct vector_t *self, uint32_t size) {
    self->elements = (double*)calloc(size, sizeof(double));
    self->size = size;
}

void vector_free(struct vector_t *self) {
    free(self->elements);
}

void vector_print(const struct vector_t *self) {
    FILE* fd = stdout;

    fprintf(fd, "[");
    if (self->size > 0) {
        fprintf(fd, "%lg", self->elements[0]);
    }
    for (uint32_t i = 1; i < self->size; i++) {
        fprintf(fd, ", %lg", self->elements[i]);
    }
    fprintf(fd, "]\n");
}

double vector_dot_product(const struct vector_t *a, const struct vector_t *b) {
    assert(a->size == b->size);

    double sum = 0.0;
    for (uint32_t i = 0; i < a->size; i++) {
        sum += a->elements[i] * b->elements[i];
    }
    return sum;
}


