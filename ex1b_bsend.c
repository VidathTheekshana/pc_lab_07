#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static void range_for_rank(long long N, int rank, int size, long long *lo, long long *hi) {
    long long base = N / size;
    long long rem  = N % size;
    long long start_index = rank * base + (rank < rem ? rank : rem);
    long long count = base + (rank < rem ? 1 : 0);
    *lo = start_index + 1;
    *hi = start_index + count;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size; MPI_Comm_rank(MPI_COMM_WORLD, &rank); MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long N = 10000000LL;
    long long lo, hi; range_for_rank(N, rank, size, &lo, &hi);

    long long local = 0;
    for (long long x = lo; x <= hi; ++x) local += x;

    if (rank == 0) {
        long long total = local;
        for (int i = 1; i < size; ++i) {
            long long v = 0;
            MPI_Recv(&v, 1, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total += v;
        }
        printf("Exercise 1(b): Sum 1..%lld = %lld\n", N, total);
    } else {
        // Attach a buffer big enough: MPI_BSEND_OVERHEAD + payload per message.
        int bufsize = 0;
        int sz;
        MPI_Pack_size(1, MPI_LONG_LONG, MPI_COMM_WORLD, &sz);
        bufsize = sz + MPI_BSEND_OVERHEAD;
        void *buf = malloc(bufsize);
        MPI_Buffer_attach(buf, bufsize);

        MPI_Bsend(&local, 1, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD);

        // Detach and free
        void *old; int oldsz;
        MPI_Buffer_detach(&old, &oldsz);
        free(old);
    }

    MPI_Finalize();
    return 0;
}

