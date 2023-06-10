/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

#define BLOCK 8

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_32(int M, int N, int A[N][M], int B[M][N]);
void transpose_64(int M, int N, int A[N][M], int B[M][N]);
void transpose_6167(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == N) {
        if (M == 32)
            transpose_32(M, N, A, B);
        else
            transpose_64(M, N, A, B);
    }
    else 
        transpose_6167(M, N, A, B);
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

void transpose_32(int M, int N, int A[N][M], int B[M][N])
{
    int ii, jj, i, j;
    int temp;
    for (ii = 0; ii < M; ii += 8)
        for (jj = 0; jj < N; jj += 8) {
            for (i = ii; i < ii + 8; i++) {
                // pre-save the conflict entry in diagonal line
                if (ii == jj)
                    temp = A[i][i];

                for (j = jj; j < jj + 8; j++) {
                    if (i != j) B[j][i] = A[i][j];
                }
                if (ii == jj) {
                    B[i][i] = temp;
                }
            }
        }
    // M = N = 32 -> misses : 288
}

void transpose_64(int M, int N, int A[N][M], int B[M][N]) {
    int ii, jj, k;
    int a0, a1, a2, a3, a4, a5, a6, a7;
    for (ii = 0; ii < M; ii += 8)
        for (jj = 0; jj < N; jj += 8) {
            for (k = 0; k < 4; k++) {
                a0 = A[k + ii][jj + 0]; a1 = A[k + ii][jj + 1]; a2 = A[k + ii][jj + 2]; a3 = A[k + ii][jj + 3];
                a4 = A[k + ii][jj + 4]; a5 = A[k + ii][jj + 5]; a6 = A[k + ii][jj + 6]; a7 = A[k + ii][jj + 7];

                B[jj + 0][k + ii] = a0; B[jj + 1][k + ii] = a1; B[jj + 2][k + ii] = a2; B[jj + 3][k + ii] = a3;
                B[jj + 0][ii + k + 4] = a4; B[jj + 1][ii + k + 4] = a5; B[jj + 2][ii + k + 4] = a6; B[jj + 3][ii + k + 4] = a7;
            }
            for (k = 0; k < 4; k++) {
                // read a column from the 3rd 4x4 A sub-block
                a4 = A[ii + 4][jj + k]; a5 = A[ii + 5][jj + k]; a6 = A[ii + 6][jj + k]; a7 = A[ii + 7][jj + k];
                // save the corresponding row in the 1st B sub-block
                a0 = B[jj + k][ii + 4]; a1 = B[jj + k][ii + 5]; a2 = B[jj + k][ii + 6]; a3 = B[jj + k][ii + 7];
                // transpose the 3-rd sub-block to 1st B block
                B[jj + k][4 + ii] = a4; B[jj + k][5 + ii] = a5; B[jj + k][6 + ii] = a6; B[jj + k][7 + ii] = a7;
                // assign the saved column to the row in the 3rd B sub-block
                B[jj + k + 4][ii + 0] = a0; B[jj + k + 4][ii + 1] = a1; B[jj + k + 4][ii + 2] = a2; B[jj + k + 4][ii + 3] = a3;
            }
            for (k = 4; k < 8; k++) {
                // transpose the 4th sub-block
                a0 = A[ii + k][jj + 4]; a1 = A[ii + k][jj + 5]; a2 = A[ii + k][jj + 6]; a3 = A[ii + k][jj + 7];
                B[jj + 4][ii + k] = a0; B[jj + 5][ii + k] = a1; B[jj + 6][ii + k] = a2; B[jj + 7][ii + k] = a3;
            }
        }
}

void transpose_6167(int M, int N, int A[N][M], int B[M][N]) {
    int ii, jj, k;
    int a0, a1, a2, a3, a4, a5, a6, a7;

    for (ii = 0; ii < 64; ii += 8) {
        for (jj = 0; jj < 56; jj += 8) {
            for (k = ii; k < ii + 8; k++) {
                a0 = A[k][jj + 0];
                a1 = A[k][jj + 1];
                a2 = A[k][jj + 2];
                a3 = A[k][jj + 3];
                a4 = A[k][jj + 4];
                a5 = A[k][jj + 5];
                a6 = A[k][jj + 6];
                a7 = A[k][jj + 7];
                B[jj + 0][k] = a0;
                B[jj + 1][k] = a1;
                B[jj + 2][k] = a2;
                B[jj + 3][k] = a3;
                B[jj + 4][k] = a4;
                B[jj + 5][k] = a5;
                B[jj + 6][k] = a6;
                B[jj + 7][k] = a7;
            }
        }
    }
    for (jj = 0; jj < 56; jj += 8) {
        for (k = 64; k < 67; k++) {
            a0 = A[k][jj + 0];
            a1 = A[k][jj + 1];
            a2 = A[k][jj + 2];
            a3 = A[k][jj + 3];
            a4 = A[k][jj + 4];
            a5 = A[k][jj + 5];
            a6 = A[k][jj + 6];
            a7 = A[k][jj + 7];
            B[jj + 0][k] = a0;
            B[jj + 1][k] = a1;
            B[jj + 2][k] = a2;
            B[jj + 3][k] = a3;
            B[jj + 4][k] = a4;
            B[jj + 5][k] = a5;
            B[jj + 6][k] = a6;
            B[jj + 7][k] = a7;
        }
    }
    for (ii = 0; ii < 64; ii++) {
        // 56 -> 61
        a0 = A[ii][56];
        a1 = A[ii][57];
        a2 = A[ii][58];
        a3 = A[ii][59];
        a4 = A[ii][60];
        B[56][ii] = a0;
        B[57][ii] = a1;
        B[58][ii] = a2;
        B[59][ii] = a3;
        B[60][ii] = a4;
    }
    for (ii = 64; ii < 67; ii++) {
        for (jj = 56; jj < 61; jj++) {
            B[jj][ii] = A[ii][jj];
        }
    }
}

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */

char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

