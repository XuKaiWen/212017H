/* 将 mpisum.c 改为打印出最大值和包含最大值的进程号及数组中的位置。给出
不同进程数、不同数组长度的性能测试结果。(提示：用MPI_MAXLOC)。 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

int main( int argc, char *argv[] )
{
    int n, myid, numprocs, i;
    double *A, sum0, sum, t0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);

    if (argc != 2) {
	if (myid == 0)
	    fprintf(stderr, "Usage: %s n\n", argv[0]);
	/* n 是数组长度 */
	MPI_Finalize();
	exit(1);
    }

    n = atoi(argv[1]);
    assert(n > 0);		/* abort the program if assertion is
				   failse */
    if ((A = malloc(n * sizeof(*A))) == NULL) {
	fprintf(stderr, "%d: failed allocation.\n", myid);
	MPI_Abort(MPI_COMM_WORLD, 1);
	exit(1);
    }

    /* 产生随机数据 */
    srandom(getpid());
    for (i = 0; i < n; i++)
	A[i] = random() / (double)RAND_MAX;

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

    MPI_Finalize();

    return 0;
}
