#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define TAG 0
#define REP 1000

/**
 * argv[1] = numero de rebote
 * argv[2] = tamaño del paquete en MB
 */
int main(int argc, char** argv)
{
    int world_size, world_rank, name_len, i, j, rafaga;
    int aux;
    long size;
    double start_time, end_time, total = 0;
    void *dummy;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Status status;
    
    //Rafaga y tamaño paquete 
    if (argc != 3) exit(1);
    rafaga = atoi(argv[1]);
    size = atoi(argv[2]);
    //size = atoi(argv[2]) * 1024 * 1024;

    //Initialize the MPI environment
    MPI_Init(NULL, NULL);

    //Allocate dummy memory
    dummy = malloc(size);

    //Get the number of processes, rank and name
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(processor_name, &name_len);

    //Print info   
    if (!world_rank)
    {
        printf("Nº Bounces %d, Packet size %d\n", rafaga, size);
        printf("Dummy array create\n");
    }

    // Envio y recepcion de rafagas
    for (i = 0; i < REP; ++i)
    {
        start_time = MPI_Wtime();
        if (!(world_rank % 2))
        {
            for (j = 0; j < rafaga; ++j)
                MPI_Send(dummy, size, MPI_BYTE, world_rank + 1, world_rank, MPI_COMM_WORLD);

            MPI_Recv(&aux, 1, MPI_INT, world_rank + 1, world_rank + 1, MPI_COMM_WORLD, &status);
        } else
        {
            for (j = 0; j < rafaga; ++j)
                MPI_Recv(dummy, size, MPI_BYTE, world_rank - 1, world_rank - 1, MPI_COMM_WORLD, &status);

            MPI_Send(&world_rank, 1, MPI_INT, world_rank - 1, world_rank, MPI_COMM_WORLD);
        }
        end_time = MPI_Wtime();
        total += (end_time - start_time);
    }
    total = total / (double) REP;

    if (!world_rank)
    printf("Time %f s, Time for Packet: %f\n", total, total / rafaga);
    //Finalize the MPI enviroment
    MPI_Finalize();
}
