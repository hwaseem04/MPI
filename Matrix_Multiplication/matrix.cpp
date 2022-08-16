# include <iostream>
# include <mpi.h>
using namespace std;

int* create_matrix(int size1, int size2){
	int* mat = (int*) malloc(sizeof(int) * (size1 * size2));
	for (int i = 0; i < size1 * size2; i++){
		mat[i] = i + 1;
	}
	return mat;
}		


void print_matrix(int size1, int size2, int* arr){
	for (int i = 0; i < size1; i++){
        for (int j = 0; j < size2; j++){
            cout << arr[i * size2 + j] << " ";
        }
		cout << "\n";
    }
}


int* subset_calculation(int col_A, int row_B, int* sub_arr, int* arr){
	int ans = 0;
	int* row_arr = (int*)malloc(sizeof(int) * col_A);
	for (int i = 0; i < row_B; i++){
		ans = 0;
		for (int j = 0; j < col_A; j++){
			ans += (sub_arr[j] * arr[j * row_B + i]);
		}
		row_arr[i] = ans;
	}
	return row_arr;	
}	


int main(int argc, char** argv){
	int rank, num_procs;
		
	int m,n,p,q;
	m = atoi(argv[1]);
	n = atoi(argv[2]);

	p = atoi(argv[3]);
	q = atoi(argv[4]);

	if (n != p){
		cout << "Invalid maltrix Multiplication, Cols of A should be equal to Rows of B" << endl;
		return 1;
	}	

	//int* Matrix_A = create_matrix(m,n);
	//int* Matrix_B = create_matrix(p,q);

	MPI_Init(NULL, NULL);

	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int* Matrix_A = (int*)malloc(sizeof(int) * m * n);
	int* Matrix_B = (int*)malloc(sizeof(int) * p * q);

	if (rank == 0){
		Matrix_A = create_matrix(m,n);
		Matrix_B = create_matrix(p,q);
	}

	int* matrix_rows_subset = (int*)malloc(sizeof(int) * n);
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(Matrix_B, p*q, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Scatter(Matrix_A, n, MPI_INT, matrix_rows_subset, n, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	//print_matrix(p,q,Matrix_B);
	/*for (int i = 0; i < 4; i++)
		cout << matrix_rows_subset[i] << " " ;
	cout << "\n";*/
	int* sub_ans = subset_calculation(n, p, matrix_rows_subset, Matrix_B);
	
	/*for (int i = 0; i < n; i++)
        cout << sub_ans[i] << " " ;
    cout << "\n";*/

	int* Matrix_C = NULL;
	if (rank == 0){
		Matrix_C = (int*)malloc(sizeof(int) * m * q);
	}

	MPI_Gather(sub_ans, q, MPI_INT, Matrix_C, q, MPI_INT, 0, MPI_COMM_WORLD);	
	MPI_Barrier(MPI_COMM_WORLD);
	
	if (rank == 0){
		print_matrix(m,q,Matrix_C);
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();	
	return 0;
}















