#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static void range_for_rank(long long N, int rank, int size, long long *lo, long long *hi) {
    // Balanced split [1..N] across ranks; each gets either ⌈N/size⌉ or ⌊N/size⌋ items
    long long base = N / size;
    long long rem  = N % size;
    long long start_index = rank * base + (rank < rem ? rank : rem);
    long long count = base + (rank < rem ? 1 : 0);
    *lo = start_index + 1;         // convert 0-based to 1-based range
    *hi = start_index + count;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size; MPI_Comm_rank(MPI_COMM_WORLD, &rank); MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long N = 10000000LL; // 10 million
    long long lo, hi; range_for_rank(N, rank, size, &lo, &hi);

    // local sum (use 128-bit temp if available; long double is also fine)
    long long local = 0;
    for (long long x = lo; x <= hi; ++x) local += x;

    if (rank == 0) {
        long long total = local;
        MPI_Status st;
        for (int i = 1; i < size; ++i) {
            long long recv_val = 0;
            // ANY_SOURCE & ANY_TAG to show wildcard receive
            MPI_Recv(&recv_val, 1, MPI_LONG_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
            total += recv_val;
            // (Optional) show who sent and which tag
            // printf("Got from rank %d tag %d: %lld\n", st.MPI_SOURCE, st.MPI_TAG, recv_val);
        }
        printf("Exercise 1(a): Sum 1..%lld = %lld\n", N, total);
    } else {
        // use a variety of tags (just to demonstrate)
        int tag = 100 + rank;
        MPI_Send(&local, 1, MPI_LONG_LONG, 0, tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}

