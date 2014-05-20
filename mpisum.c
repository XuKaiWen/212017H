/* 将 mpisum.c 改为打印出最大值和包含最大值的进程号及数组中的位置。给出
   不同进程数、不同数组长度的性能测试结果。(提示：用MPI_MAXLOC)。 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

/* 每个进程数组的最大值，及所在位置 */
typedef struct{
  double value;
  int index;
  int rank;
} Mymax;

/* 自定义的规约运算 */
void
myCompr(void *inP, void *outP, int *len, MPI_Datatype *dptr)
{
  Mymax *in = (Mymax *)inP, *out = (Mymax *)outP;

  if (in->value > out->value)
    *out = *in;
}

int main( int argc, char *argv[] )
{
  int n, myid, numprocs, i;
  double *A, sum0, sum, t0;
  char *array_str;
  int wd = 6;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);

  if (argc != 2) {
    if (myid == 0){
      printf( "Usage: %s n\n", argv[0]);
      printf( "n , length of the array.\n");
    }
    /* n 是数组长度 */
    MPI_Finalize();
    exit(1);
  }

  n = atoi(argv[1]);
  assert(n > 0);                /* abort the program if assertion is
                                   failse */
  if ((A = malloc(n * sizeof(*A))) == NULL) {
    printf( "%d: failed allocation.\n", myid);
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(1);
  }
  array_str = malloc(n*wd + 1);

  /* 产生随机数据 */
  srandom(getpid());            /* getpid: get process
                                   indentification */
  for (i = 0; i < n; i++)
    A[i] = random() / (double)RAND_MAX;

  /* 输出每个进程的数组 */
  for ( i = 0; i < n; ++i){
    sprintf(&array_str[i*wd], "%6.2f", A[i]);
  }
  sprintf(&array_str[n*wd], "\n", A[i]);
  printf( "Array of proc %d: %s", myid, array_str);
  MPI_Barrier(MPI_COMM_WORLD);
  free(array_str);
  
#if 1
  /* 输出最大值，最大值所在的进程，在该数组中的位置 */
  Mymax in, out;
  MPI_Op myOpCmpr;
  int len;
  int root=0;

  in.rank = myid;
  in.value = 0.0;
  in.index = 0;
  /* 求每个进程的最大值及位置 */
  for ( i = 0; i < n; ++i){
    if (A[i] > in.value){
      in.value = A[i];
      in.index = i;
    }
  }

  /* 自定义一个规约运算，1: 运算是可交换的 */
  MPI_Op_create( myCompr, 1, &myOpCmpr);

  len = sizeof(Mymax);
  /* 开始计时 */
  t0 = MPI_Wtime();
  
  MPI_Reduce(&in, &out, len, MPI_BYTE, myOpCmpr, root, MPI_COMM_WORLD);

  if (myid == root){
    printf( "Maximum : %6.2f\n", out.value);
    printf( "Location: pid %d, index %d\n", out.rank, out.index);
    printf( "Time    : %g (s)\n", MPI_Wtime()-t0);
  }

  MPI_Op_free(&myOpCmpr);
#else
  /* 计算本地数组和 */
  sum0 = 0.0;
  for (i = 0; i < n; i++)
    sum0 += A[i];
  printf("Proc %d: n = %d, local sum = %g\n", myid, n, sum0);

  /* 开始计时 */
  t0 = MPI_Wtime();

  /* 计算全局和 */
  MPI_Reduce(&sum0, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  /* 打印最终结果及通信花费的墙钟时间 */
  if (myid == 0)
    printf("n = %d, sum = %g, time = %g (s)\n", n, sum, MPI_Wtime()-t0);
#endif

  MPI_Finalize();

  return 0;
}
