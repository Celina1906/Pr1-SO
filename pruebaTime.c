#include <stdio.h>
#include <time.h>

int main() {
    clock_t inicio = clock();
  
    printf("Fin del programa\n");
    printf("Fin del programa\n");
    printf("Fin del programa\n");
    printf("Fin del programa\n");
    printf("Fin del programa\n");
    printf("Fin del programa\n");
    // Código del programa

    clock_t fin = clock();
    double tiempo = (double)(fin - inicio) / CLOCKS_PER_SEC;
    printf("El programa tomó %f segundos en ejecutarse.\n", tiempo);

    return 0;
}
