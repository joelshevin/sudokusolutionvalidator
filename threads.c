#define _gnu_source
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<ncurses.h>
#include <ctype.h>
#include<time.h>
#include <sys/mman.h>
#include <sys/wait.h>
pthread_mutex_t lock;
pthread_cond_t condA  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct 
{
    // This structure is used to pass the starting row,column data to the child threads

    int row;
    int col;
    int (* arr1)[9];

} parameters;

//declare and initialize counter variable as a global variable to share among threads
int counter =0;


//this method is used to check the validity of rows by the child threads
/*check each row and return 1 if valid and 0 if invalid*/
void * row_check(void * params) {
	
    pthread_t pthread_self(void);
    
    //open file to write invalid rows
	FILE *f = fopen("threads_log_file.txt", "a");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}   
    pthread_mutex_lock(&lock);
    parameters * data = (parameters *) params;
    int startRow = data->row;
    int startCol = data->col;   
    for (int i = startRow; i <= startRow; ++i) {
        int row[10] = {0};
        for (int j = startCol; j < 9; ++j) {
            int val = data->arr1[i][j];
	    
            if (row[val] != 0) {
		   printf("Validation result from thread id: %u. :row %d is invalid\n", (unsigned int) pthread_self(),startRow+1);
		   fprintf(f, "Validation result from thread id: %u. :row %d is invalid\n", (unsigned int) pthread_self(),startRow+1);
		   pthread_mutex_unlock(&lock);
	           return (void *) 0;
            }
            else{
                row[val] = 1;
            }
	    
        }
    }	
    printf("Validation result from thread id: %u. :row %d is valid\n",(unsigned int) pthread_self(),startRow+1);
    counter++;
    fclose(f);
    pthread_mutex_unlock(&lock);
    return (void *) 1;

}

//this method is used to check the validity of columns by the child threads
/*checks all columns and stores the number of valid columns in the variable ‘validColumns’*/
void * col_check(void * params) {
    parameters * data = (parameters *) params;
    int startRow = data->row;
    int startCol = data->col;
    int validColumns=9;

    //open file to append invalid 3x3 squares
	FILE *f = fopen("threads_log_file.txt", "a");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}

    for (int i = startCol; i < 9; ++i) {
        int col[10] = {0};
        for (int j = startRow; j < 9; ++j) {
            int val = data->arr1[j][i];
	    parameters * data = (parameters *) params;
            if (col[val] != 0) {
		validColumns=validColumns-1;
		fprintf(f, "Validation result from thread id: %u. :Column %d is invalid\n", (unsigned int) pthread_self(),i+1);
          	break;
	    }
            else{
                col[val] = 1;
		

            }
        }
    }
    counter=counter+validColumns;
    printf("Validation result from thread id: %u. : %d out of 9 Columns is valid\n",(unsigned int) pthread_self(),validColumns);
    fclose(f);
    return (void *) 1;
}



//this method is used to check the validity of the 3x3 squares
/*this method checks all sub-grids and returns an array with a ‘valid’ value.
value is 1 if its valid, 0 if invalid*/
void * check_square(void * params) {
    
    parameters * data = (parameters *) params;
    int startRow = data->row;
    int startCol = data->col;
    
    int *colm = malloc(sizeof(int)*9);
    int valid=1;

  for(int x=0; x<9; x++)
  {
	
	if(x==1)
	{
		startRow=0;
		startCol=3;
	}

	if(x==2)
	{
		startRow=0;
		startCol=6;
	}
	
	if(x==3)
	{
		startRow=3;
		startCol=0;
	}
	
	if(x==4)
	{
		startRow=3;
		startCol=3;
	}

	if(x==5)
	{
		startRow=3;
		startCol=6;
	}

	if(x==6)
	{
		startRow=6;
		startCol=0;
	}

	if(x==7)
	{
		startRow=6;
		startCol=3;
	}

	if(x==8)
	{
		startRow=6;
		startCol=6;
	}

    	int saved[10] = {0};

    for (int i = startRow; i < startRow + 3; ++i) {
        for ( int j = startCol; j < startCol + 3; ++j) {
            int val = data->arr1[i][j];
            if (saved[val] != 0) {
		valid=0;
            }
            else{
                saved[val] = 1;
			
            }
        }
	
    }
		
	if(valid==1)
	{
		colm[x]=1;
	}

	valid=1;
	
  }
    pthread_cond_signal(&condA);
    return colm;
}


