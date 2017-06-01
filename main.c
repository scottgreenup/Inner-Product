
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
#include <unistd.h>

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

void load_matrix(const struct arguments *args) {

}

void random_matrix(const struct arguments *args) {

}

int main(int argc, char **argv) {
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

    if (strcmp(args.input_filename, "-") == 0) {
        random_matrix(&args);
    } else {
        load_matrix(&args);
    }

    struct vector_t vector;
    vector_init(&vector, 10);
    vector.elements[0] = 10.0;
    vector.elements[1] = 5.3;
    vector_print(&vector);
    vector_free(&vector);

}

