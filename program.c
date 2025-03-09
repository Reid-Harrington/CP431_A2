#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
//global variables
int *A; //A array
int *B;//B array
int n = 10; // size of each array


int binary_search(int array[], int size, int target);

//function for q sort, requires const void * as parameters
int compare(const void *a, const void *b) 
{
    return (*(int*)a - *(int*)b);
}


void random_array(int *a, int seed)
{
    srand(seed);
    for (int i = 0; i < n; i++) 
    {
        a[i] = rand() % 200;
    }

    qsort(a, n, sizeof(int), compare);
}

void merge(int *A, int sizeA, int *B, int sizeB, int *C)
{
    //indexes for A B and C arrays
    int i = 0, j = 0, k = 0;
    while(i < sizeA || j < sizeB) //while there are still elements left in either A or B
    {
        //copy element from A
        if (j >= sizeB || (i < sizeA && A[i] <= B[j])) // if no elements left in B, or if we still have elements in A and the current element is less than the current element in B
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

    //Allocate memory for our arrays
    A = (int *)malloc(n * sizeof(int));
    B = (int *)malloc(n * sizeof(int));
    if(world_rank == 0)
    {
        int seedA = time(NULL);
        int seedB = seedA + 17; 
        //generate random numbers for arrays
        random_array(A, seedA);
        random_array(B, seedB);
        printf("A array:\n");
        for(int i = 0; i < n; i++)
        {
            printf("%d ", A[i]);
        }
        printf("\n B array:\n");
        for(int i = 0; i < n; i++)
        {
            printf("%d ", B[i]);
        }
    }
    MPI_Bcast(A, n, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, n, MPI_INT, 0, MPI_COMM_WORLD);

    int base = n / world_size; // number of elements each process gets
    int rem = n % world_size; // remainder if n isnt divisible 


    //step 2: split points
    int *split_points = NULL;
    //rank 0 processes the split points for B
    if(world_rank == 0)
    {

        //creating the split points array, of size processors + 1
        split_points = (int *)malloc((world_size + 1) * sizeof(int));
        //first split point at 0
        split_points[0] = 0; 
        
        int base = n / world_size;
        int current = 0;
        //find all split points based on A partition
        for (int i = 0; i < world_size; i++)
        {
            int size_A = base + (i < rem ? 1 : 0);
            int a_last = current + size_A - 1;
            current += size_A;
            split_points[i + 1] = binary_search(B, n, A[a_last]);
        }
        split_points[world_size] = n;
    }
    //broadcast split points to all processes
    else if (world_rank != 0)
    {
        split_points = (int *)malloc((world_size + 1) * sizeof(int));
    }
    MPI_Bcast(split_points, world_size + 1, MPI_INT, 0, MPI_COMM_WORLD);

    //step 3: local partitioning
    //define a and b starting index, and its size
   // int a_start = world_rank * (n / world_size);
    //int size_A = (world_rank + 1) * (n / world_size) - a_start;
    int a_start = world_rank * base + (world_rank < rem ? world_rank : rem); //calculating corrent partitioning if theres a remainder
    int size_A = base + (world_rank < rem ? 1 : 0); //calculating corrent partitioning if theres a remainder
    
    int b_start = split_points[world_rank];
    int size_B = split_points[world_rank + 1] - b_start;


    //allocating memory for the C array, which holds our final merged result
    int *local_C = (int *) malloc((n * 2) * sizeof(int));
    int size_C = size_A + size_B;
    //step 4: merging
    merge(&A[a_start],size_A, &B[b_start], size_B, local_C);

    //step 5: gathering results of locally merged arrays at process 0
    int *C = NULL;
    int *recv_counts = NULL;
    int *displs = NULL;
    //process 0 prepares to gather data
    if (world_rank == 0) 
    {
        C = (int *)malloc(2 * n * sizeof(int));
        recv_counts = (int *)malloc(world_size * sizeof(int));
        displs = (int *)malloc(world_size * sizeof(int));
    }

    MPI_Gather(&size_C, 1, MPI_INT, recv_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (world_rank == 0) 
    {
        displs[0] = 0;
        for (int i = 1; i < world_size; i++) 
        {
            displs[i] = displs[i - 1] + recv_counts[i - 1];
        }
    }
    //gather all parts into C
    MPI_Gatherv(local_C, size_C, MPI_INT, C, recv_counts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    
    // Root prints the merged array
    if (world_rank == 0) 
    {
        printf("Merged array: ");
        for (int i = 0; i < 2 * n; i++) {
            printf("%d ", C[i]);
        }
        printf("\n");
        free(C);
        free(recv_counts);
        free(displs);
    }


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

        if (array[middle] <= target) left = middle + 1;

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

    results: rank 0 process gathers all local C arrays using MPI_Gatherv to form the final merged array C.
/*/