//main program
int main(int argc, char* argv[])
{
	int i,j;
	int arr1[9][9];
	int mdelay=atoi(argv[2]);

	//generate unique random number between 1-5 for each execution
	srand(time(NULL));
	int random_number = rand() % mdelay +1;

	pthread_mutex_init(&lock, NULL);	

	//opening file to get inputs
	FILE *file = fopen(argv[1], "r");
	if (file == 0)
	{
		printf("Error opening input file!\n");
		exit(1);
	}	

	//reading input from file
		char * line = NULL;
    		size_t len = 0;
    		ssize_t read;
		int rows = 0;

   	 while ((read = getline(&line, &len, file)) != -1){
         int *a = arr1[rows];

        	 if(9 != sscanf(line, "%d%d%d%d%d%d%d%d%d", a, a+1, a+2,a+3,a+4,a+5,a+6,a+7,a+8)){
         	   fprintf(stderr, "%s invalid format at input file\n", line);
            	   return EXIT_FAILURE;
        	 }
 	       ++rows;
    	 }

	fclose(file);
   
	free(line);	

	FILE *f = fopen("threads_log_file.txt", "w");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}  

	fclose(f);

	//create parameters for rows to passed to the method
	parameters * param1 = (parameters *) malloc(sizeof(parameters));
 	param1->row = 0;
	param1->col = 0;
    	param1->arr1 = arr1;

	parameters * param2 = (parameters *) malloc(sizeof(parameters));
        param2->row = 1;
        param2->col = 0;
        param2->arr1 = arr1;

	parameters * param3 = (parameters *) malloc(sizeof(parameters));
        param3->row = 2;
        param3->col = 0;
        param3->arr1 = arr1;

	parameters * param4 = (parameters *) malloc(sizeof(parameters));
        param4->row = 3;
        param4->col = 0;
        param4->arr1 = arr1;

	parameters * param5 = (parameters *) malloc(sizeof(parameters));
        param5->row = 4;
        param5->col = 0;
        param5->arr1 = arr1;

	parameters * param6 = (parameters *) malloc(sizeof(parameters));
        param6->row = 5;
        param6->col = 0;
        param6->arr1 = arr1;

	parameters * param7 = (parameters *) malloc(sizeof(parameters));
        param7->row = 6;
        param7->col = 0;
        param7->arr1 = arr1;

	parameters * param8 = (parameters *) malloc(sizeof(parameters));
        param8->row = 7;
        param8->col = 0;
        param8->arr1 = arr1;

	parameters * param9 = (parameters *) malloc(sizeof(parameters));
        param9->row = 8;
        param9->col = 0;
        param9->arr1 = arr1;
	
	//create parameter to check columns
	parameters * param10 = (parameters *) malloc(sizeof(parameters));
        param10->row = 0;
        param10->col = 0;
        param10->arr1 = arr1;

	//create parameters to check 3x3 squares using a thread
  	parameters * param11 = (parameters *) malloc(sizeof(parameters));
    	param11->row = 0;
    	param11->col = 0;
    	param11->arr1 = arr1;
    
	pthread_t thread_1, thread_2, thread_3,thread_4, thread_5, thread_6, thread_7,thread_8, thread_9, thread_10, thread_11;

	void * row1;
	void * row2;
	void * row3;
	void * row4;
	void * row5;
	void * row6;
	void * row7;
	void * row8;
	void * row9;
	void * square1;
	void * cols;

	//creating threads and passing parameters to the methods

	//check row validity
	pthread_create(&thread_1, NULL, row_check, (void *) param1);
	sleep(random_number);

	pthread_create(&thread_2, NULL, row_check, (void *) param2);
	sleep(random_number);

	pthread_create(&thread_3, NULL, row_check, (void *) param3);
	sleep(random_number);

	pthread_create(&thread_4, NULL, row_check, (void *) param4);
	sleep(random_number);
	
	pthread_create(&thread_5, NULL, row_check, (void *) param5);
	sleep(random_number);

	pthread_create(&thread_6, NULL, row_check, (void *) param6);
	sleep(random_number);

	pthread_create(&thread_7, NULL, row_check, (void *) param7);
	sleep(random_number);

	pthread_create(&thread_8, NULL, row_check, (void *) param8);
	sleep(random_number);

	pthread_create(&thread_9, NULL, row_check, (void *) param9);
	sleep(random_number);

	//check column validity
	pthread_create(&thread_10, NULL, col_check, (void *) param10);
	sleep(random_number);

	//using a single thread to check all 3x3 squares
	pthread_create(&thread_11, NULL, check_square, (void *) param11);

	//block parent task
	pthread_cond_wait(&condA, &mutex);
	sleep(random_number);

	//wait for threads
	pthread_join(thread_1, &row1);
	pthread_join(thread_2, &row2);
	pthread_join(thread_3, &row3);
	pthread_join(thread_4, &row4);
	pthread_join(thread_5, &row5);
	pthread_join(thread_6, &row6);
	pthread_join(thread_7, &row7);
	pthread_join(thread_8, &row8);
	pthread_join(thread_9, &row9);
	pthread_join(thread_10, &cols);
	pthread_join(thread_11, &square1);
	
	
	f = fopen("threads_log_file.txt", "a");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}


	//check validity of 3x3 sub-grids through the return values

	int squares=0;

	if ( ((int*)square1)[0]==1){

		squares++;
		counter++;
	
	}
	else{
		 fprintf(f, "Validation result from thread id: %u. :sub-grid [1..3, 1..3] is invalid\n", (unsigned int)thread_11, NULL);
	}

	if ( ((int*)square1)[1]==1){

                squares++;
                counter++;

        }
        else{
                 fprintf(f, "Validation result from thread id: %u. :sub-grid [1..3, 4..6] is invalid\n", (unsigned int)thread_11, NULL);
        }

	if ( ((int*)square1)[2]==1){

                squares++;
                counter++;

        }
        else{
                fprintf(f, "Validation result from thread id: %u. :sub-grid [1..3, 7..9] is invalid\n", (unsigned int)thread_11, NULL);
        }

	if ( ((int*)square1)[3]==1){

                squares++;
                counter++;

        }
        else{
                fprintf(f, "Validation result from thread id: %u. :sub-grid [4..6, 1..3] is invalid\n", (unsigned int)thread_11, NULL);
        }

	if ( ((int*)square1)[4]==1){

                squares++;
                counter++;

        }
        else{
               fprintf(f, "Validation result from thread id: %u. :sub-grid [4..6, 4..6] is invalid\n", (unsigned int)thread_11, NULL);
        }

	if (((int*)square1)[5]==1){

                squares++;
                counter++;

        }
        else{
               fprintf(f, "Validation result from thread id: %u. :sub-grid [4..6, 7..9] is invalid\n", (unsigned int)thread_11, NULL);
        }
	
	if ( ((int*)square1)[6]==1){

                squares++;
                counter++;

        }
        else{
               fprintf(f, "Validation result from thread id: %u. :sub-grid [7..9, 1..3] is invalid\n", (unsigned int)thread_11, NULL);
        }

	if ( ((int*)square1)[7]==1){

                squares++;
                counter++;

        }
        else{
               fprintf(f, "Validation result from thread id: %u. :sub-grid [7..9, 4..6] is invalid\n", (unsigned int)thread_11, NULL);
        }

	if ( ((int*)square1)[8]==1){

                squares++;
                counter++;

        }
        else{
              fprintf(f, "Validation result from thread id: %u. :sub-grid [7..9, 7..9] is invalid\n", (unsigned int)thread_11, NULL);
        }

	printf("Validation result from thread id: %u. :%d out of 9 3x3 Squares are valid\n", (unsigned int)thread_11, squares);
	sleep(random_number);

	//terminate threads
	pthread_cancel(thread_1);
	pthread_cancel(thread_2);
	pthread_cancel(thread_3);
	pthread_cancel(thread_4);
	pthread_cancel(thread_5);
	pthread_cancel(thread_6);
	pthread_cancel(thread_7);
	pthread_cancel(thread_8);
	pthread_cancel(thread_9);
	pthread_cancel(thread_11);
	pthread_cancel(thread_10);


	//freeing memory allocations
	free(param1);
	free(param2);
	free(param3);
	free(param4);
	free(param5);
	free(param6);
	free(param7);
	free(param8);
	free(param9);	
	free(param10);
	free(param11);

	pthread_mutex_destroy(&lock);

	
	//check if the sudoku solution is valid through the counter
	if(counter==27)
	{
		printf("\nThere are %d valid sub-grids, and thus the solution is valid\n", counter);
	}
	else		
		printf("\nThere are %d valid sub-grids, and thus the solution is invalid\n", counter);


	fclose(f);

return 0;

}
