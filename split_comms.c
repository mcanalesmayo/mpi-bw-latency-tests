/**
*   Authors:
*       - Agustin Navarro Torres
*       - Marcos Canales Mayo
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <float.h>

#define TAG 0

/**
 * argv[1] = repetitions
 * argv[2] = number of bursts
 * argv[3] = packet size in bytes
 */
int main(int argc, char** argv)
{
    int world_size, world_rank, name_len, i, j, n_bursts;
    int color, pair_rank, pair_mate_rank, senders_size, senders_rank;
    int senders_group_excl_range[1][3];
    int aux, repetitions;
    long packet_size;
    double start_time, end_time, *res_time;
    double local_worst_lat = DBL_MIN, local_best_lat = DBL_MAX, local_mean_lat = 0;
    double global_worst_lat, global_best_lat, global_mean_lat, bandwith;
    void *dummy;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Status status;
    MPI_Group world_group, senders_group;
    MPI_Comm pair_comm, senders_comm;
    
    // Get params
    if (argc != 4) exit(1);
    repetitions = atoi(argv[1]);
    n_bursts = atoi(argv[2]);
    packet_size = atoi(argv[3]);

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Allocate dummy memory
    dummy = malloc(packet_size);

    // Get the number of processes, rank and name
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(processor_name, &name_len);

    // Split into communicators of pairs of processes, determined by rank: 0 and 1; 2 and 3; 4 and 5; ...
    color = world_rank / 2;
    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &pair_comm);
    MPI_Comm_rank(pair_comm, &pair_rank);
    // Pair communicator ranks will always be 0 and 1
    pair_mate_rank = (pair_rank % 2) ? 0 : 1;

    // Pair rank 1: sender
    // Pair rank 0: receiver

    // Create senders communicator, needed to gather results

    // first, last, stride: exclude even ranks
    senders_group_excl_range[0][0] = 0;
    senders_group_excl_range[0][1] = world_size - 1;
    senders_group_excl_range[0][2] = 2;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    MPI_Group_range_excl(world_group, 1, senders_group_excl_range, &senders_group);
    MPI_Comm_create(MPI_COMM_WORLD, senders_group, &senders_comm);
    if (senders_comm != MPI_COMM_NULL){
        MPI_Comm_size(senders_comm, &senders_size);
        MPI_Comm_rank(senders_comm, &senders_rank);
    
        // Allocate memory on senders, to measure latency
        res_time = malloc(repetitions*sizeof(double));

        // Root process
	    if (senders_rank == 0) {
	        // Print info
	        printf("Number of Repetitions %d, Number of Bounces %d, Packet size %d bytes\n", repetitions, n_bursts, packet_size);
	    }
    }

    // Send and receive bursts
    for (i = 0; i < repetitions; ++i)
    {
        if (pair_rank)
        {
            // Senders measure latency
            start_time = MPI_Wtime();
            for (j = 0; j < n_bursts; ++j)
                MPI_Send(dummy, packet_size, MPI_BYTE, pair_mate_rank, TAG, pair_comm);

            MPI_Recv(&aux, 1, MPI_INT, pair_mate_rank, TAG, pair_comm, &status);
            end_time = MPI_Wtime();
            res_time[i] = end_time - start_time;
        } else
        {
            for (j = 0; j < n_bursts; ++j)
                MPI_Recv(dummy, packet_size, MPI_BYTE, pair_mate_rank, TAG, pair_comm, &status);

            MPI_Send(&world_rank, 1, MPI_INT, pair_mate_rank, TAG, pair_comm);
        }
    }

    // Reduce results
    // Need to check MPI_COMM_NULL because some processes are not in the senders communicator group,
    // which will raise an exception when calling MPI_Gather
    if (senders_comm != MPI_COMM_NULL){
    	// Local vector reduction
    	for(i = 0; i < repetitions; i++){
    		local_mean_lat += res_time[i];
    		if (res_time[i] > local_worst_lat) local_worst_lat = res_time[i];
    		if (res_time[i] < local_best_lat) local_best_lat = res_time[i];
    	}

        // Global reduction
        // Root process calculates worst, best and mean latency
        MPI_Reduce(&local_mean_lat, &global_mean_lat, 1, MPI_DOUBLE, MPI_SUM, 0, senders_comm);
        MPI_Reduce(&local_worst_lat, &global_worst_lat, 1, MPI_DOUBLE, MPI_MAX, 0, senders_comm);
		MPI_Reduce(&local_best_lat, &global_best_lat, 1, MPI_DOUBLE, MPI_MIN, 0, senders_comm);

        // Root process in senders communicator is 0
        if (senders_rank == 0){
            bandwith = (repetitions*n_bursts*packet_size)/global_mean_lat;
            global_mean_lat /= repetitions*senders_size;

            // Print results
            printf("Bandwith: %.2f MBytes/s\n", bandwith/(1024*1024));
            printf("Mean latency per packet: %f\n", global_mean_lat);
            printf("Worst latency per packet: %f\n", global_worst_lat);
            printf("Best latency per packet: %f\n", global_best_lat);
        }
    }

    // Free allocated memory
    free(dummy);
    if (senders_comm != MPI_COMM_NULL){
        free(res_time);
        MPI_Comm_free(&senders_comm);
    }
    MPI_Group_free(&senders_group);
    MPI_Comm_free(&pair_comm);

    // Finalize the MPI enviroment
    MPI_Finalize();

    return 0;
}
