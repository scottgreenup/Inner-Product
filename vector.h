#ifndef _VECTOR_H_
#define _VECTOR_H_

struct vector_t {
    double *elements;
    uint32_t size;
};

void vector_init(struct vector_t *self, uint32_t size);
void vector_free(struct vector_t *self);
void vector_print(const struct vector_t *self);
void vector_sprint(const struct vector_t *self, char *buf);

double vector_dot_product(const struct vector_t *a, const struct vector_t *b);

#endif
