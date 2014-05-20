/* MPI ����ʵ������ѭ���㷨ʵ�� Allgather
 *
 * ����������Ҫ p-1 ����ɣ�ÿ�����̷��� (p-1)*size ���ݣ����� (p-1)*size ����
 *
 * �÷�: mpirun -np 4 allgather 4m      (���ݿ��Сȡ 4MB)
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <mpi.h>

int
My_Allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype,
             void *recvbuf, int recvcount, MPI_Datatype recvtype,
             MPI_Comm comm)
{
  /* �ú�����ѧԱ��� */
  int i;
  int source, dest;
  int myrank, nprocs;
  int ext, lb, size;
  void *ptr;

  MPI_Comm_size(comm, &nprocs);
  MPI_Comm_rank(comm, &myrank);

  /* ע���Ƕ������ type����������Ҫ��ȡ type ����Ϣ */
  MPI_Type_get_extent(recvtype, &lb, &ext);
  size = recvcount * ext;

  for (i = 0; i < nprocs; ++i){
    source = (myrank-i+nprocs) % nprocs;
    dest = (myrank+i) % nprocs;
    /* �� source �����������ݴ洢�ڵ� source ��λ���ϡ�ѭ���ĵ�һ���Ǹ�
       �Լ������ݡ� */
    ptr = recvbuf + source*size;

    MPI_Sendrecv(sendbuf, sendcount, sendtype, dest, 0,
		 ptr, recvcount, recvtype, source, 0,
		 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

/*------------------------------------------------------------------------*/
#if 1
typedef unsigned char byte;

static void
check(int nprocs, int myrank, size_t size, byte *buffer)
/* ������: ���ݿ� j ��ֵӦ�õ��� j + 1 */
{
  size_t i, j;

  for (j = 0; j < nprocs; j++)
    for (i = 0; i < size; i++)
      if (buffer[j * size + i] != ((j + 1) & 255)) {
        fprintf(stderr, "Process %d: incorrect value at block %d, "
                "position %d\n", myrank, j, i);
        MPI_Abort(MPI_COMM_WORLD, 1);
      }
}

int
main(int argc, char **argv)
{
  int nprocs, myrank;
  byte *send_buffer, *recv_buffer;
  size_t size = 0 /* this makes gcc happy */ ;
  double time0, time1;

  /* ��ʼ�� MPI ���� */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  /* ��ȡ�����в��� */
  if (argc != 2) {
    if (myrank == 0)
      fprintf(stderr, "Usage:   %s buffersize[K|M|G]\n", argv[0]);
    /*MPI_Abort(MPI_COMM_WORLD, 1);*/
    MPI_Finalize();
    exit(1);
  }
  else {
    char *p;
    /* p is the position of the last char */
    size = strtol(argv[1], &p, 10);
    switch (toupper(*p)) {
    case 'G':
      size *= 1024;
    case 'M':
      size *= 1024;
    case 'K':
      size *= 1024;
      break;
      /* ������� case ������ break ��ô�ҵ�ƥ�����֮���һ�ε����к�
         ������� */
    }
  }
  if (size <= 0) {
    fprintf(stderr, "Process %d: invalid size %d\n", myrank, size);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  if (myrank == 0) {
    fprintf(stderr, "Allgather with %d processes, buffer size: %d\n",
            nprocs, size);
  }

  /* ���뷢�ͺͽ��ջ����� */
  send_buffer = malloc(size);
  recv_buffer = malloc(nprocs * size);
  if (send_buffer == NULL || recv_buffer == NULL) {
    fprintf(stderr, "Process %d: memory allocation error!\n", myrank);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  /* ���ͻ�������ֵ */
  /* fill memory with a constant byte */
  memset(send_buffer, myrank + 1, size);

  /* ���� My_Allgather() ���� */

  /* ������ջ����� */
  memset(recv_buffer, 0, nprocs * size);
  MPI_Barrier(MPI_COMM_WORLD);
  time0 = MPI_Wtime();
  My_Allgather(send_buffer, size, MPI_BYTE, recv_buffer, size, MPI_BYTE,
               MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  time1 = MPI_Wtime();
  if (myrank == 0)
    fprintf(stderr, "The circular algorithm: wall time = %lf\n",
            time1 - time0);
  check(nprocs, myrank, size, recv_buffer);

  /* ���� MPI_Allgather() ���� */

  /* ������ջ����� */
  memset(recv_buffer, 0, nprocs * size);
  MPI_Barrier(MPI_COMM_WORLD);
  time0 = MPI_Wtime();
  MPI_Allgather(send_buffer, size, MPI_BYTE, recv_buffer, size, MPI_BYTE,
                MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);
  time1 = MPI_Wtime();
  if (myrank == 0)
    fprintf(stderr, "MPI_Allgather: wall time = %lf\n", time1 - time0);
  check(nprocs, myrank, size, recv_buffer);

  MPI_Finalize();
  return 0;
}
#endif
