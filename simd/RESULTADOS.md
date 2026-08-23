# Laboratorio 2 - Resultados
# Estudiante: Fabián Parreguirre Hidalgo

## Como correrlo

```bash
cd simd
make clean
make all
./matmul_avx2 3
./matmul_scalar 3
```

El numero (3) es la cantidad de veces que repite todo el calculo, es solo para que el tiempo
no salga tan chiquito y se pueda medir mejor.

## Output real de la terminal (copy paste)

Compilando:

```
$ make clean && make all
rm -f matmul_avx2 matmul_scalar
gcc -Wall -Wextra -O3 -mavx2 matmul_avx2.c -o matmul_avx2
gcc -Wall -Wextra -O3 matmul_scalar.c -o matmul_scalar
```

Corriendo la version con AVX2:

```
$ ./matmul_avx2 3
Tamano matriz: 2048x2048
Repeticiones: 3
Checksum: 86972906452.000000
C[0][0]: 26800.500000
C[1023][1023]: 26836.500000
Operaciones: 51539607552
Tiempo: 5.881313 segundos
Rendimiento: 8.763282 GFLOP/s
```

Corriendo la version escalar (sin vectorizar):

```
$ ./matmul_scalar 3
Tamano matriz: 2048x2048
Repeticiones: 3
Checksum: 86972906452.000000
C[0][0]: 26800.500000
C[1023][1023]: 26836.500000
Operaciones: 51539607552
Tiempo: 26.705810 segundos
Rendimiento: 1.929902 GFLOP/s
```

## Que hice en cada ejercicio

**Ejercicio A y B:** en `matmul_avx2.c` estaban las funciones `simd_mul_ps` y
`simd_reduce_add_ps` sin hacer nada (venian vacias en la plantilla). `simd_mul_ps` la hice con
`_mm256_mul_ps`, que multiplica los 8 floats del registro al mismo tiempo. Y `simd_reduce_add_ps`
la hice para sumar esos 8 numeros y que quede uno solo (eso es la reduccion), toco partir el
registro en dos mitades e ir sumando hasta que queda 1 valor.

**Ejercicio C:** con esas dos funciones de arriba arme `dot_product_avx2`, que es basicamente el
producto punto de dos vectores pero yendo de 8 en 8 en vez de de uno en uno. Esta funcion ya la
usaba `matrix_multiply_avx2_1024` (esa parte ya venia hecha en la plantilla) para calcular cada
casilla de la matriz resultado, entonces con eso quedo completa la multiplicacion de matrices
usando AVX2.

## Ejercicio D - comparacion de tiempos

| Version | Tiempo | Rendimiento | Checksum |
|---|---|---|---|
| Escalar (sin vectorizar) | 26.70 s | 1.93 GFLOP/s | 86972906452.000000 |
| AVX2 (vectorizada) | 5.88 s | 8.76 GFLOP/s | 86972906452.000000 |

Quedo un speedup de mas o menos **4.5 veces mas rapido** con AVX2.

Lo primero que reviso es que el checksum de las dos versiones da exactamente igual, o sea que
la version vectorizada esta calculando bien, no se dañaron los numeros por usar SIMD, solo se
hizo mas rapido.

Uno pensaria que como AVX2 mueve 8 floats de una vez deberia dar 8 veces mas rapido y no los 4.5x
que dio, pero no llega a eso por un par de cosas: la transpuesta de la matriz b se sigue haciendo
de forma normal (no esta vectorizada), y ademas aunque uno vectorice las cuentas, la memoria no se
vectoriza, entonces el programa se queda esperando que lleguen los datos de la cache/RAM y ese
tiempo no se gana por mas rapido que sea el CPU haciendo las multiplicaciones. Por eso en la
practica casi nunca se llega al speedup teorico (8x), pero igual 4.5x es una mejora bastante
buena para el cambio que se hizo.
