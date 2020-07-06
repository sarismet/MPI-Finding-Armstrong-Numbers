/*Student Name: İsmet Sarı
Student Number: 2016400324
Compile Status: Compiling
Program Status: Working
*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MASTER 0   /* taskid of first process */
#define indexmsg 1 //the tag to send int values to workers
#define arraymsg 2 // tag to send array to workers

//this function is designed to shuffle items of arrays.
void shuffle(int *array, size_t n)
{
    if (n > 1)
    {
        size_t i;
        for (i = 0; i < n - 1; i++)
        {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}
// the all functions swap, partition, and quickSort are the part of clasic quick sort.
// I took all the codes from geekforgeek.com
void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

/* This function takes last element as pivot, places 
   the pivot element at its correct position in sorted 
    array, and places all smaller (smaller than pivot) 
   to left of pivot and all greater elements to right 
   of pivot */
int partition(int arr[], int low, int high)
{
    int pivot = arr[high]; // pivot
    int i = (low - 1);     // Index of smaller element

    for (int j = low; j <= high - 1; j++)
    {
        // If current element is smaller than the pivot
        if (arr[j] < pivot)
        {
            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

/* The main function that implements QuickSort 
 arr[] --> Array to be sorted, 
  low  --> Starting index, 
  high  --> Ending index */
void quickSort(int arr[], int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now 
           at right place */
        int pi = partition(arr, low, high);

        // Separately sort elements before
        // partition and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int main(int argc, char **argv)
{

    // Initialize the MPI environment
    MPI_Status status;

    //we take the array size from user.
    int ARRAYSIZE = atoi(argv[1]);
    int *data;                                    //we are defining the general data
    data = (int *)calloc(ARRAYSIZE, sizeof(int)); //and constructing it
    MPI_Init(NULL, NULL);                         //int the MPI
    // Find out rank, size
    int numtasks, taskid;
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid); //take the task id
    int numworkers;
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks); //and the number of processors

    numworkers = numtasks - 1;                //specify the worker processors numbers
    int chunksize = (ARRAYSIZE / numworkers); //specift the chunksize

    if (taskid == MASTER) //if we are in master
    {

        for (int i = 0; i < ARRAYSIZE; i++) //we constructing the array
            data[i] = i + 1;
        shuffle(data, ARRAYSIZE); // and shuffle them
        int index = 0;

        //Here we send the part of the data array to worker processors.
        for (int i = 1; i <= numworkers; i++)
        {

            MPI_Send(&data[index], chunksize, MPI_INT, i, arraymsg,
                     MPI_COMM_WORLD);
            index = index + chunksize;
        }
        //when we are finish we take the total number of amstrong number and assign this variable
        int global_total_of_all_amstrong_numbers = 0;
        //this is the size of array filled with all amstrong number
        int size_of_all_amstrong_numbers = 0;

        //we constuct an array to store all amstrong numbers
        int *global_total_array;
        global_total_array = (int *)calloc((ARRAYSIZE / 5), sizeof(int));
        for (int i = 1; i <= numworkers; i++)
        {
            int size = 0; //receive size of amstrong number from a worker processor
            MPI_Recv(&size, 1, MPI_INT, i, 3, MPI_COMM_WORLD,
                     &status);

            int *from_worker_process;
            from_worker_process = (int *)calloc(size, sizeof(int)); //create an array to store them
            MPI_Recv(&from_worker_process[0], size, MPI_INT, i, 4, MPI_COMM_WORLD,
                     &status);
            //assign all these numbers to global array
            for (int k = 0; k < size; k++)
            {
                global_total_array[size_of_all_amstrong_numbers] = from_worker_process[k];
                size_of_all_amstrong_numbers += 1;
            }
        }

        //we receibr the total of all amstrong number here
        MPI_Recv(&global_total_of_all_amstrong_numbers, 1, MPI_INT, numworkers, 5, MPI_COMM_WORLD,
                 &status);

        //Sort all these numbers here
        quickSort(global_total_array, 0, size_of_all_amstrong_numbers - 1);
        FILE *fptr; //create the file
        fptr = fopen("armstrong.txt", "w+");
        int armstrong_number = 0;
        //write all amstrong number here on the file.
        for (int i = 0; i < size_of_all_amstrong_numbers; i++)
        {
            armstrong_number = global_total_array[i];
            fprintf(fptr, "%d\n", armstrong_number);
        }
        //print all the total number of amstrong number on terminal.
        printf("MASTER: Sum of all Armstrong numbers = %d \n", global_total_of_all_amstrong_numbers);
    }

    //if we are on worker processoe
    if (taskid > MASTER)
    {
        //we define master as a source
        int source = MASTER;
        int index = (taskid - 1) * chunksize; //define the index

        int local_total = 0;                    //to store total number of amstrong number only this processor
        int local_size_of_amstrong_numbers = 0; //the size of all amstrong numbers
        int limit = 0;                          //we define a limit to create a dynamically allocated array
        int global_total_of_amstrong_numbers = 0;
        if (taskid == 1)
            limit = chunksize / 2; //if we are the first worker processor then limit need to be higher since the first 9 number is amstrong number
        else
            limit = chunksize / 3;

        int *local_amstrong_numbers;
        //construct the array dynamically
        local_amstrong_numbers = (int *)calloc(limit, sizeof(int));
        //each worker processor take some part of the general data array from master processor
        MPI_Recv(&data[index], chunksize, MPI_INT, source, arraymsg,
                 MPI_COMM_WORLD, &status);
        //all taje worker processors except the first worker procesors take total number of amstrong numbers up to this point
        if (taskid != 1)
            MPI_Recv(&global_total_of_amstrong_numbers, 1, MPI_INT, taskid - 1, 11,
                     MPI_COMM_WORLD, &status);
        //ın this loop we iterate the data array and check if the number is amstrong or not
        for (int i = index; i < index + chunksize; i++)
        {

            int originalNumber;
            int number = data[i];
            int count = 0;

            originalNumber = number;

            // number of digits calculation
            while (originalNumber != 0)
            {
                originalNumber /= 10;
                ++count;
            }

            originalNumber = number;
            int rem = 0;
            int result = 0;

            // result contains sum of nth power of individual digits
            while (originalNumber != 0)
            {
                rem = originalNumber % 10;
                result += pow(rem, count);
                originalNumber /= 10;
            }

            // check if number is equal to the sum of nth power of individual digits
            if ((int)result == number)
            {
                //This is probally is never going to be executed but in any case that the size of dynamically allocated
                //array is not enough then we need to reallcaote
                if (limit == local_size_of_amstrong_numbers)
                {
                    limit = limit * 2;
                    local_amstrong_numbers = realloc(local_amstrong_numbers, limit * sizeof(int));
                }
                //add the amstrong numbers to local array
                local_amstrong_numbers[local_size_of_amstrong_numbers] = number;
                local_size_of_amstrong_numbers += 1; //increase the size by one
                local_total = local_total + number;  //and add the total variable
            }
        }
        //print the local total on terminal
        printf("Sum of Armstrong numbers in Process %d = %d\n", taskid, local_total);
        //add the local total of amstrong number to global
        global_total_of_amstrong_numbers += local_total;
        //send the size of local amstong number to master
        MPI_Send(&local_size_of_amstrong_numbers, 1, MPI_INT, MASTER, 3, MPI_COMM_WORLD);
        //send the array filled with all local amstrong number to master
        MPI_Send(&local_amstrong_numbers[0], local_size_of_amstrong_numbers, MPI_INT, MASTER, 4, MPI_COMM_WORLD);
        // if the worker process is not the last one then send the global total to next processor
        if (taskid != numworkers)
        {
            MPI_Send(&global_total_of_amstrong_numbers, 1, MPI_INT, taskid + 1, 11, MPI_COMM_WORLD);
        }
        //if we are the last worker processor then send the total global amstrong number to master instead of another worker processor
        else if (taskid == numworkers)
        {

            MPI_Send(&global_total_of_amstrong_numbers, 1, MPI_INT, MASTER, 5, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize(); //end the MPI

    return 0;
}