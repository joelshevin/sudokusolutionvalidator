#define _gnu_source
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
//#include<ncurses.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include<sys/stat.h>
#include<time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h> 

typedef struct 
{
    // This structure is used to pass the starting row,column data to the child threads

    int row;
    int col;
    int (* arr1)[9];
} parameters;

static int *counter;
sem_t *sem;

//this method is used to check the validity of rows by the child threads
/*check each row and return 1 if valid and 0 if invalid*/
void * row_check(void * params) {
	
    int pd=getpid();
    
    //open file to write invalid rows
	FILE *f = fopen("process_log_file.txt", "a");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}   
    sem_wait(sem);	
    parameters * data = (parameters *) params;
    int startRow = data->row;
    int startCol = data->col;   
    for (int i = startRow; i <= startRow; ++i) {
        int row[10] = {0};
        for (int j = startCol; j < 9; ++j) {
            int val = data->arr1[i][j];
	    
            if (row[val] != 0) {
		   printf("Validation result from process id: %d. :row %d is invalid\n", pd,startRow+1);
		   fprintf(f, "Validation result from process id: %d. :row %d is invalid\n", pd,startRow+1);
		    sem_post(sem); 
	           return (void *) 0;
            }
            else{
                row[val] = 1;
            }
	    
        }
    }	
    printf("Validation result from process id: %d. :row %d is valid\n",pd,startRow+1);
    *counter=*counter+1;
    fclose(f);
    sem_post (sem); 
    return (void *) 1;

}

//this method is used to check the validity of columns by the child threads
/*checks all columns and stores the number of valid columns in the variable ‘validColumns’*/
void * col_check(void * params) {
    parameters * data = (parameters *) params;
    int startRow = data->row;
    int startCol = data->col;
    int validColumns=9;
    int pd=getpid();

    //open file to append invalid 3x3 squares
	FILE *f = fopen("process_log_file.txt", "a");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}
    sem_wait(sem);
    for (int i = startCol; i < 9; ++i) {
        int col[10] = {0};
        for (int j = startRow; j < 9; ++j) {
            int val = data->arr1[j][i];
	    parameters * data = (parameters *) params;
            if (col[val] != 0) {
		validColumns=validColumns-1;
		fprintf(f, "Validation result from process id: %d. :Column %d is invalid\n", pd,startCol+1);
          	break;
	    }
            else{
                col[val] = 1;
		

            }
	    
        }
    }
    *counter=*counter+validColumns;
    printf("Validation result from process id: %d. :%d out of 9 Columns is valid\n",pd,validColumns);
    fclose(f);
    sem_post (sem);
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
    
    return colm;
}


