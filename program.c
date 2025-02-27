#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int binary_search(int array[], int size, int target);

int main(int argc, char *argv[]) 
{
    MPI_Init(&argc, &argv);
 
    int world_size, world_rank;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


    printf("Hello from process %d out of %d\n", world_rank, world_size);

    int n = atoi(argv[1]); //converts the first argument to a int (n)
    int r = n / (2 * world_size); //number of groups
    int k = log(n); //number of elements in each group
    int array[] = {1,2,3,4,5,6};

    printf("%d, %d, %d\n", n,r,k);
    for(int i = 1; i < r + 1; i++)
    {
        // int insersionPoint = binary_search()
        //getting length array
        int size = sizeof(array) / sizeof(array[0]);
        int target = 3;

        int index = binary_search(array, size, target);
        printf("%d", index);
    }



    MPI_Finalize();
    return 0;
}

//helper method for binary search
int binary_search(int array[], int size, int target)
{
    int left = 0;
    int right = size - 1; //right = length of array - 1

    while (left <= right) 
    {
        int middle = left + (right - left) / 2; 
        //target found
        if (array[middle] == target) return middle;  
        //search right half
        else if (array[middle] < target) left = middle + 1;  
        //search left half
        else right = middle - 1; 
    }

    //target not found
    return -1; 
} 