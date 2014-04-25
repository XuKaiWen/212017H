/* 编写一个测试点对点通信的程序 pingpong.c,其中由进程 0 发送一个指定长
   度的消息给进程 1,进程 1 收到消息后再把它发回给进程 0,重复上述过程指定的
   次数,统计每次消息往返所花费的平均时间。 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>

/* 2^23 B = 8 MB */
#define N 24
/* 通讯次数 */
#define M 1

/*
  得到传输 size 大小的数据块的平均时间。
  size, 数据块大小，1 byte 的倍数
  time, 得到的平均时间
*/
void pingpong(int size, double *time, int myrank)
{
  int i;
  struct timeval st, et;
  void *buff;
  MPI_Status status;

  /* 根据size大小开辟内存空间 */
  /* 以 byte 为单位 */
  buff = malloc((size_t) size);

  /* 消息发送部分，计算时间 */
  if (myrank == 0)
    {
      gettimeofday(&st,NULL);

      for ( i = 0; i < M; ++i)
        {
          MPI_Send(buff, size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
          MPI_Recv(buff, size, MPI_BYTE, 1, 0, MPI_COMM_WORLD, &status);
        }

      gettimeofday(&et,NULL);
      /* 计算平均时间 */
      /* avg_time = elps_time/M; */
      *time = (et.tv_sec+et.tv_usec*1e-6-st.tv_sec-st.tv_usec*1e-6)/M;
      *time *= 1000;		/* 这样单位是 ms */
    }
  else if (myrank == 1)
    {
      for ( i = 0; i < M; ++i)
        {
          MPI_Recv(buff, size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
          MPI_Send(buff, size, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
        }
    }

  free(buff);
  return;
}


/*
  输出
*/
void output(double *time)
{
  int i,j, size;
  /* len 每一行的个数 */
  int len = 7;

  printf("\n------------------------------------------------------------------------\n");
  printf("通信次数 %d  时间单位 (ms)\n", M);

  j=0;
  size = 1;
  do
    {
      j += len;

      printf("------------------------------------------------------------------------\n");
      printf("消息长度 ");
      for ( i = j-len; i<N && i<j; ++i)
        {
          if (i < 10)
            printf(" %6dB", size);
          else if (i < 20)
            printf(" %5dKB", size/1024);
          else
            printf(" %5dMB", size/(1024*1024));

          fflush(stdout);
          size *= 2;
        }
      printf("\n");

      printf("------------------------------------------------------------------------\n");
      printf("返回时间 ");
      for ( i = j-len; i<N && i<j; ++i)
        printf("%8.4f", time[i]);
      printf("\n");
      printf("------------------------------------------------------------------------\n");
    } while (j < N);

  return;
}

int main(int argc, char *argv[])
{
  int i, size;
  int myrank;
  double time[N];
  /* 时间统一的由 0 进程来计算。 */

  /* 初始化 mpi */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  /* 计算平均用时 */
  /* 这个循环每个进程都要运行，具体的运行方式不同。 */
  size = 1;
  for ( i = 0; i < N; ++i)
    {
      /* pingpong(size, &time[i]， myrank， comm); */
      pingpong(size, &time[i], myrank);
      size *= 2;
    }

  /* 输出结果 */
  if (myrank == 0)
    {
      output(time);
    }

  MPI_Finalize();
  return 0;
}


