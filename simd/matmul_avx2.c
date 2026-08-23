#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VECTOR_SIZE 2048
#define MATRIX_SIZE 2048
#define AVX_FLOATS 8
#define DEFAULT_REPETITIONS 1

static inline __m256 simd_loadu_ps(const float *values)
{
    return _mm256_loadu_ps(values);
}

// Ejercicio A: multiplico los 8 floats del registro de una sola vez (c = a * b)
static inline __m256 simd_mul_ps(__m256 a, __m256 b)
{
    return _mm256_mul_ps(a, b);
}

// Ejercicio B: toca sumar los 8 numeros que quedaron en el registro para que quede un solo valor
// primero parto el registro de 256 bits en dos de 128 y los sumo, y despues voy sumando de a pares
// hasta que queda solo 1 numero
static inline float simd_reduce_add_ps(__m256 value)
{
    __m128 lo = _mm256_castps256_ps128(value);
    __m128 hi = _mm256_extractf128_ps(value, 1);
    lo = _mm_add_ps(lo, hi);

    __m128 shuf = _mm_movehdup_ps(lo);
    __m128 sums = _mm_add_ps(lo, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);

    return _mm_cvtss_f32(sums);
}

// Ejercicio C: aca uso lo de A y B juntos para sacar el producto punto
// voy avanzando de 8 en 8 (AVX_FLOATS), multiplico y acumulo en el registro,
// y al final si hago la reduccion (una sola vez, no en cada vuelta del for)
float dot_product_avx2(const float a[VECTOR_SIZE], const float b[VECTOR_SIZE])
{
    __m256 sum_vec = _mm256_setzero_ps();

    for (int i = 0; i < VECTOR_SIZE; i += AVX_FLOATS) {
        __m256 va = simd_loadu_ps(&a[i]);
        __m256 vb = simd_loadu_ps(&b[i]);
        sum_vec = _mm256_add_ps(sum_vec, simd_mul_ps(va, vb));
    }

    return simd_reduce_add_ps(sum_vec);
}

void transpose_matrix_1024(
    const float matrix[MATRIX_SIZE][MATRIX_SIZE],
    float transposed[MATRIX_SIZE][MATRIX_SIZE])
{
    for (int row = 0; row < MATRIX_SIZE; ++row) {
        for (int col = 0; col < MATRIX_SIZE; ++col) {
            transposed[col][row] = matrix[row][col];
        }
    }
}

void matrix_multiply_avx2_1024(
    const float a[MATRIX_SIZE][MATRIX_SIZE],
    const float b[MATRIX_SIZE][MATRIX_SIZE],
    float result[MATRIX_SIZE][MATRIX_SIZE])
{
    static float b_transposed[MATRIX_SIZE][MATRIX_SIZE];

    transpose_matrix_1024(b, b_transposed);

    for (int row = 0; row < MATRIX_SIZE; ++row) {
        for (int col = 0; col < MATRIX_SIZE; ++col) {
            result[row][col] = dot_product_avx2(a[row], b_transposed[col]);
        }
    }
}

static double get_time_seconds(void)
{
    struct timespec time;

    clock_gettime(CLOCK_MONOTONIC, &time);

    return (double)time.tv_sec + (double)time.tv_nsec / 1000000000.0;
}

static void init_matrix_1024(float matrix[MATRIX_SIZE][MATRIX_SIZE], float scale)
{
    for (int row = 0; row < MATRIX_SIZE; ++row) {
        for (int col = 0; col < MATRIX_SIZE; ++col) {
            matrix[row][col] = scale * (float)((row + col) % 17 + 1);
        }
    }
}

static double checksum_matrix_1024(const float matrix[MATRIX_SIZE][MATRIX_SIZE])
{
    double sum = 0.0;

    for (int row = 0; row < MATRIX_SIZE; ++row) {
        for (int col = 0; col < MATRIX_SIZE; ++col) {
            sum += matrix[row][col];
        }
    }

    return sum;
}

static float matrix_a[MATRIX_SIZE][MATRIX_SIZE];
static float matrix_b[MATRIX_SIZE][MATRIX_SIZE];
static float matrix_result[MATRIX_SIZE][MATRIX_SIZE];

int main(int argc, char **argv)
{
    int repetitions = DEFAULT_REPETITIONS;

    if (argc > 1) {
        repetitions = atoi(argv[1]);
    }

    if (repetitions <= 0) {
        fprintf(stderr, "Uso: %s [repeticiones]\n", argv[0]);
        return EXIT_FAILURE;
    }

    init_matrix_1024(matrix_a, 0.5f);
    init_matrix_1024(matrix_b, 0.25f);

    double start = get_time_seconds();

    for (int rep = 0; rep < repetitions; ++rep) {
        matrix_multiply_avx2_1024(matrix_a, matrix_b, matrix_result);
    }

    double end = get_time_seconds();
    double elapsed = end - start;
    double operations = (double)repetitions * 2.0 * MATRIX_SIZE * MATRIX_SIZE * MATRIX_SIZE;
    double gflops = operations / elapsed / 1000000000.0;
    double checksum = checksum_matrix_1024(matrix_result);

    printf("Tamano matriz: %dx%d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Repeticiones: %d\n", repetitions);
    printf("Checksum: %.6f\n", checksum);
    printf("C[0][0]: %.6f\n", matrix_result[0][0]);
    printf("C[1023][1023]: %.6f\n", matrix_result[1023][1023]);
    printf("Operaciones: %.0f\n", operations);
    printf("Tiempo: %.6f segundos\n", elapsed);
    printf("Rendimiento: %.6f GFLOP/s\n", gflops);

    return EXIT_SUCCESS;
}
