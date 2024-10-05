// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <semaphore.h>
// #include <immintrin.h>

// #define MATRIX_SIZE 500

// // 定义矩阵大小
// #define N MATRIX_SIZE
// #define M MATRIX_SIZE
// #define P MATRIX_SIZE

// // 定义线程数量
// #define NUM_THREADS 16

// // 定义矩阵结构体
// typedef struct
// {
//     int row;
//     int col;
// } MatrixSize;

// // 定义矩阵 A, B 和结果矩阵 C
// int A[N][M];
// int B[M][P];
// int C[N][P];

// // 线程池结构体
// typedef struct
// {
//     pthread_t *threads;
//     int count;
//     int next;
//     sem_t semaphore;
// } ThreadPool;

// // 初始化线程池
// void initThreadPool(ThreadPool *pool, int threadCount)
// {
//     pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * threadCount);
//     pool->count = threadCount;
//     pool->next = 0;
//     sem_init(&pool->semaphore, 0, threadCount);
// }

// // 销毁线程池
// void destroyThreadPool(ThreadPool *pool)
// {
//     free(pool->threads);
//     sem_destroy(&pool->semaphore);
// }

// // 从线程池获取一个线程
// pthread_t getThread(ThreadPool *pool)
// {
//     sem_wait(&pool->semaphore);
//     pthread_t thread = pool->threads[pool->next++];
//     pool->next %= pool->count;
//     return thread;
// }

// // 归还线程到线程池
// void returnThread(ThreadPool *pool, pthread_t thread)
// {
//     pool->threads[pool->next++] = thread;
//     pool->next %= pool->count;
//     sem_post(&pool->semaphore);
// }

// // 矩阵乘法的单个元素计算（使用AVX2指令集）
// void multiply(MatrixSize *size)
// {
//     int i, j;
//     __m256i result = _mm256_setzero_si256();

//     for (i = 0; i < M; i += 8)
//     {
//         __m256i a = _mm256_loadu_si256((__m256i *)&A[size->row][i]);
//         __m256i b = _mm256_loadu_si256((__m256i *)&B[i][size->col]);
//         result = _mm256_add_epi32(result, _mm256_mullo_epi32(a, b));
//     }

//     int *res = (int *)&result;
//     C[size->row][size->col] = res[0] + res[1] + res[2] + res[3] + res[4] + res[5] + res[6] + res[7];
// }

// // 线程执行函数
// void *threadFunction(void *arg)
// {
//     ThreadPool *pool = (ThreadPool *)arg;

//     while (1)
//     {
//         pthread_t self = pthread_self();
//         pthread_t thread = getThread(pool);

//         if (pthread_equal(self, thread))
//         {
//             break; // 退出线程
//         }

//         multiply((void *)&thread);

//         returnThread(pool, self);
//     }

//     pthread_exit(NULL);
// }

// int main()
// {
//     ThreadPool pool;
//     initThreadPool(&pool, NUM_THREADS);

//     MatrixSize sizes[N][P];

//     // 初始化矩阵 A 和 B
//     printf("Matrix A:\n");
//     for (int i = 0; i < N; i++)
//     {
//         for (int j = 0; j < M; j++)
//         {
//             A[i][j] = rand() % 10;
//             printf("%d ", A[i][j]);
//         }
//         printf("\n");
//     }

//     printf("\nMatrix B:\n");
//     for (int i = 0; i < M; i++)
//     {
//         for (int j = 0; j < P; j++)
//         {
//             B[i][j] = rand() % 10;
//             printf("%d ", B[i][j]);
//         }
//         printf("\n");
//     }

//     // 创建线程并进行矩阵乘法
//     for (int i = 0; i < pool.count; i++)
//     {
//         pthread_create(&pool.threads[i], NULL, threadFunction, (void *)&pool);
//     }

//     // 等待线程完成
//     for (int i = 0; i < pool.count; i++)
//     {
//         pthread_join(pool.threads[i], NULL);
//     }

//     destroyThreadPool(&pool);

//     // 打印结果矩阵 C
//     printf("Result Matrix C:\n");
//     for (int i = 0; i < N; i++)
//     {
//         for (int j = 0; j < P; j++)
//         {
//             printf("%d ", C[i][j]);
//         }
//         printf("\n");
//     }

//     return 0;
// }
#include <immintrin.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#define MATRIX_SIZE 8192
#define NUM_THREAD 4
float matA[MATRIX_SIZE][MATRIX_SIZE];
float matB[MATRIX_SIZE][MATRIX_SIZE];
float matC[MATRIX_SIZE][MATRIX_SIZE];
int step = 0;
void *multiplicationAVX(void *args)
{
    __m256 vecA, vecB, vecC;
    int thread = step++;
    for (int i = thread * MATRIX_SIZE / NUM_THREAD;
         i < (thread + 1) * MATRIX_SIZE / NUM_THREAD; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            matC[i][j] = 0.0f;
        }
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            vecA = _mm256_set1_ps(matA[i][j]);
            for (int k = 0; k < MATRIX_SIZE; k += 8)
            {
                vecB = _mm256_loadu_ps(&matB[j][k]);
                vecC = _mm256_loadu_ps(&matC[i][k]);
                vecC = _mm256_fmadd_ps(vecA, vecB, vecC);
                _mm256_storeu_ps(&matC[i][k], vecC);
            }
        }
    }
    return 0;
}
void *multiplicationNormal(void *args)
{
    int thread = step++;
    for (int i = thread * MATRIX_SIZE / NUM_THREAD;
         i < (thread + 1) * MATRIX_SIZE / NUM_THREAD; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            for (int k = 0; k < MATRIX_SIZE; k++)
            {
                matC[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    return 0;
}
void createMatrix()
{
    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            matA[i][j] = i + j * 2;
            matB[i][j] = i * 2 + j;
            matC[i][j] = 0.0f;
        }
    }
}
void printMatrix()
{
    if (MATRIX_SIZE <= 16)
    {
        printf("Matriz A");
        for (int i = 0; i < MATRIX_SIZE; i++)
        {
            printf("\n");
            for (int j = 0; j < MATRIX_SIZE; j++)
            {
                printf("%f ", matA[i][j]);
            }
        }
        printf("\n\n");
        printf("Matriz B");
        for (int i = 0; i < MATRIX_SIZE; i++)
        {
            printf("\n");
            for (int j = 0; j < MATRIX_SIZE; j++)
            {
                printf("%f ", matB[i][j]);
            }
        }
        printf("\n\n");
        printf("Multiplying matrix A with B");
        for (int i = 0; i < MATRIX_SIZE; i++)
        {
            printf("\n");
            for (int j = 0; j < MATRIX_SIZE; j++)
            {
                printf("%f ", matC[i][j]);
            }
        }
    }
}
int main()
{
    pthread_t threads[NUM_THREAD];
    clock_t start, end;
    createMatrix();
    start = clock();
    for (int i = 0; i < NUM_THREAD; i++)
    {
        // pthread_create(&threads[i], NULL, multiplicationNormal, NULL);
        pthread_create(&threads[i], NULL, multiplicationAVX, NULL);
    }
    for (int i = 0; i < NUM_THREAD; i++)
    {
        pthread_join(threads[i], NULL);
    }
    end = clock();
    printMatrix();
    printf("\n\n使用的线程数 -> %d\n", NUM_THREAD);
    printf("\n矩阵大小 -> %d\n", MATRIX_SIZE);
    printf("\n程序运行时间（毫秒） -> %f\n\n", (float)(end - start) * 1000 / CLOCKS_PER_SEC);
}