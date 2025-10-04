#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_STEPS 100

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank, size; MPI_Comm_rank(MPI_COMM_WORLD, &rank); MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size < 2) { if(rank==0) fprintf(stderr,"Need at least 2 ranks\n"); MPI_Abort(MPI_COMM_WORLD,1); }

    int partner = (rank == 0) ? 1 : 0;

    int length = 0;
    int *arr = (int*)malloc((MAX_STEPS) * sizeof(int));

    srand((unsigned)time(NULL) + rank*1337);

    if (rank == 0) {
        // Start the ping-pong by sending an empty array (length=0)
        MPI_Send(&length, 1, MPI_INT, partner, 0, MPI_COMM_WORLD);
        // then loop receiving, appending, sending back
        for (;;) {
            MPI_Recv(arr, MAX_STEPS, MPI_INT, partner, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&length, 1, MPI_INT, partner, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (length >= MAX_STEPS) break;
            // append random and send to partner
            arr[length] = rand() % 1000;
            length++;
            MPI_Send(arr, MAX_STEPS, MPI_INT, partner, 1, MPI_COMM_WORLD);
            MPI_Send(&length, 1, MPI_INT, partner, 2, MPI_COMM_WORLD);
            if (length >= MAX_STEPS) break;
        }
        printf("Exercise 7: Final length=%d, last=%d\n", length, arr[length-1]);
    } else if (rank == 1) {
        // Wait for initial length from rank 0
        MPI_Recv(&length, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for (;;) {
            // append random then send to rank 0
            if (length < MAX_STEPS) {
                arr[length] = rand() % 1000;
                length++;
            }
            MPI_Send(arr, MAX_STEPS, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(&length, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
            if (length >= MAX_STEPS) break;
            // receive back
            MPI_Recv(arr, MAX_STEPS, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&length, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (length >= MAX_STEPS) break;
        }
        printf("Exercise 7 (rank1): Final length=%d, last=%d\n", length, arr[length-1]);
    }

    free(arr);
    MPI_Finalize();
    return 0;
}

