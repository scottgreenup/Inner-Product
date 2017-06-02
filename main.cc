
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
#include <list>

#include "matrix.h"
#include "timer.h"
#include "vector.h"

const int MPI_DEFAULT_TAG = 1;
const int MPI_MASTER_ID = 0;

typedef std::map<uint32_t, std::map<uint32_t, double>> DotResults;

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

bool serial(const Matrix& matrix, DotResults results) {
    std::cout << std::endl;
    std::cout << "Performing serial check of the values." << std::endl;

    for (uint32_t i = 0; i < matrix.rows(); i++) {
        for (uint32_t j = i+1; j < matrix.rows(); j++) {
            double result = matrix[i].Dot(matrix[j]);

            if (!doubleEqual(results[i][j], result)) {
                std::cout << "(" << i << ", " << j << "): " << result << std::endl;
                std::cout << "and did not match... ";
                std::cout << results[i][j] << std::endl;
                return false;
            }
        }
    }

    return true;
}

class WorkPair {
public:
    uint32_t a;
    uint32_t b;
    WorkPair() : a(0), b(0) {}
    WorkPair(uint32_t a, uint32_t b) : a(a), b(b) { }

    static uint32_t MarshalSize(const std::vector<WorkPair>& workload) {
        // Room for the size and for each pair of ints.
        return 1 + workload.size() * 2;
    }

    static uint32_t* Marshal(const std::vector<WorkPair>& workload) {
        uint32_t bufSize = WorkPair::MarshalSize(workload);
        uint32_t* buf = new uint32_t[bufSize];
        buf[0] = workload.size();

        for (uint32_t i = 0; i < workload.size(); i++) {
            buf[1 + i*2]     = workload[i].a;
            buf[1 + i*2 + 1] = workload[i].b;
        }
        return buf;
    }

    static std::vector<WorkPair> Unmarshal(uint32_t* marshalled_data) {
        uint32_t bufSize = marshalled_data[0];

        std::vector<WorkPair> workload(bufSize);

        for (uint32_t i = 0; i < bufSize; i++) {
            workload[i].a = marshalled_data[1 + i*2];
            workload[i].b = marshalled_data[1 + i*2 +1];
        }

        return workload;
    }
};

class Result {
public:
    uint32_t a;
    uint32_t b;
    double result;


    static size_t MarshalSize() {
        return sizeof(uint32_t) * 2 + sizeof(double);
    }

    char* Marshal() {
        char* data = new char[MarshalSize()];
        char* ptr = &data[0];

        (reinterpret_cast<uint32_t*>(ptr))[0] = a;
        ptr += sizeof(uint32_t);
        (reinterpret_cast<uint32_t*>(ptr))[0] = b;
        ptr += sizeof(uint32_t);
        (reinterpret_cast<double*>(ptr))[0] = result;

        return data;
    }

    void Unmarshal(char* ser) {
        char* data = new char[MarshalSize()];
        char* ptr = &ser[0];

        a = (reinterpret_cast<uint32_t*>(ptr))[0];
        ptr += sizeof(uint32_t);
        b = (reinterpret_cast<uint32_t*>(ptr))[0];
        ptr += sizeof(uint32_t);
        result = (reinterpret_cast<double*>(ptr))[0];
    }
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

