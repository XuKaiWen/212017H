/* 编写一个环形通信程序ring.c。该程序中首先由进程0随机产生一个整数，然
   后将该整数发给进程1，进程1收到后将该数打印出来，然后再发送给进程2，依次
   类推，直到最后一个进程收到后再发回给进程0。进程0将所收到的数与原来的数
   进行比较，如果一样则打印出"ring test passed."，否则则打印出"ring test
   failed."。 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* 获取随机整数 */
void get_vol(int *vol)
{
  time_t t;
  srandom((unsigned int) time(&t));
  *vol = (int) random()%100;
}

int main(int argc, char *argv[])
{
  int myrank;
  int nprocs;
  int vol, recv_vol;
  MPI_Status status;
  MPI_Comm comm;

  /* 初始化mpi环境 */
  MPI_Init(&argc, &argv);
  MPI_Comm_dup(MPI_COMM_WORLD, &comm);

  // 获取当前进程在通信器MPI_COMM_WORLD中的进程号
  MPI_Comm_rank(comm, &myrank);
  MPI_Comm_size(comm, &nprocs);

  if (myrank == 0)
    {
      get_vol(&vol);
      fprintf (stderr,"proc %d: the number is = %d.\n",myrank, vol);

      MPI_Send(&vol, 1, MPI_INT, 1, 0, comm);
      MPI_Recv(&recv_vol, 1, MPI_INT, nprocs-1, 0, comm, &status);

    }
  else
    {
      MPI_Recv(&vol, 1, MPI_INT, myrank-1, 0, comm, &status);
      fprintf (stderr,"proc %d: the number is = %d.\n",myrank, vol);
      MPI_Send(&vol, 1, MPI_INT, (myrank+1)%nprocs, 0, comm);
    }

  MPI_Barrier(MPI_COMM_WORLD);
  if (myrank == 0){
    if (recv_vol == vol)
      fprintf (stderr,"proc %d: ring test passed.\n",myrank);
    else
      fprintf (stderr,"proc %d: ring test failed.\n",myrank);
  }

  MPI_Finalize();
  return 0;
}
