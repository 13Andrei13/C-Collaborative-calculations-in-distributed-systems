#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{

    int N = atoi(argv[1]);
    int error_status = atoi(argv[2]);
    int numtasks, rank, worker, dst;
    int msg = 0;
    int number_leg0 = 0;
    int number_leg1 = 0;
    int number_leg2 = 0;
    int number_elem0 = 0;
    int number_elem1 = 0;
    int number_elem2 = 0;
    int rest = 0;
    // vectorii in care pun particelele pentru fiecare
    int *v0_recv1 = NULL;
    int *v0_recv2 = NULL;
    // variabile dimensiune vectori taskuri
    int dim = 0;
    int *v_recv = NULL;
    //------
    int *v0 = NULL;
    int *v1 = NULL;
    int *v2 = NULL;
    int *v = NULL;
    int master = -1;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (error_status == 0)
    {
        if (rank == 0)
        {

            FILE *polFunctionFile = fopen("cluster0.txt", "rt");

            fscanf(polFunctionFile, "%d", &number_leg0);

            v0 = (int *)malloc(sizeof(int) * number_leg0);

            for (int i = 0; i < number_leg0; i++)
            {
                fscanf(polFunctionFile, "%d", &worker);
                v0[i] = worker;
                msg = rank;
                dst = worker;

                printf("M(%d,%d) \n", msg, dst);

                MPI_Send(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                MPI_Recv(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD, &status);
            }

            MPI_Recv(&number_leg2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            MPI_Recv(&number_leg1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

            MPI_Send(&number_leg0, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Send(&number_leg0, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);

            printf("M(%d,%d) \n", rank, 1);
            printf("M(%d,%d) \n", rank, 2);

            v1 = (int *)malloc(sizeof(int) * number_leg1);
            v2 = (int *)malloc(sizeof(int) * number_leg2);

            MPI_Recv(v1, number_leg1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

            MPI_Recv(v2, number_leg2, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            // afisare topologie
            printf("%d -> ", rank);
            for (int i = 0; i < 3; i++)
            {
                printf("%d:", i);
                if (i == 0)
                {
                    for (int j = 0; j < number_leg0; j++)
                    {
                        if (j == number_leg0 - 1)
                        {
                            printf("%d ", v0[j]);
                        }
                        else
                        {
                            printf("%d,", v0[j]);
                        }
                    }
                }
                else if (i == 1)
                {
                    for (int j = 0; j < number_leg1; j++)
                    {
                        if (j == number_leg1 - 1)
                        {
                            printf("%d ", v1[j]);
                        }
                        else
                        {
                            printf("%d,", v1[j]);
                        }
                    }
                }
                else if (i == 2)
                {
                    for (int j = 0; j < number_leg2; j++)
                    {
                        if (j == number_leg2 - 1)
                        {
                            printf("%d ", v2[j]);
                        }
                        else
                        {
                            printf("%d,", v2[j]);
                        }
                    }
                }
            }
            printf("\n");

            MPI_Send(v0, number_leg0, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 1);
            MPI_Send(v0, number_leg0, MPI_INT, 2, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 2);

            // trimitere topologie la workeri
            for (int i = 0; i < number_leg0; i++)
            {
                dst = v0[i];
                MPI_Send(&number_leg0, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg1, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg2, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);

                MPI_Send(v0, number_leg0, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v1, number_leg1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v2, number_leg2, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
            }

            // alocarea vectorului----------------------------------------------------

            v = (int *)malloc(sizeof(int) * N);
            worker = number_leg0 + number_leg1 + number_leg2;
            for (int i = 0; i < N; i++)
            {
                v[i] = i;
            }
            if (N / worker)
            {
                number_elem0 = (N / worker) * number_leg0;
                number_elem1 = (N / worker) * number_leg1;
                number_elem2 = (N / worker) * number_leg2;

                if (N % worker)
                {
                    rest = N % worker;
                    if (rest > number_leg0)
                    {
                        rest = rest - number_leg0;
                        number_elem0 += number_leg0;
                    }
                    else if (rest == number_leg0 || rest < number_leg0)
                    {
                        number_elem0 += rest;
                        rest = 0;
                    }

                    if (rest > number_leg1 && rest != 0)
                    {
                        rest = rest - number_leg1;
                        number_elem1 += number_leg1;
                    }
                    else if ((rest == number_leg1 || rest < number_leg1) && rest != 0)
                    {
                        number_elem1 += rest;
                        rest = 0;
                    }

                    if (rest > number_leg2 && rest != 0)
                    {
                        rest = rest - number_leg2;
                        number_elem2 += number_leg2;
                    }
                    else if ((rest == number_leg2 || rest < number_leg2) && rest != 0)
                    {
                        number_elem2 += rest;
                        rest = 0;
                    }
                }
            }
            else
            {

                rest = N % worker;
                if (rest > number_leg0)
                {
                    rest = rest - number_leg0;
                    number_elem0 += number_leg0;
                }
                else if (rest == number_leg0 || rest < number_leg0)
                {
                    number_elem0 += rest;
                    rest = 0;
                }

                if (rest > number_leg1 && rest != 0)
                {
                    rest = rest - number_leg1;
                    number_elem1 += number_leg1;
                }
                else if ((rest == number_leg1 || rest < number_leg1) && rest != 0)
                {
                    number_elem1 += rest;
                    rest = 0;
                }

                if (rest > number_leg2 && rest != 0)
                {
                    rest = rest - number_leg2;
                    number_elem2 += number_leg2;
                }
                else if ((rest == number_leg2 || rest < number_leg2) && rest != 0)
                {
                    number_elem2 += rest;
                    rest = 0;
                }
            }

            MPI_Send(&number_elem1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 1);
            MPI_Send(&number_elem2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 2);

            MPI_Send((v + number_elem0), number_elem1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 1);
            MPI_Send((v + number_elem0 + number_elem1), number_elem2, MPI_INT, 2, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 2);

            // trimitere valori------------------
            int indice = 0;
            for (int i = 0; i < number_leg0; i++)
            {
                int elemente_task = 0;
                if (i == number_leg0 - 1)
                {
                    elemente_task = number_elem0 / number_leg0 + number_elem0 % number_leg0;
                }
                else
                {
                    elemente_task = number_elem0 / number_leg0;
                }
                MPI_Send(&elemente_task, 1, MPI_INT, v0[i], 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, v0[i]);
                MPI_Send((v + indice), elemente_task, MPI_INT, v0[i], 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, v0[i]);

                indice += elemente_task;
            }

            // primire valori --------------
            indice = 0;
            for (int i = 0; i < number_leg0; i++)
            {
                int elemente_task = 0;
                if (i == number_leg0 - 1)
                {
                    elemente_task = number_elem0 / number_leg0 + number_elem0 % number_leg0;
                }
                else
                {
                    elemente_task = number_elem0 / number_leg0;
                }
                MPI_Recv((v + indice), elemente_task, MPI_INT, v0[i], 1, MPI_COMM_WORLD, &status);

                indice += elemente_task;
            }
            // primire valori de la clustere--------

            MPI_Recv((v + number_elem0), number_elem1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

            MPI_Recv((v + number_elem0 + number_elem1), number_elem2, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
            printf("M(%d,%d) \n", 1, 0);
            printf("M(%d,%d) \n", 2, 0);
            printf("Rezultat:");
            for (int i = 0; i < N; i++)
            {
                printf(" %d", v[i]);
            }
            printf("\n");
        }
        else if (rank == 1)
        {
            FILE *polFunctionFile = fopen("cluster1.txt", "rt");

            fscanf(polFunctionFile, "%d", &number_leg1);

            v1 = (int *)malloc(sizeof(int) * number_leg1);

            for (int i = 0; i < number_leg1; i++)
            {
                fscanf(polFunctionFile, "%d", &worker);
                v1[i] = worker;
                msg = rank;
                dst = worker;

                printf("M(%d,%d) \n", msg, dst);

                MPI_Send(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);

                MPI_Recv(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD, &status);
            }

            MPI_Recv(&number_leg2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            MPI_Send(&number_leg1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Send(&number_leg1, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);

            MPI_Recv(&number_leg0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

            printf("M(%d,%d) \n", rank, 0);
            printf("M(%d,%d) \n", rank, 2);

            MPI_Send(v1, number_leg1, MPI_INT, 0, 0, MPI_COMM_WORLD);

            v0 = (int *)malloc(sizeof(int) * number_leg0);
            v2 = (int *)malloc(sizeof(int) * number_leg2);

            MPI_Recv(v0, number_leg0, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(v2, number_leg2, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            printf("%d -> ", rank);
            for (int i = 0; i < 3; i++)
            {
                printf("%d:", i);
                if (i == 0)
                {
                    for (int j = 0; j < number_leg0; j++)
                    {
                        if (j == number_leg0 - 1)
                        {
                            printf("%d ", v0[j]);
                        }
                        else if (j == 0)
                        {
                            printf("%d,", v0[j]);
                        }
                    }
                }
                else if (i == 1)
                {
                    for (int j = 0; j < number_leg1; j++)
                    {
                        if (j == number_leg1 - 1)
                        {
                            printf("%d ", v1[j]);
                        }
                        else
                        {
                            printf("%d,", v1[j]);
                        }
                    }
                }
                else if (i == 2)
                {
                    for (int j = 0; j < number_leg2; j++)
                    {
                        if (j == number_leg2 - 1)
                        {
                            printf("%d ", v2[j]);
                        }
                        else
                        {
                            printf("%d,", v2[j]);
                        }
                    }
                }
            }
            printf("\n");
            MPI_Send(v1, number_leg1, MPI_INT, 2, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 2);

            for (int i = 0; i < number_leg1; i++)
            {
                dst = v1[i];
                MPI_Send(&number_leg0, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg1, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg2, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);

                MPI_Send(v0, number_leg0, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v1, number_leg1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v2, number_leg2, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
            }
            MPI_Recv(&number_elem1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

            v0_recv1 = (int *)malloc(sizeof(int) * number_elem1);
            MPI_Recv(v0_recv1, number_elem1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            // trimite valori--------
            int indice = 0;
            for (int i = 0; i < number_leg1; i++)
            {
                int elemente_task = 0;
                if (i == number_leg1 - 1)
                {
                    elemente_task = number_elem1 / number_leg1 + number_elem1 % number_leg1;
                }
                else
                {
                    elemente_task = number_elem1 / number_leg1;
                }
                MPI_Send(&elemente_task, 1, MPI_INT, v1[i], 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, v1[i]);
                MPI_Send((v0_recv1 + indice), elemente_task, MPI_INT, v1[i], 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, v1[i]);

                indice += elemente_task;
            }
            // primeste valorile--------------------------
            indice = 0;
            for (int i = 0; i < number_leg1; i++)
            {
                int elemente_task = 0;
                if (i == number_leg1 - 1)
                {
                    elemente_task = number_elem1 / number_leg1 + number_elem1 % number_leg1;
                }
                else
                {
                    elemente_task = number_elem1 / number_leg1;
                }
                MPI_Recv((v0_recv1 + indice), elemente_task, MPI_INT, v1[i], 1, MPI_COMM_WORLD, &status);

                indice += elemente_task;
            }

            // trimitere la 0

            MPI_Send(v0_recv1, number_elem1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
        else if (rank == 2)
        {
            FILE *polFunctionFile = fopen("cluster2.txt", "rt");

            fscanf(polFunctionFile, "%d", &number_leg2);

            v2 = (int *)malloc(sizeof(int) * number_leg2);

            for (int i = 0; i < number_leg2; i++)
            {
                fscanf(polFunctionFile, "%d", &worker);
                v2[i] = worker;
                msg = rank;
                dst = worker;

                printf("M(%d,%d) \n", msg, dst);

                MPI_Send(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                MPI_Recv(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD, &status);
            }
            MPI_Send(&number_leg2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Send(&number_leg2, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

            MPI_Recv(&number_leg1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

            MPI_Recv(&number_leg0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

            printf("M(%d,%d) \n", rank, 0);
            printf("M(%d,%d) \n", rank, 1);

            MPI_Send(v2, number_leg2, MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 0);
            MPI_Send(v2, number_leg2, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 1);

            v0 = (int *)malloc(sizeof(int) * number_leg0);
            v1 = (int *)malloc(sizeof(int) * number_leg1);

            MPI_Recv(v0, number_leg0, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(v1, number_leg1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

            printf("%d -> ", rank);
            for (int i = 0; i < 3; i++)
            {
                printf("%d:", i);
                if (i == 0)
                {
                    for (int j = 0; j < number_leg0; j++)
                    {
                        if (j == number_leg0 - 1)
                        {
                            printf("%d ", v0[j]);
                        }
                        else
                        {
                            printf("%d,", v0[j]);
                        }
                    }
                }
                else if (i == 1)
                {
                    for (int j = 0; j < number_leg1; j++)
                    {
                        if (j == number_leg1 - 1)
                        {
                            printf("%d ", v1[j]);
                        }
                        else
                        {
                            printf("%d,", v1[j]);
                        }
                    }
                }
                else if (i == 2)
                {
                    for (int j = 0; j < number_leg2; j++)
                    {
                        if (j == number_leg2 - 1)
                        {
                            printf("%d", v2[j]);
                        }
                        else
                        {
                            printf("%d,", v2[j]);
                        }
                    }
                }
            }
            printf("\n");

            // trimitere topologii la workeri-------------------------------------------------
            for (int i = 0; i < number_leg2; i++)
            {
                dst = v2[i];
                MPI_Send(&number_leg0, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg1, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg2, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);

                MPI_Send(v0, number_leg0, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v1, number_leg1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v2, number_leg2, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
            }

            MPI_Recv(&number_elem2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

            v0_recv2 = (int *)malloc(sizeof(int) * number_elem2);

            MPI_Recv(v0_recv2, number_elem2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            // trimite valori la workeri
            int indice = 0;
            for (int i = 0; i < number_leg2; i++)
            {
                int elemente_task = 0;
                if (i == number_leg2 - 1)
                {
                    elemente_task = number_elem2 / number_leg2 + number_elem2 % number_leg2;
                }
                else
                {
                    elemente_task = number_elem2 / number_leg2;
                }
                MPI_Send(&elemente_task, 1, MPI_INT, v2[i], 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, v2[i]);
                MPI_Send((v0_recv2 + indice), elemente_task, MPI_INT, v2[i], 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, v2[i]);

                indice += elemente_task;
            }
            // primeste valorile
            indice = 0;
            for (int i = 0; i < number_leg2; i++)
            {
                int elemente_task = 0;
                if (i == number_leg2 - 1)
                {
                    elemente_task = number_elem2 / number_leg2 + number_elem2 % number_leg2;
                }
                else
                {
                    elemente_task = number_elem2 / number_leg2;
                }
                MPI_Recv((v0_recv2 + indice), elemente_task, MPI_INT, v2[i], 1, MPI_COMM_WORLD, &status);

                indice += elemente_task;
            }
            // trimite la rank 0
            MPI_Send(v0_recv2, number_elem2, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Recv(&master, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            msg = 1;

            MPI_Send(&msg, 1, MPI_INT, master, 1, MPI_COMM_WORLD);

            printf("M(%d,%d)\n", rank, master);

            MPI_Recv(&number_leg0, 1, MPI_INT, master, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&number_leg1, 1, MPI_INT, master, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&number_leg2, 1, MPI_INT, master, 1, MPI_COMM_WORLD, &status);

            v0 = (int *)malloc(sizeof(int) * number_leg0);
            v1 = (int *)malloc(sizeof(int) * number_leg1);
            v2 = (int *)malloc(sizeof(int) * number_leg2);

            MPI_Recv(v0, number_leg0, MPI_INT, master, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(v1, number_leg1, MPI_INT, master, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(v2, number_leg2, MPI_INT, master, 1, MPI_COMM_WORLD, &status);

            printf("%d -> ", rank);
            for (int i = 0; i < 3; i++)
            {
                printf("%d:", i);
                if (i == 0)
                {
                    for (int j = 0; j < number_leg0; j++)
                    {
                        if (j == number_leg0 - 1)
                        {
                            printf("%d ", v0[j]);
                        }
                        else
                        {
                            printf("%d,", v0[j]);
                        }
                    }
                }
                else if (i == 1)
                {
                    for (int j = 0; j < number_leg1; j++)
                    {
                        if (j == number_leg1 - 1)
                        {
                            printf("%d ", v1[j]);
                        }
                        else
                        {
                            printf("%d,", v1[j]);
                        }
                    }
                }
                else if (i == 2)
                {
                    for (int j = 0; j < number_leg2; j++)
                    {
                        if (j == number_leg2 - 1)
                        {
                            printf("%d", v2[j]);
                        }
                        else
                        {
                            printf("%d,", v2[j]);
                        }
                    }
                }
            }
            printf("\n");

            // primire particica vector

            MPI_Recv(&dim, 1, MPI_INT, master, 1, MPI_COMM_WORLD, &status);

            v_recv = (int *)malloc(sizeof(int) * dim);
            MPI_Recv(v_recv, dim, MPI_INT, master, 1, MPI_COMM_WORLD, &status);

            for (int i = 0; i < dim; i++)
            {
                v_recv[i] = v_recv[i] * 2;
            }

            MPI_Send(v_recv, dim, MPI_INT, master, 1, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, master);

            printf("\n");
        }
    }
    else
    // BONUS---------------------------------------------------------------------------------
    {
        if (rank == 0)
        {

            FILE *polFunctionFile = fopen("cluster0.txt", "rt");

            fscanf(polFunctionFile, "%d", &number_leg0);

            v0 = (int *)malloc(sizeof(int) * number_leg0);

            for (int i = 0; i < number_leg0; i++)
            {
                fscanf(polFunctionFile, "%d", &worker);
                v0[i] = worker;
                msg = rank;
                dst = worker;

                printf("M(%d,%d) \n", msg, dst);

                MPI_Send(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                MPI_Recv(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD, &status);
            }

            MPI_Recv(&number_leg2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            MPI_Send(&number_leg0, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);

            MPI_Recv(&number_leg1, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            printf("M(%d,%d) \n", rank, 2);

            v1 = (int *)malloc(sizeof(int) * number_leg1);
            v2 = (int *)malloc(sizeof(int) * number_leg2);
            MPI_Recv(v2, number_leg2, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            MPI_Send(v0, number_leg0, MPI_INT, 2, 0, MPI_COMM_WORLD);

            MPI_Recv(v1, number_leg1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            printf("%d -> ", rank);
            for (int i = 0; i < 3; i++)
            {
                printf("%d:", i);
                if (i == 0)
                {
                    for (int j = 0; j < number_leg0; j++)
                    {
                        if (j == number_leg0 - 1)
                        {
                            printf("%d ", v0[j]);
                        }
                        else
                        {
                            printf("%d,", v0[j]);
                        }
                    }
                }
                else if (i == 1)
                {
                    for (int j = 0; j < number_leg1; j++)
                    {
                        if (j == number_leg1 - 1)
                        {
                            printf("%d ", v1[j]);
                        }
                        else
                        {
                            printf("%d,", v1[j]);
                        }
                    }
                }
                else if (i == 2)
                {
                    for (int j = 0; j < number_leg2; j++)
                    {
                        if (j == number_leg2 - 1)
                        {
                            printf("%d", v2[j]);
                        }
                        else
                        {
                            printf("%d,", v2[j]);
                        }
                    }
                }
            }
            printf("\n");

            // trimitere topologii la workeri --------------------
            for (int i = 0; i < number_leg0; i++)
            {
                dst = v0[i];
                MPI_Send(&number_leg0, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg1, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg2, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);

                MPI_Send(v0, number_leg0, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v1, number_leg1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v2, number_leg2, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
            }

            // alocarea vectorului----------------------------------------------------

            v = (int *)malloc(sizeof(int) * N);
            worker = number_leg0 + number_leg1 + number_leg2;
            for (int i = 0; i < N; i++)
            {
                v[i] = i;
            }
            if (N / worker)
            {
                number_elem0 = (N / worker) * number_leg0;
                number_elem1 = (N / worker) * number_leg1;
                number_elem2 = (N / worker) * number_leg2;

                if (N % worker)
                {
                    rest = N % worker;
                    if (rest > number_leg0)
                    {
                        rest = rest - number_leg0;
                        number_elem0 += number_leg0;
                    }
                    else if (rest == number_leg0 || rest < number_leg0)
                    {
                        number_elem0 += rest;
                        rest = 0;
                    }

                    if (rest > number_leg1 && rest != 0)
                    {
                        rest = rest - number_leg1;
                        number_elem1 += number_leg1;
                    }
                    else if ((rest == number_leg1 || rest < number_leg1) && rest != 0)
                    {
                        number_elem1 += rest;
                        rest = 0;
                    }

                    if (rest > number_leg2 && rest != 0)
                    {
                        rest = rest - number_leg2;
                        number_elem2 += number_leg2;
                    }
                    else if ((rest == number_leg2 || rest < number_leg2) && rest != 0)
                    {
                        number_elem2 += rest;
                        rest = 0;
                    }
                }
            }
            else
            {

                rest = N % worker;
                if (rest > number_leg0)
                {
                    rest = rest - number_leg0;
                    number_elem0 += number_leg0;
                }
                else if (rest == number_leg0 || rest < number_leg0)
                {
                    number_elem0 += rest;
                    rest = 0;
                }

                if (rest > number_leg1 && rest != 0)
                {
                    rest = rest - number_leg1;
                    number_elem1 += number_leg1;
                }
                else if ((rest == number_leg1 || rest < number_leg1) && rest != 0)
                {
                    number_elem1 += rest;
                    rest = 0;
                }

                if (rest > number_leg2 && rest != 0)
                {
                    rest = rest - number_leg2;
                    number_elem2 += number_leg2;
                }
                else if ((rest == number_leg2 || rest < number_leg2) && rest != 0)
                {
                    number_elem2 += rest;
                    rest = 0;
                }
            }

            MPI_Send(&number_elem1, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 2);
            MPI_Send(&number_elem2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 2);

            MPI_Send((v + number_elem0), number_elem1, MPI_INT, 2, 1, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 2);
            MPI_Send((v + number_elem0 + number_elem1), number_elem2, MPI_INT, 2, 2, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 2);

            // trimitere valori------------------
            int indice = 0;
            for (int i = 0; i < number_leg0; i++)
            {
                int elemente_task = 0;
                if (i == number_leg0 - 1)
                {
                    elemente_task = number_elem0 / number_leg0 + number_elem0 % number_leg0;
                    MPI_Send(&elemente_task, 1, MPI_INT, v0[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v0[1]);
                    MPI_Send((v + indice), elemente_task, MPI_INT, v0[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v0[1]);
                }
                else
                {
                    elemente_task = number_elem0 / number_leg0;
                    MPI_Send(&elemente_task, 1, MPI_INT, v0[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v0[1]);
                    MPI_Send((v + indice), elemente_task, MPI_INT, v0[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v0[1]);
                }

                indice += elemente_task;
            }

            // primire valori --------------
            indice = 0;
            for (int i = 0; i < number_leg0; i++)
            {
                int elemente_task = 0;
                if (i == number_leg0 - 1)
                {
                    elemente_task = number_elem0 / number_leg0 + number_elem0 % number_leg0;

                    MPI_Recv((v + indice), elemente_task, MPI_INT, v0[i], 1, MPI_COMM_WORLD, &status);
                }
                else
                {
                    elemente_task = number_elem0 / number_leg0;
                    MPI_Recv((v + indice), elemente_task, MPI_INT, v0[i], 1, MPI_COMM_WORLD, &status);
                }

                indice += elemente_task;
            }
            // primire valori de la clustere--------

            MPI_Recv((v + number_elem0 + number_elem1), number_elem2, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);

            MPI_Recv((v + number_elem0), number_elem1, MPI_INT, 2, 1, MPI_COMM_WORLD, &status);

            printf("Rezultat:");
            for (int i = 0; i < N; i++)
            {
                printf(" %d", v[i]);
            }
            printf("\n");
        }
        else if (rank == 1)
        {
            FILE *polFunctionFile = fopen("cluster1.txt", "rt");

            fscanf(polFunctionFile, "%d", &number_leg1);

            v1 = (int *)malloc(sizeof(int) * number_leg1);

            for (int i = 0; i < number_leg1; i++)
            {
                fscanf(polFunctionFile, "%d", &worker);
                v1[i] = worker;
                msg = rank;
                dst = worker;

                printf("M(%d,%d) \n", msg, dst);

                MPI_Send(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);

                MPI_Recv(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD, &status);
            }

            MPI_Recv(&number_leg2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            MPI_Send(&number_leg1, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);

            MPI_Recv(&number_leg0, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            printf("M(%d,%d) \n", rank, 2);

            v0 = (int *)malloc(sizeof(int) * number_leg0);
            v2 = (int *)malloc(sizeof(int) * number_leg2);

            MPI_Recv(v2, number_leg2, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            MPI_Send(v1, number_leg1, MPI_INT, 2, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 2);

            MPI_Recv(v0, number_leg0, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            printf("%d -> ", rank);
            for (int i = 0; i < 3; i++)
            {
                printf("%d:", i);
                if (i == 0)
                {
                    for (int j = 0; j < number_leg0; j++)
                    {
                        if (j == number_leg0 - 1)
                        {
                            printf("%d ", v0[j]);
                        }
                        else
                        {
                            printf("%d,", v0[j]);
                        }
                    }
                }
                else if (i == 1)
                {
                    for (int j = 0; j < number_leg1; j++)
                    {
                        if (j == number_leg1 - 1)
                        {
                            printf("%d ", v1[j]);
                        }
                        else
                        {
                            printf("%d,", v1[j]);
                        }
                    }
                }
                else if (i == 2)
                {
                    for (int j = 0; j < number_leg2; j++)
                    {
                        if (j == number_leg2 - 1)
                        {
                            printf("%d", v2[j]);
                        }
                        else
                        {
                            printf("%d,", v2[j]);
                        }
                    }
                }
            }
            printf("\n");

            // trimittere topologii la workeri---------------------------------------
            for (int i = 0; i < number_leg1; i++)
            {
                dst = v1[i];
                MPI_Send(&number_leg0, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg1, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg2, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);

                MPI_Send(v0, number_leg0, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v1, number_leg1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v2, number_leg2, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
            }
            // primire vectori
            MPI_Recv(&number_elem1, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            v0_recv1 = (int *)malloc(sizeof(int) * number_elem1);

            MPI_Recv(v0_recv1, number_elem1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

            // trimite valori--------
            int indice = 0;
            for (int i = 0; i < number_leg1; i++)
            {
                int elemente_task = 0;
                if (i == number_leg1 - 1)
                {
                    elemente_task = number_elem1 / number_leg1 + number_elem1 % number_leg1;
                    MPI_Send(&elemente_task, 1, MPI_INT, v1[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v1[i]);
                    MPI_Send((v0_recv1 + indice), elemente_task, MPI_INT, v1[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v1[i]);
                }
                else
                {
                    elemente_task = number_elem1 / number_leg1;
                    MPI_Send(&elemente_task, 1, MPI_INT, v1[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v1[i]);
                    MPI_Send((v0_recv1 + indice), elemente_task, MPI_INT, v1[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v1[i]);
                }

                indice += elemente_task;
            }
            // primeste valorile--------------------------
            indice = 0;
            for (int i = 0; i < number_leg1; i++)
            {
                int elemente_task = 0;
                if (i == number_leg1 - 1)
                {
                    elemente_task = number_elem1 / number_leg1 + number_elem1 % number_leg1;

                    MPI_Recv((v0_recv1 + indice), elemente_task, MPI_INT, v1[i], 1, MPI_COMM_WORLD, &status);
                }
                else
                {
                    elemente_task = number_elem1 / number_leg1;
                    MPI_Recv((v0_recv1 + indice), elemente_task, MPI_INT, v1[i], 1, MPI_COMM_WORLD, &status);
                }

                indice += elemente_task;
            }

            // trimitere la 1

            MPI_Send(v0_recv1, number_elem1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        }
        else if (rank == 2)
        {
            FILE *polFunctionFile = fopen("cluster2.txt", "rt");

            fscanf(polFunctionFile, "%d", &number_leg2);

            v2 = (int *)malloc(sizeof(int) * number_leg2);

            for (int i = 0; i < number_leg2; i++)
            {
                fscanf(polFunctionFile, "%d", &worker);
                v2[i] = worker;
                msg = rank;
                dst = worker;

                printf("M(%d,%d) \n", msg, dst);

                MPI_Send(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                MPI_Recv(&msg, 1, MPI_INT, dst, 1, MPI_COMM_WORLD, &status);
            }
            MPI_Send(&number_leg2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Send(&number_leg2, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

            MPI_Recv(&number_leg0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&number_leg1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

            MPI_Send(&number_leg1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 0);
            MPI_Send(&number_leg0, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 1);

            printf("M(%d,%d) \n", rank, 0);
            printf("M(%d,%d) \n", rank, 1);

            MPI_Send(v2, number_leg2, MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 0);
            MPI_Send(v2, number_leg2, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 1);

            v0 = (int *)malloc(sizeof(int) * number_leg0);
            v1 = (int *)malloc(sizeof(int) * number_leg1);

            MPI_Recv(v0, number_leg0, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(v1, number_leg1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

            MPI_Send(v0, number_leg0, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 1);
            MPI_Send(v1, number_leg1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 0);

            printf("%d -> ", rank);
            for (int i = 0; i < 3; i++)
            {
                printf("%d:", i);
                if (i == 0)
                {
                    for (int j = 0; j < number_leg0; j++)
                    {
                        if (j == number_leg0 - 1)
                        {
                            printf("%d ", v0[j]);
                        }
                        else
                        {
                            printf("%d,", v0[j]);
                        }
                    }
                }
                else if (i == 1)
                {
                    for (int j = 0; j < number_leg1; j++)
                    {
                        if (j == number_leg1 - 1)
                        {
                            printf("%d ", v1[j]);
                        }
                        else
                        {
                            printf("%d,", v1[j]);
                        }
                    }
                }
                else if (i == 2)
                {
                    for (int j = 0; j < number_leg2; j++)
                    {
                        if (j == number_leg2 - 1)
                        {
                            printf("%d", v2[j]);
                        }
                        else
                        {
                            printf("%d,", v2[j]);
                        }
                    }
                }
            }
            printf("\n");

            // trimitere topologii la workeri----------------------
            for (int i = 0; i < number_leg2; i++)
            {
                dst = v2[i];
                MPI_Send(&number_leg0, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg1, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(&number_leg2, 1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);

                MPI_Send(v0, number_leg0, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v1, number_leg1, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
                MPI_Send(v2, number_leg2, MPI_INT, dst, 1, MPI_COMM_WORLD);
                printf("M(%d,%d) \n", rank, dst);
            }

            MPI_Recv(&number_elem1, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&number_elem2, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);

            MPI_Send(&number_elem1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 1);

            v0_recv2 = (int *)malloc(sizeof(int) * number_elem2);
            v0_recv1 = (int *)malloc(sizeof(int) * number_elem1);

            MPI_Recv(v0_recv1, number_elem1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(v0_recv2, number_elem2, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);

            MPI_Send(v0_recv1, number_elem1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 1);

            // trimite valori la workeri
            int indice = 0;
            for (int i = 0; i < number_leg2; i++)
            {
                int elemente_task = 0;
                if (i == number_leg2 - 1)
                {
                    elemente_task = number_elem2 / number_leg2 + number_elem2 % number_leg2;
                    MPI_Send(&elemente_task, 1, MPI_INT, v2[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v2[i]);
                    MPI_Send((v0_recv2 + indice), elemente_task, MPI_INT, v2[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v2[i]);
                }
                else
                {
                    elemente_task = number_elem2 / number_leg2;
                    MPI_Send(&elemente_task, 1, MPI_INT, v2[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v2[i]);
                    MPI_Send((v0_recv2 + indice), elemente_task, MPI_INT, v2[i], 1, MPI_COMM_WORLD);
                    printf("M(%d,%d) \n", rank, v2[i]);
                }

                indice += elemente_task;
            }
            // primeste valorile
            indice = 0;
            for (int i = 0; i < number_leg2; i++)
            {
                int elemente_task = 0;
                if (i == number_leg2 - 1)
                {
                    elemente_task = number_elem2 / number_leg2 + number_elem2 % number_leg2;

                    MPI_Recv((v0_recv2 + indice), elemente_task, MPI_INT, v2[i], 1, MPI_COMM_WORLD, &status);
                }
                else
                {
                    elemente_task = number_elem2 / number_leg2;
                    MPI_Recv((v0_recv2 + indice), elemente_task, MPI_INT, v2[i], 1, MPI_COMM_WORLD, &status);
                }

                indice += elemente_task;
            }
            // trimite la rank 0
            MPI_Send(v0_recv2, number_elem2, MPI_INT, 0, 2, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, 0);

            MPI_Recv(v0_recv1, number_elem1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

            MPI_Send(v0_recv1, number_elem1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Recv(&master, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            msg = 1;

            MPI_Send(&msg, 1, MPI_INT, master, 1, MPI_COMM_WORLD);

            printf("M(%d,%d)\n", rank, master);

            MPI_Recv(&number_leg0, 1, MPI_INT, master, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&number_leg1, 1, MPI_INT, master, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(&number_leg2, 1, MPI_INT, master, 1, MPI_COMM_WORLD, &status);

            v0 = (int *)malloc(sizeof(int) * number_leg0);
            v1 = (int *)malloc(sizeof(int) * number_leg1);
            v2 = (int *)malloc(sizeof(int) * number_leg2);

            MPI_Recv(v0, number_leg0, MPI_INT, master, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(v1, number_leg1, MPI_INT, master, 1, MPI_COMM_WORLD, &status);
            MPI_Recv(v2, number_leg2, MPI_INT, master, 1, MPI_COMM_WORLD, &status);

            printf("%d -> ", rank);
            for (int i = 0; i < 3; i++)
            {
                printf("%d:", i);
                if (i == 0)
                {
                    for (int j = 0; j < number_leg0; j++)
                    {
                        if (j == number_leg0 - 1)
                        {
                            printf("%d ", v0[j]);
                        }
                        else
                        {
                            printf("%d,", v0[j]);
                        }
                    }
                }
                else if (i == 1)
                {
                    for (int j = 0; j < number_leg1; j++)
                    {
                        if (j == number_leg1 - 1)
                        {
                            printf("%d ", v1[j]);
                        }
                        else
                        {
                            printf("%d,", v1[j]);
                        }
                    }
                }
                else if (i == 2)
                {
                    for (int j = 0; j < number_leg2; j++)
                    {
                        if (j == number_leg2 - 1)
                        {
                            printf("%d", v2[j]);
                        }
                        else
                        {
                            printf("%d,", v2[j]);
                        }
                    }
                }
            }
            printf("\n");

            // primire particica vector

            MPI_Recv(&dim, 1, MPI_INT, master, 1, MPI_COMM_WORLD, &status);

            v_recv = (int *)malloc(sizeof(int) * dim);
            MPI_Recv(v_recv, dim, MPI_INT, master, 1, MPI_COMM_WORLD, &status);

            for (int i = 0; i < dim; i++)
            {
                v_recv[i] = v_recv[i] * 2;
            }

            MPI_Send(v_recv, dim, MPI_INT, master, 1, MPI_COMM_WORLD);
            printf("M(%d,%d) \n", rank, master);

            printf("\n");
        }
    }
    MPI_Finalize();
}
