#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

static void make_counts_displs(int n, int size, int *counts, int *displs) {
    int base=n/size, rem=n%size, off=0;
    for (int r=0;r<size;++r){ counts[r]=base+(r<rem?1:0); displs[r]=off; off+=counts[r]; }
}

int main(int argc,char**argv){
    MPI_Init(&argc,&argv);
    int rank,size; MPI_Comm_rank(MPI_COMM_WORLD,&rank); MPI_Comm_size(MPI_COMM_WORLD,&size);
    const int N=10000;

    if (argc<2 && rank==0) fprintf(stderr,"Usage: %s numbers.csv\n",argv[0]);

    double *all=NULL;
    if (rank==0){
        all=(double*)malloc(N*sizeof(double));
        FILE*f=fopen(argv[1],"r"); if(!f){perror("fopen"); MPI_Abort(MPI_COMM_WORLD,1);}
        int i=0; while(i<N && fscanf(f," %lf%*[, \n\r]", &all[i])==1) i++; fclose(f);
        if(i!=N){fprintf(stderr,"Need %d numbers, got %d\n",N,i); MPI_Abort(MPI_COMM_WORLD,2);}
    }
    int *counts=(int*)malloc(size*sizeof(int));
    int *displs=(int*)malloc(size*sizeof(int));
    make_counts_displs(N,size,counts,displs);
    int myN=counts[rank];
    double *chunk=(double*)malloc(myN*sizeof(double));

    MPI_Scatterv(all,counts,displs,MPI_DOUBLE,chunk,myN,MPI_DOUBLE,0,MPI_COMM_WORLD);

    double local=0.0; for(int i=0;i<myN;++i) local+=chunk[i];

    double total=0.0;
    MPI_Reduce(&local, &total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if(rank==0) printf("Exercise 5 (Scatterv + Reduce) total = %.10f\n", total);

    free(chunk); free(counts); free(displs); if(rank==0) free(all);
    MPI_Finalize(); return 0;
}


