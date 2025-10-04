// ex2_bcast_sum.c  (self-contained)
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// --- helper: compute [lo,hi) for rank ---
static void block_bounds(int n, int rank, int size, int *lo, int *hi) {
    int base = n / size, rem = n % size;
    int start = rank * base + (rank < rem ? rank : rem);
    int count = base + (rank < rem ? 1 : 0);
    *lo = start; *hi = start + count; // half-open
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size; MPI_Comm_rank(MPI_COMM_WORLD, &rank); MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2 && rank == 0) {
        fprintf(stderr, "Usage: %s numbers.csv\n", argv[0]);
    }

    const int N = 10000;
    double *arr = (double*)malloc(N * sizeof(double));
    if (!arr) { if(rank==0) perror("malloc"); MPI_Abort(MPI_COMM_WORLD, 1); }

    if (rank == 0) {
        FILE *f = fopen(argv[1], "r");
        if (!f) { perror("fopen"); MPI_Abort(MPI_COMM_WORLD, 2); }
        int i = 0;
        while (i < N && fscanf(f, " %lf%*[, \n\r]", &arr[i]) == 1) i++;
        fclose(f);
        if (i != N) {
            fprintf(stderr, "Expected %d numbers, got %d\n", N, i);
            MPI_Abort(MPI_COMM_WORLD, 3);
        }
    }

    MPI_Bcast(arr, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int lo, hi; block_bounds(N, rank, size, &lo, &hi);
    double local = 0.0;
    for (int i = lo; i < hi; ++i) local += arr[i];

    if (rank == 0) {
        double total = local;
        for (int src = 1; src < size; ++src) {
            double v; MPI_Recv(&v, 1, MPI_DOUBLE, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total += v;
        }
        printf("Exercise 2 total = %.10f\n", total);
    } else {
        MPI_Send(&local, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    free(arr);
    MPI_Finalize();
    return 0;
}