    MPI_Recv(
        &serialized_size,
        1,
        MPI_INT,
        MPI_MASTER_ID,
        MPI_DEFAULT_TAG,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    delete serialized_data;
    serialized_data = new uint32_t[serialized_size];
    MPI_Recv(
        serialized_data,
        serialized_size,
        MPI_INT,
        MPI_MASTER_ID,
        MPI_DEFAULT_TAG,
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

    std::vector<WorkPair> workload = WorkPair::Unmarshal(serialized_data);
    delete serialized_data;

    std::vector<char*> marshals(workload.size());
    std::vector<MPI_Request> requests;

    for (WorkPair& wp : workload) {
        Result result;
        result.a = wp.a;
        result.b = wp.b;
        result.result = m[wp.a].Dot(m[wp.b]);

        char* m = result.Marshal();
        marshals.push_back(m);
        requests.push_back(MPI_Request());

        MPI_Isend(
            m,
            Result::MarshalSize(),
            MPI_CHAR,
            MPI_MASTER_ID,
            MPI_DEFAULT_TAG,
            MPI_COMM_WORLD,
            &requests.back());

        if (args.verbose) {
            std::cout
                << "Sending: (" << result.a << ", " << result.b << ")"
                << " == " << result.result << std::endl;
        }
    }

    for (uint32_t i = 0; i < requests.size(); i++) {
        MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
    }

    for (uint32_t i = 0; i < marshals.size(); i++) {
        delete marshals[i];
    }
}


DotResults parallel(const Matrix& matrix, const struct arguments* args) {
    DotResults results;

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

    // Create workloads worker_id -> Work
    std::map<uint32_t, std::vector<WorkPair>> workload;
    uint32_t total_workload = 0;

    // Go through all the pairs and assign them to a worker.
    for (uint32_t i = 0; i < (args->rows / 2); i++) {
        uint32_t curr = i % args->workers;

        for (uint32_t j = i+1; j < args->rows; j++) {
            workload[curr].push_back(WorkPair(i, j));
            total_workload++;
        }

        uint32_t opposite = args->rows - 1 - i;
        for (uint32_t j = opposite+1; j < args->rows; j++) {
            workload[curr].push_back(WorkPair(opposite, j));
            total_workload++;
        }
    }

    // We miss the middle row of jobs if the amount of rows is even.
    if (args->rows % 2 == 1) {
        uint32_t i = (args->rows / 2);
        uint32_t curr = i % args->workers;

        for (uint32_t j = i; j < args->rows; j++) {
            workload[curr].push_back(WorkPair(i, j));
            total_workload++;
        }
    }

    // Send a copy of the matrix to all the workers.
    std::vector<MPI_Request> requests;
    std::vector<uint32_t*> wm_data;
    uint32_t* buf = matrix.Serialize();
    uint32_t size = matrix.SerializeSize();

    for (auto& work : workload) {
        uint32_t pid = work.first + 1;
        std::cout << "MASTER: Sending " << pid << " matrix." << std::endl;

        requests.push_back(MPI_Request());
        MPI_Isend(
            &size,
            1,
            MPI_INT,
            pid,
            MPI_DEFAULT_TAG,
            MPI_COMM_WORLD,
            &requests.back());

        requests.push_back(MPI_Request());
        MPI_Isend(
            buf,
            size,
            MPI_INT,
            pid,
            MPI_DEFAULT_TAG,
            MPI_COMM_WORLD,
            &requests.back());

        std::cout << "MASTER: Sending " << pid << " workload." << std::endl;
        uint32_t wm_size = WorkPair::MarshalSize(work.second);
        uint32_t* wm = WorkPair::Marshal(work.second);
        wm_data.push_back(wm);

        MPI_Isend(
            &wm_size,
            1,
            MPI_INT,
            pid,
            MPI_DEFAULT_TAG,
            MPI_COMM_WORLD,
            &requests.back());

        MPI_Isend(
            wm,
            wm_size,
            MPI_INT,
            pid,
            MPI_DEFAULT_TAG,
            MPI_COMM_WORLD,
            &requests.back());
    }


    std::cout << "MASTER: Waiting on workers..." << std::endl;
    for (uint32_t i = 0; i < requests.size(); i++) {
        MPI_Wait(&requests[i], MPI_STATUS_IGNORE);
    }
    delete buf;
    std::cout << "MASTER: Workers received data." << std::endl;

    std::cout << "MASTER: Dot Products: " << total_workload << std::endl;
    char* serialized_data = new char[Result::MarshalSize()];
    double milestone = 0.1;
    for (uint32_t i = 0; i < total_workload; i++) {
        MPI_Recv(
            serialized_data,
            Result::MarshalSize(),
            MPI_CHAR,
            MPI_ANY_SOURCE,
            MPI_DEFAULT_TAG,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);

        Result result;
        result.Unmarshal(serialized_data);
        results[result.a][result.b] = result.result;
        results[result.b][result.a] = result.result;

        if (args->verbose) {
            std::cout << "Received (" << result.a << ", " << result.b << ")";
            std::cout << " == " << result.result << std::endl;
        }

        double percentage = static_cast<double>(i);
        percentage /= static_cast<double>(total_workload);
        if (percentage > milestone) {
            std::cout << "MASTER: Progress at "
                      << static_cast<uint32_t>(percentage * 100)
                      << "%" << std::endl;
            milestone += 0.1;
        }
    }

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
    assert(args.workers >= 1);

    // Generate a random matrix
    if (id == MPI_MASTER_ID) {
        Matrix matrix(args.rows, args.cols);
        random_matrix(matrix, &args);

        if (args.verbose) {
            std::cout << matrix.ToString() << std::endl;
        }

        // Copy the matrix for serial and parallel calculations.
        Matrix matrix_serial(matrix);
        Matrix matrix_parallel(matrix);

        DotResults results = parallel(matrix_parallel, &args);

        if (serial(matrix_serial, results)) {
            std::cout << "Serial check passed." << std::endl;
        }
    } else {
        parallel_worker(args, id);
    }

    MPI_Finalize();
    return 0;
}

