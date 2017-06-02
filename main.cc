
#include <argp.h>
#include <execinfo.h>
#include <mpi.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "matrix.h"
#include "timer.h"
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
    std::string input_filename;
    bool verbose;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *args = (struct arguments*)state->input;
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
    std::cerr << "Error " << errnum << ": " << message << std::endl;
    exit(errnum);
}

double generate_random() {
    double mul = (rand() % 2 == 0) ? -1.0 : 1.0;
    mul = 1.0;
    double range = 16;
    range = 9;
    return (((float)rand()) / ((float)(RAND_MAX))) * mul * range;
}

void random_matrix(Matrix& matrix, const struct arguments *args) {
    for (uint32_t r = 0; r < args->rows; r++) {
        for (uint32_t c = 0; c < args->cols; c++) {
            matrix[r][c] = static_cast<uint32_t>(generate_random());
        }
    }
}

bool doubleEqual(double a, double b, double delta=0.0001) {
    double diff = a - b;
    diff = (diff < 0 ? -diff : diff);
    return (diff < delta);
}

bool serial(const Matrix& matrix, const std::vector<double>& results) {
    std::cout << std::endl;
    std::cout << "Performing serial check of the values." << std::endl;

    auto it = results.cbegin();

    for (uint32_t i = 0; i < matrix.rows(); i++) {
        for (uint32_t j = i+1; j < matrix.rows(); j++) {
            double result = matrix[i].Dot(matrix[j]);
            std::cout << "(" << i << ", " << j << "): " << result << std::endl;

            if (it == results.cend() || !doubleEqual(result, *it)) {
                std::cout << "Failed" << std::endl;
                return false;
            }

            ++it;
        }
    }
}

class WorkPair {
public:
    uint32_t a;
    uint32_t b;
    WorkPair(uint32_t a, uint32_t b) : a(a), b(b) { }
};

std::vector<double> parallel(const Matrix& matrix, const struct arguments* args) {
    std::vector<double> results;

    // Divide up the amount of work per worker. The divvy up is done in a round-
    // robin fashion. Here is an example with 4 workers in terms of n
    //
    //      worker       pair
    //      0        <-- 0, n-1
    //      1        <-- 1, n-2
    //      2        <-- 2, n-3
    //      3        <-- 3, n-4
    //      0        <-- 4, n-5
    //      ...
    //
    // This is stored in pairs[][]; where pairs[x] will be given to worker x.

    std::map<uint32_t, std::vector<WorkPair>> work;

    // Go through all the pairs and assign them to a worker.
    for (uint32_t i = 0; i < (args->rows / 2); i++) {

        uint32_t curr = i % args->workers;

        for (uint32_t j = i; j < args->rows; j++) {
            work[curr].push_back(WorkPair(i, j));
        }

        uint32_t opposite = args->rows - 1 - i;

        for (uint32_t j = opposite; j < args->rows; j++) {
            work[curr].push_back(WorkPair(opposite, j));
        }
    }

    if (args->rows % 2 == 1) {
        uint32_t i = (args->rows / 2);
        uint32_t curr = i % args->workers;

        for (uint32_t j = i; j < args->rows; j++) {
            work[curr].push_back(WorkPair(i, j));
        }
    }

    for (auto& workv : work) {
        std::cout << "Worker " << workv.first << " given " << workv.second.size() << " jobs." <<  std::endl;

        /*
        for (auto& pair : workv.second) {
            std::cout << pair.a << ", " << pair.b << std::endl;
        }
        std::cout << std::endl;
        */

    }

    return results;
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

    Matrix matrix(args.rows, args.cols);
    random_matrix(matrix, &args);

    std::cout << matrix.ToString() << std::endl;

    Matrix matrix_serial(matrix);
    Matrix matrix_parallel(matrix);

    Timer::Start();

    std::vector<double> results = parallel(matrix_parallel, &args);
    Timer::DeltaRemember("parallel(...)");

    serial(matrix_serial, results);
    Timer::DeltaRemember("  serial(...)");

    Timer::PrintDelta();

    //struct vector_t vector;
    //vector_init(&vector, 10);
    //vector.elements[0] = 10.0;
    //vector.elements[1] = 5.3;
    //vector_print(&vector);
    //vector_free(&vector);

}

