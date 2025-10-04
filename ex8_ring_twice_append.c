#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char**argv){
    MPI_Init(&argc,&argv);
    int rank,size; MPI_Comm_rank(MPI_COMM_WORLD,&rank); MPI_Comm_size(MPI_COMM_WORLD,&size);

    int visits_target = 2*size; // two full passes
    int *arr = (int*)malloc(visits_target * sizeof(int));
    int len = 0;

    int next = (rank + 1) % size;
    int prev = (rank - 1 + size) % size;

    if (rank == 0) {
        // Start the ring
        arr[len++] = rank; // append self
        MPI_Send(&len, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
        MPI_Send(arr, visits_target, MPI_INT, next, 1, MPI_COMM_WORLD);

        // Receive final
        MPI_Recv(&len, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(arr, visits_target, MPI_INT, prev, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Exercise 8: final length=%d\nSequence:", len);
        for (int i=0;i<len;++i) printf(" %d", arr[i]);
        printf("\n");
    } else {
        // Relay until full
        for (;;) {
            MPI_Recv(&len, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(arr, visits_target, MPI_INT, prev, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (len >= visits_target) {
                // just forward final to next and stop
                MPI_Send(&len, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
                MPI_Send(arr, visits_target, MPI_INT, next, 1, MPI_COMM_WORLD);
                break;
            }

            // append my rank and forward
            arr[len++] = rank;
            MPI_Send(&len, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
            MPI_Send(arr, visits_target, MPI_INT, next, 1, MPI_COMM_WORLD);

            if (len >= visits_target) break;
        }
    }

    free(arr);
    MPI_Finalize(); return 0;
}

