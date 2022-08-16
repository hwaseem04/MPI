# include <iostream>
# include <mpi.h>

using namespace std;

float* create_nums(int num_elements){	
	float *arr = (float*)malloc(sizeof(float) * num_elements);
	for(int i = 0; i < num_elements; i++){
		arr[i] = i + 1;
	}
	return arr;
}	

float compute_avg(float* arr, int elems_per_process){
	float sum = 0;
	for (int i = 0; i < elems_per_process; i++){
		sum += arr[i];
	}
	//cout << sum << endl;
	return sum/elems_per_process;
}

		
int main(int argc, char** argv){

	int rank, num_procs;
	int elements_per_proc = 16/atoi(argv[1]);
	MPI_Init(NULL,NULL);

	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	//element_per_proc = 16/4;
	float* nums = NULL;
	if (rank == 0){
		nums = create_nums(elements_per_proc * num_procs);
	}

	float *subset_nums = (float*)malloc(sizeof(float) * elements_per_proc);

	MPI_Scatter(nums, elements_per_proc, MPI_FLOAT, subset_nums, elements_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

	float avg = compute_avg(subset_nums, elements_per_proc);
	float *avgs = NULL;	
	if (rank == 0){
		avgs = (float*)malloc(sizeof(float) * num_procs);
	}
	
	MPI_Gather(&avg, 1, MPI_FLOAT, avgs,1 , MPI_FLOAT, 0, MPI_COMM_WORLD);
	//MPI_Gather(&avg, 1, MPI_FLOAT, avgs, num_procs, MPI_FLOAT, 0, MPI_COMM_WORLD);
	
	if (rank == 0){
		float total = compute_avg(nums,16);
		cout <<"Without : " << total << "\n";
		float total_avg = compute_avg(avgs, num_procs);
		cout <<"With parallel Processing : "<< total_avg << "\n";
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();	
	return 0;
}