int main(int argc, char* argv[])
{
	int i,j;
	int arr1[9][9];
	int mdelay=atoi(argv[2]); 	

	unsigned int semv;
	key_t shmkey; 
	shmkey = ftok ("/dev/null", 5);
	int shmid = shmget(shmkey, sizeof (int), 0644 | IPC_CREAT);
	counter=(int *) shmat (shmid, NULL, 0);
	*counter=0;

	sem = sem_open("pSem", O_CREAT | O_EXCL, 0644, semv);
	//sem_unlink("pSem");

	//generate unique random number between 1-5 for each execution
	srand(time(NULL));
	int random_number = rand() % mdelay +1;

	//opening file to get inputs
	FILE *file = fopen(argv[1], "r");
	if (file == 0)
	{
		printf("Error opening input file!\n");
		exit(1);
	}	
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
	free(line); //freeing memory allocated for line	
	fclose(file);

	//empty the log file
	FILE *f = fopen("process_log_file.txt", "w");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}  

	fclose(f);

	//open file to append invalid 3x3 squares
	f = fopen("process_log_file.txt", "a");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}

	//create parameters for each row to be passed to the method
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

	//parameter to check column validity
	parameters * param10 = (parameters *) malloc(sizeof(parameters));
        param10->row = 0;
        param10->col = 0;
        param10->arr1 = arr1;
	
	//create parameter to check 3x3 squares
  	parameters * param11 = (parameters *) malloc(sizeof(parameters));
    	param11->row = 0;
    	param11->col = 0;
    	param11->arr1 = arr1;


	pid_t pids[11];

	void * row1;
	void * row2;
	void * row3;
	void * row4;
	void * row5;
	void * row6;
	void * row7;
	void * row8;
	void * row9;
	void * cols;
	void * square1;

	//creating child processes to check row validity

	if((pids[0] =fork())== 0)
	{
		row1 = row_check(param1);	
		exit(0);
	}
	
	sleep(random_number);
	
	if((pids[1]=fork()) == 0)
	{
		row2 = row_check(param2);
		exit(0);
	}
	sleep(random_number);

	if((pids[2]=fork()) == 0)
	{
		row3 = row_check(param3);
		exit(0);
	}
	sleep(random_number);

	if((pids[3]=fork()) == 0)
	{
		row4 = row_check(param4);	
		exit(0);
	}
	sleep(random_number);

	if((pids[4]=fork())==0)
	{
		row5 = row_check(param5);
		exit(0);
	}
	sleep(random_number);

	if((pids[5]=fork()) == 0)
	{
		row6 = row_check(param6);
		exit(0);
	}
	sleep(random_number);

	if((pids[6]=fork()) == 0)
	{
		row7 = row_check(param7);
		exit(0);
	}
	sleep(random_number);

	if((pids[7]=fork()) == 0)
	{
		row8 = row_check(param8);
		exit(0);
	}
	sleep(random_number);

	if((pids[8]=fork()) == 0)
	{
		row9 = row_check(param9);
		exit(0);
	}
	sleep(random_number);

	//check validity of columns
	if((pids[9]=fork()) == 0)
	{
		cols = col_check(param10);
		exit(0);
	}
	sleep(random_number);

	//check validity of 3x3 sub-grids	
	int squares=0;

	if((pids[10]=fork()) == 0)
	{
		square1 = check_square(param11);
		int pid11=getpid();
		int squares=0;

	if ( ((int*)square1)[0]==1){

		squares++;
		sem_wait(sem);
		*counter=*counter+1;
		sem_post(sem);

	
	}
	else{

		 fprintf(f, "Validation result from process id: %d. :sub-grid [1..3, 1..3] is invalid\n", pid11, NULL);
	}

	if ( ((int*)square1)[1]==1){

                squares++;
		sem_wait(sem);
                *counter=*counter+1;
		sem_post(sem);

        }
        else{

                 fprintf(f, "Validation result from process id: %d. :sub-grid [1..3, 4..6] is invalid\n", pid11, NULL);
        }

	if ( ((int*)square1)[2]==1){

                squares++;
		sem_wait(sem);
                *counter=*counter+1;
		sem_post(sem);

        }
        else{

                fprintf(f, "Validation result from process id: %d. :sub-grid [1..3, 7..9] is invalid\n", pid11, NULL);
        }

	if ( ((int*)square1)[3]==1){

                squares++;
		sem_wait(sem);
                *counter=*counter+1;
		sem_post(sem);

        }
        else{

                fprintf(f, "Validation result from process id: %d. :sub-grid [4..6, 1..3] is invalid\n", pid11, NULL);
        }

	if ( ((int*)square1)[4]==1){

                squares++;
		sem_wait(sem);
                *counter=*counter+1;
		sem_post(sem);

        }
        else{

               fprintf(f, "Validation result from process id: %d. :sub-grid [4..6, 4..6] is invalid\n", pid11, NULL);
        }

	if (((int*)square1)[5]==1){

                squares++;
		sem_wait(sem);
                *counter=*counter+1;
		sem_post(sem);
        }
        else{

               fprintf(f, "Validation result from process id: %d. :sub-grid [4..6, 7..9] is invalid\n", pid11, NULL);
        }
	
	if ( ((int*)square1)[6]==1){

                squares++;
		sem_wait(sem);
                *counter=*counter+1;
		sem_post(sem);

        }
        else{

               fprintf(f, "Validation result from process id: %d. :sub-grid [7..9, 1..3] is invalid\n", pid11, NULL);
        }

	if ( ((int*)square1)[7]==1){

                squares++;
		sem_wait(sem);
                *counter=*counter+1;
		sem_post(sem);

        }
        else{

               fprintf(f, "Validation result from process id: %d. :sub-grid [7..9, 4..6] is invalid\n", pid11, NULL);
        }

	if ( ((int*)square1)[8]==1){

                squares++;
		sem_wait(sem);
                *counter=*counter+1;
		sem_post(sem);

        }
        else{

              fprintf(f, "Validation result from process id: %d. :sub-grid [7..9, 7..9] is invalid\n", pid11, NULL);
        }

	printf("Validation result from process id: %d. :%d out of 9 3x3 Squares are valid\n", pid11, squares);


		exit(0);
	}
	sleep(random_number);
	
	//wait for termination of child processes
	int status;
	pid_t wpid;
	int n=11;
	while (n > 0) {
  		wpid = wait(&status);
  		--n;
	}

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
	
	//validate the sudoku solution
	if(*counter==27)
	{
		printf("\nThere are %d valid sub-grids, and thus the solution is valid\n", *counter);
	}
	else		
		printf("\nThere are %d valid sub-grids, and thus the solution is invalid\n", *counter);

	shmdt (counter);
        shmctl (shmid, IPC_RMID, 0);
	sem_close(sem);
 

	fclose(f);
return 0;

}
