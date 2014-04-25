 /* 修改hello world 程序，让每次屏幕打印输出结果均按进程号由小到大顺序输出。 */

#include <stdio.h>
#include <mpi.h>
int main(int argc, char * argv[])
{
  int myrank;
  int nprocs;
  int *buf;
  MPI_Status status, status1;
  int flag, count;

  // 初始化MPI环境
  MPI_Init(&argc, &argv);

  MPI_Comm comm;
  MPI_Comm_dup(MPI_COMM_WORLD, &comm);

  // 获取当前进程在通信器MPI_COMM_WORLD中的进程号
  MPI_Comm_rank(comm, &myrank);
  MPI_Comm_size(comm, &nprocs);

  if (myrank == 0)
    {
      printf("myrank = %d: Hello, world!\n", myrank);
      MPI_Send(buf, 0, MPI_INT, 1, 0, comm);
    }
  else if (myrank == nprocs-1)
    {
      /* MPI_Recv(buf, 0, MPI_INT, myrank-1, 0, comm, &status); */
      MPI_Probe(myrank-1, 0, comm, &status) ;
      printf("myrank = %d: Hello, world!\n", myrank);
    }
  else
    {
      /* MPI_Recv(buf, 0, MPI_INT, myrank-1, 0, comm, &status); */
      MPI_Probe(myrank-1, 0, comm, &status) ;
      printf("myrank = %d: Hello, world!\n", myrank);
      MPI_Send(buf, 0, MPI_INT, myrank+1, 0, comm);
    }
  
  MPI_Finalize();
  return 0;
}

