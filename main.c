
#include <assert.h>
#include <argp.h>
#include <execinfo.h>
#include <mpi.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "matrix.h"
#include "vector.h"

// name, key, arg name, falgs, doc, group
static struct argp_option options[] = {
    {"columns",   'm', "columns",   0, "The size of each vector."},
    {"rows",      'n', "rows",      0, "The number of vectors."},
    {"processes", 'p', "processes", 0, "The number of workers."},
    {"input",     'i', "filename",  0,
        "Load matrix from filename. Requires --columns and --rows. "
        "If missing, a random matrix will be created and echoed." },
    {"verbose",   'v', 0,           0, "Verbose mode."},
    {0}
};

struct arguments {
    uint32_t cols;
    uint32_t rows;
    uint32_t workers; // A.K.A. processes
    char* input_filename;
    bool verbose;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *args = state->input;
    switch (key) {
        case 'm': args->cols = atoi(arg); break;
        case 'n': args->rows = atoi(arg); break;
        case 'p': args->workers = atoi(arg); break;
        case 'i': args->input_filename = arg; break;
        case 'v': args->verbose = true; break;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, "", "", 0, 0, 0};

void print_and_exit(uint32_t errnum, const char *message) {
    fprintf(stderr, "Error %d: %s\n", errnum, message);
    exit(errnum);
}

struct matrix_t* load_matrix(const struct arguments *args) {

    return 0x0;
}

double generate_random() {
    double mul = (rand() % 2 == 0) ? -1.0 : 1.0;
    mul = 1.0;
    double range = 16;
    range = 9;
    return (((float)rand()) / ((float)(RAND_MAX))) * mul * range;
}

void random_matrix(struct matrix_t* matrix, const struct arguments *args) {
    matrix_init(matrix, args->rows, args->cols);

    for (uint32_t r = 0; r < args->rows; r++) {
        for (uint32_t c = 0; c < args->cols; c++) {
            matrix->rows[r]->elements[c] = (int)generate_random();
        }
    }
}

void serial(const struct matrix_t* matrix) {
    for (uint32_t i = 0; i < matrix->nrows; i++) {
        for (uint32_t j = i+1; j < matrix->nrows; j++) {
            fprintf(stderr, "%lg ", vector_dot_product(matrix->rows[i], matrix->rows[j]));
        }
        fprintf(stderr, "\n");
    }
}

void parallel(const struct matrix_t* matrix) {
    sleep(5);
}

long get_time() {
    return clock();
}

double timeval_diff(struct timeval ta, struct timeval tb) {
    return ((double)(tb.tv_usec - ta.tv_usec) / CLOCKS_PER_SEC) +
        (double)(tb.tv_sec - ta.tv_sec);
}

int main(int argc, char **argv) {
    srand(time(NULL));

    struct arguments args;
    args.cols = 0;
    args.rows = 0;
    args.workers = 0;
    args.input_filename = "-";
    args.verbose = false;
    argp_parse(&argp, argc, argv, 0, 0, &args);

    assert(args.cols > 1);
    assert(args.rows > 1);
    assert(args.workers > 1);
    assert(args.rows % args.workers == 0);

    struct matrix_t matrix;
    if (strcmp(args.input_filename, "-") == 0) {
        random_matrix(&matrix, &args);
    } else {
        load_matrix(&args);
    }

    struct matrix_t copy_serial;
    matrix_copy(&copy_serial, &matrix);

    struct matrix_t copy_parallel;
    matrix_copy(&copy_parallel, &matrix);

    struct timeval ta, tb, tc;
    gettimeofday(&ta, NULL);

    serial(&copy_serial);
    gettimeofday(&tb, NULL);

    parallel(&copy_parallel);
    gettimeofday(&tc, NULL);

    double serial_dt = timeval_diff(ta, tb);
    double parallel_dt = timeval_diff(tb, tc);

    printf("serial time = %lf\n", serial_dt);
    printf("parallel time = %lf\n", parallel_dt);

    //struct vector_t vector;
    //vector_init(&vector, 10);
    //vector.elements[0] = 10.0;
    //vector.elements[1] = 5.3;
    //vector_print(&vector);
    //vector_free(&vector);

}

