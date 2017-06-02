
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

const int MPI_DEFAULT_TAG = 1;
const int MPI_MASTER_ID = 0;

// name, key, arg name, falgs, doc, group
static struct argp_option options[] = {
    {"columns",   'm', "columns",   0, "The size of each vector."},
    {"rows",      'n', "rows",      0, "The number of vectors."},
    {"verbose",   'v', 0,           0, "Verbose mode."},
    {0}
};

struct arguments {
    uint32_t cols;
    uint32_t rows;
    uint32_t workers;
    bool verbose;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *args = (struct arguments*)state->input;
    switch (key) {
        case 'm': args->cols = atoi(arg); break;
        case 'n': args->rows = atoi(arg); break;
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

void parallel_worker(const struct arguments& args, int id) {
    uint32_t serialized_size;
    MPI_Recv(
        &serialized_size,
        1,
        MPI_INT,
        MPI_MASTER_ID,
        MPI_DEFAULT_TAG,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    uint32_t* serialized_data = new uint32_t[serialized_size];
    MPI_Recv(
        serialized_data,
        serialized_size,
        MPI_INT,
        MPI_MASTER_ID,
        MPI_DEFAULT_TAG,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    Matrix m(serialized_data);

    //std::cout << m.ToString() << std::endl;

    delete serialized_data;
}

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

    // Send a copy of the matrix to all the workers.
    MPI_Request requests_size[args->workers];
    MPI_Request requests_data[args->workers];
    uint32_t* buf = matrix.Serialize();
    uint32_t size = matrix.SerializeSize();

    for (auto& workv : work) {
        std::cout
            << "Worker " << workv.first
            << " given " << workv.second.size() << " jobs." <<  std::endl;

        uint32_t pid = workv.first + 1;

        MPI_Isend(
            &size,
            1,
            MPI_INT,
            pid,
            MPI_DEFAULT_TAG,
            MPI_COMM_WORLD,
            &requests_size[pid - 1]);

        MPI_Isend(
            buf,
            size,
            MPI_INT,
            pid,
            MPI_DEFAULT_TAG,
            MPI_COMM_WORLD,
            &requests_data[pid - 1]);

        /*
        for (auto& pair : workv.second) {
            std::cout << pair.a << ", " << pair.b << std::endl;
        }
        std::cout << std::endl;
        */

    }

    std::cout << "Waiting for workers to recieve matrix." << std::endl;
    for (uint32_t i = 0; i < args->workers; i++) {
        MPI_Wait(&requests_size[i], MPI_STATUS_IGNORE);
        MPI_Wait(&requests_data[i], MPI_STATUS_IGNORE);
    }
    std::cout << "All workers have a copy of the matrix." << std::endl;

    delete buf;
    return results;
}

int main(int argc, char **argv) {
    int id;
    int num_procs;
    int retval;

    if ((retval = MPI_Init(&argc, &argv)) != MPI_SUCCESS) {
        print_and_exit(retval, "Could not initialize MPI");
    }

    if ((retval = MPI_Comm_size(MPI_COMM_WORLD, &num_procs)) != MPI_SUCCESS) {
        print_and_exit(retval, "Error getting number of processes in MPI");
    }

    if ((retval = MPI_Comm_rank(MPI_COMM_WORLD, &id)) != MPI_SUCCESS) {
        print_and_exit(retval, "Error getting rank/id from MPI");
    }

    // Parse and perform argument validation.
    struct arguments args;
    args.cols = 0;
    args.rows = 0;
    args.workers = num_procs - 1;
    args.verbose = false;
    argp_parse(&argp, argc, argv, 0, 0, &args);
    assert(args.cols > 1);
    assert(args.rows > 1);
    assert(args.workers > 1);
    assert(args.rows % args.workers == 0);

    // Generate a random matrix

    if (id == MPI_MASTER_ID) {
        Matrix matrix(args.rows, args.cols);
        random_matrix(matrix, &args);
        std::cout << matrix.ToString() << std::endl;

        // Copy the matrix for serial and parallel calculations.
        Matrix matrix_serial(matrix);
        Matrix matrix_parallel(matrix);

        std::vector<double> results = parallel(matrix_parallel, &args);

        serial(matrix_serial, results);
    } else {
        parallel_worker(args, id);
    }

    MPI_Finalize();
    return 0;
}

