#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>



int binary_search(int array[], int size, int target);


void merge(int *A, int *B, int *C)
{
    //indexes for A B and C arrays
    int i, j, k = 0;
    while(i < n || j < n) //while there are still elements left in either A or B
    {
        //copy element from A
        if (j >= n || (i < n && A[i] <= B[j])) // if no elements left in B, or if we still have elements in A and the current element is less than the current element in B
        {
            C[k++] = A[i++];
        }
        //copy element from B
        else
        {
            C[k++] = B[j++];
        }
    }


}

int main(int argc, char *argv[]) 
{
    //step 1: Initiating MPI
    MPI_Init(&argc, &argv);
 
    int world_size, world_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    printf("Hello from process %d out of %d\n", world_rank, world_size);

    //Global variables
    int A[] = {2,4,6,8,10,12,14,16};
    int B[] = {1,3,5,7,9,11,13,15};

    int n = 8; //atoi(argv[1]); //size of each input array
    int r = n / (2 * world_size); //number of groups
    int k = (int)log(n) / log(2); //number of elements in each group


    int target = 3;

    //step 2: split points
    int *split_points = NULL;
    //rank 0 processes the split points for B
    if(world_rank == 0)
    {
        //creating the split points array, of size processors + 1
        split_points = (int *)malloc((world_size + 1) * sizeof(int));
        //first split point at 0
        split_points[0] = 0; 

        //find all split points based on A partition
        for (int i = 0; i < world_size; i++)
        {
            int a_lastIndex = (i + 1) * (n / world_size) - 1; //last index of the current segment in A for process i
            if (a_lastIndex >= n) a_lastIndex = n - 1; //if the last index is greater than the size of array, assign it to n-1
            int a_end = A[a_lastIndex];


            //finds the first index in B where the element is >= a_end. 
            //this becomes the split point for B for process i + 1
            split_points[i + 1] = binary_search(B, n, a_end);
        }

        
        for (int i = 0; i < world_size + 1; i++) {
            printf("%d ", split_points[i]);
        }
    }
    //broadcast split points to all processes
    else if (world_rank != 0)
    {

    }

    //step 3: local partitioning

    //allocating memory for the C array, which holds our final merged result
    int *C_part = malloc((a_size + b_size) * sizeof(int));
    //step 4: merging
    merge(&A[a_start], a_size, &B[b_start], b_size, C_part);

    //step 5: gathering results

    MPI_Finalize();
    return 0;
}

//helper method for binary search, finds lower bound.
int binary_search(int array[], int size, int target)
{
    int left = 0;
    int right = size; 

    while (left < right) 
    {
        int middle = left + (right - left) / 2; 

        if (array[middle] < target) left = middle + 1;

        else right = middle;
    }

    //target not found
    return left; 
} 

/*/ ALGORITHM
    init: MPI is initialized

    global arrays: A and B are global sorted arrays.

    split points: rank 0 computes split points in B using binary search based on partitions of A. These points ensure each processor merges non-overlapping sections.

    broadcast: the split points are broadcasted to all processes.

    local merge: each process extracts its portion of A and B, merges them sequentially, and stores the result in C_part.

    results: rank 0 process gathers all C_part arrays using MPI_Gatherv to form the final merged array C.
/*/