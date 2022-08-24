# include <iostream>
# include <mpi.h>
# include <cmath>

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


int* subset_calculation(int n, int q, int* sub_mat, int** mat){
    int ans = 0;
    int* row_arr = (int*)malloc(sizeof(int) * q);
    for (int i = 0; i < q; i++){
        ans = 0;
        for (int j = 0; j < n; j++){
            ans += (sub_mat[j] * mat[j][i]);
        }
        row_arr[i] = ans;
    }
    return row_arr;
}

int NumRows_to_Proc(int N, int j, int P){
    return (floor(N * (j + 1)/P) - floor(N * j/P));
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
        int** Matrix_C = NULL;

        int* matrix_rows_subset = (int*)malloc(sizeof(int) * n);
        int* subset_ans = (int*) malloc(sizeof(q));
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
            //Changes made in mac
            
            subset_ans = subset_calculation(n, q, matrix_rows_subset, Matrix_B);
            Matrix_C = (int**)malloc(sizeof(int) * m);
            for(int i = 0; i < m; i++){
                *(Matrix_C + i) = (int*)malloc(sizeof(int) * q);	  
            }
            *(Matrix_C + 0) = subset_ans;
            
            for (int i = 1; i < m; i++){
                MPI_Recv(*(Matrix_C + i), q, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            
            //print_matrix(m,q,Matrix_C);
        }
        else if((rank < usable_proc) ){   // can i add additional parameter as "&& rank != 0"" ??
            MPI_Recv(matrix_rows_subset, n, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            /*for(int i = 0; i < n; i++){
                cout << matrix_rows_subset[i] << " ";
            }
            cout << "\n";*/

            for (int j = 0; j < p; j++){
                Matrix_B[j] = (int*)malloc(sizeof(int) * q);
                MPI_Recv(*(Matrix_B + j), q, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            ////MPI_Recv(&(Matrix_B), p*q, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            /*print_matrix(p,q,Matrix_B);
            cout << "\n";*/
            // Changes in mac
            int* subset_ans = subset_calculation(n, q, matrix_rows_subset, Matrix_B);
            MPI_Send(subset_ans, q, MPI_INT, 0, 3, MPI_COMM_WORLD);
        }
            
        MPI_Barrier(MPI_COMM_WORLD);
        if(Matrix_C != NULL)
            print_matrix(m, q, Matrix_C);
    }
    else if(num_procs < m){
        int row_count = 0;
        int proc_map[num_procs];
        int flag = 0;
        
        //int extra_rows = m - num_procs;
        
        int* matrix_rows_subset = (int*)malloc(sizeof(int) * n);
        int* subset_ans = NULL;
        int** Matrix_C = NULL;
        
        
        
        
        int num = 0;
        if (rank == 0){
            for (int i = 1; i < num_procs; i++){
                for(int j = 0; j < p; j++){
                    flag = MPI_Send(*(Matrix_B + j), q, MPI_INT, i, 200, MPI_COMM_WORLD);
                    //cout << "First send flag " << flag << endl; //flag = 0
                }
            }
            
            for(int i = 0; i < num_procs; i++){
                proc_map[i] = NumRows_to_Proc(m, i, num_procs);
            }
            
            int local = proc_map[0];
            row_count = local;
            
            for (int i = local; i < num_procs; i++){
                for (int j = 0; j < proc_map[i]; j++){
                    flag = MPI_Send(*(Matrix_A + row_count), n, MPI_INT, i, row_count, MPI_COMM_WORLD);
                    row_count++;
                    //cout << "Second send flag "<< flag << endl; //flag = 0
                }
            }

            
            Matrix_C = (int**)malloc(sizeof(int) * m);
            for(int i = 0; i < m; i++){
                *(Matrix_C + i) = (int*)malloc(sizeof(int) * q);	  
            }
            
            
            for(int i = 0; i < local; i++){
                *(Matrix_C + i) = subset_calculation(n, q, *(Matrix_A + i), Matrix_B);
            }
            //print_matrix(m,q,Matrix_C);  //PErfectly printing for local value
            
            for(int j = local; j < m; j++){
                cout << "DEBUG " << flag << endl;
                flag = MPI_Recv(*(Matrix_C + j), q, MPI_INT, 0, j, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                cout << "Final recv " << flag << endl;
            }
            //cout << "DEBUG\n"; //Not working
            //print_matrix(m,q,Matrix_C);
        }
        else if (rank > 0){
            
            for (int j = 0; j < p; j++){
                Matrix_B[j] = (int*)malloc(sizeof(int) * q);
                MPI_Recv(*(Matrix_B + j), q, MPI_INT, 0, 200, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            //print_matrix(p,q, Matrix_B);
            
            MPI_Status status;
            int tag_tracker[m];
            int** temp_buffer = (int**)malloc(sizeof(int*) * m);
            int** subset_ans_buffer = (int**)malloc(sizeof(int*) * m);
            int total_rows = 0;
            for (int i = 0; i < m; i++) tag_tracker[i] = -1;  
            
            //cout << "DEBUG\n";
            int k = 0;
            while(k < 2){
                *(temp_buffer + total_rows) = (int*)malloc(sizeof(int) * n);
                flag = MPI_Recv(*(temp_buffer + total_rows), n, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                /*if (rank == 1){
                    for (int j = 0; j < n; j++){
                        cout << *(*(temp_buffer + k) + j) << " ";
                    }
                    cout << " row " << k << "\n";
                }*/
                //cout << "Rank > 0, Recv flag " << flag << endl ;
                *(subset_ans_buffer + total_rows) = (int*)malloc(sizeof(int) * q);
                *(subset_ans_buffer + total_rows) = subset_calculation(n, q, *(temp_buffer + total_rows), Matrix_B);
                tag_tracker[total_rows++] = status.MPI_TAG;
                k++;
            }
            //cout << "DEBUG\n"; // After changing k value, it is running
            
            //trying to print buffer
            /*for (int i = 0; i < 2; i++){
                for (int j = 0; j < q; j++){
                    cout << *(*(subset_ans_buffer + i) + j) << " ";
                }
                cout << endl;
            }*/
            
            for (int i = 0; i < total_rows; i++){
                flag = MPI_Send(*(subset_ans_buffer + i), q, MPI_INT, 0, tag_tracker[i], MPI_COMM_WORLD);
                //cout << "Send flag " << flag << endl;
            }
            //cout << "Working well upto here\n";
        }
    }
    
    MPI_Finalize();
    
    return 0;
}

































