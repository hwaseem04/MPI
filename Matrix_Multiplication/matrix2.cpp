# include <iostream>
# include <mpi.h>
using namespace std;

int** create_matrix(int rows, int cols){
    int** mat = (int**) malloc(sizeof(int*) * rows);
    for (int i = 0; i < rows; i++){
        mat[i] = (int*)malloc(sizeof(int) * cols);
    }

	for (int i = 0; i < rows * cols; i++){
		*(*(mat + i/cols) + i%cols) = i + 1;	
	}
    return mat;
}

void print_matrix(int rows, int cols, int** mat){
    /*for (int i = 0; i < rows; i++){
        for (int j = 0; j < cols; j++){
            cout << mat[i * cols + j] << " ";
        }
        cout << "\n";
    }*/
	
	for (int i = 0; i < rows; i++){
		for (int j = 0; j < cols; j++){
			cout << *(*(mat + i) + j) << " ";
		}
		cout << "\n";
	}
	/*
	for (int i = 0; i < rows * cols; i++)
		cout << *(*(mat + i/cols) + i%cols) << " ";
	cout << "\n";*/
}


int* subset_calculation(int n, int q, int* sub_mat, int* mat){
    int ans = 0;
    int* row_arr = (int*)malloc(sizeof(int) * q);
    for (int i = 0; i < q; i++){
        ans = 0;
        for (int j = 0; j < n; j++){
            ans += (sub_mat[j] * mat[j * q + i]);
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

	
	MPI_Init(NULL, NULL);

	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	int** Matrix_A = (int**)malloc(sizeof(int*) * m); 
    int** Matrix_B = (int**)malloc(sizeof(int*) * p); 

	if (rank == 0){
        Matrix_A = create_matrix(m,n);
        Matrix_B = create_matrix(p,q);
		//print_matrix(m,n,Matrix_A);
    }
	

	if (num_procs > m){
		int usable_proc = num_procs - m;
		
		//MPI_Bcast(Matrix_B, p*q, MPI_INT, 0, MPI_COMM_WORLD);
		//MPI_Barrier(MPI_COMM_WORLD);	
		int* Matrix_C = NULL;

		int* matrix_rows_subset = (int*)malloc(sizeof(int) * n);
		if (rank == 0){
			for(int i = 1; i < usable_proc; i++){
				MPI_Send(*(Matrix_A + i), n, MPI_INT, i, 1, MPI_COMM_WORLD); 
			}
			matrix_rows_subset = *Matrix_A;
			/*for(int i = 0; i < n; i++){
                cout << matrix_rows_subset[i] << " ";
			}
			cout << "\n";*/
			for (int i = 1; i < usable_proc; i++){
				for(int j = 0; j < p; j++){
					MPI_Send(*(Matrix_B + j), q, MPI_INT, i, 2, MPI_COMM_WORLD);
				}
				//MPI_Send(&(Matrix_B), p*q, MPI_INT, i, 2, MPI_COMM_WORLD);
			}
		}
		else if((rank < usable_proc) ){
			MPI_Recv(matrix_rows_subset, n, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			/*for(int i = 0; i < n; i++){
				cout << matrix_rows_subset[i] << " ";
			}
			cout << "\n";*/

			for (int j = 0; j < p; j++){
				Matrix_B[j] = (int*)malloc(sizeof(int) * q);
				MPI_Recv(*(Matrix_B + j), q, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			//MPI_Recv(&(Matrix_B), p*q, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			print_matrix(p,q,Matrix_B);
			cout << "\n";
			//int* subset_ans = subset_calculation(n, q, matrix_rows_subset, Matrix_B);
			
		}
		//cout << "WORKING ??\n";
	}
	else if(num_procs < m){
		;
	}

	MPI_Finalize();
	
	return 0;
}			

































