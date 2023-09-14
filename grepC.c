#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

struct buffer {
    char data[100];
};

struct message {
    int type;
    int pid;
    struct buffer buffer;
};
void grep(const char *pattern, const char *filename, long int posicion) {
    FILE *file = fopen(filename, "r");
    struct message msg;
    if (file == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo '%s'\n", filename);
        exit(EXIT_FAILURE);
    }
     if (fseek(file, posicion, SEEK_SET) != 0) {
        fprintf(stderr, "No se pudo establecer la posición de lectura\n");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    regex_t regex;
    int reti;

    reti = regcomp(&regex, pattern, 0);
    if (reti) {
        fprintf(stderr, "No se pudo compilar la expresión regular\n");
        exit(EXIT_FAILURE);
    }
    msg.type = 0;
    msg.pid = 0;
    memset(msg.buffer.data, 0, sizeof(msg.buffer.data)); // Inicializa el buffer con ceros

    // Lee el archivo carácter por carácter
    char caracter;
    int indice = 0;
    while ((caracter = fgetc(file)) != EOF) {
        
            if (regexec(&regex, line, 0, NULL, 0) == 0) {
                printf("%s", line);
                break;
            }
        
        msg.buffer.data[indice] = caracter;
        indice++;

        // Verifica si el buffer se llena
        if (indice >= sizeof(msg.buffer.data) - 1) {
            perror("Buffer de línea lleno");
            break;
        }
    }

    fclose(file);
    regfree(&regex);
    // Imprime la línea leída y la posición en el archivo
    printf("Línea leída: %s\n", msg.buffer.data);
}


int main(int argc, char *argv[]) {

/*
    Fork

    PROCESOS HIJOS
        while(trabajando){
        - 1 sleep ( espera )
        - 2 leer --> posicion --> 3
        - 3 regex --> resultado --> 1
        }

    Matar Hijos


*/
    int cantidadHijos=3;
    int status, i;
    pid_t pids[3];

    for (i=0; i<cantidadHijos; i++) {
        pids[i] = fork();
        if (pids[i]==0) {
            while (1){
                printf ("Hijo %d corriendo ...\n", getpid());
            }
            sleep(1);
            exit(0);
        }
    }    

    sleep(1); //esperar que los hijos entren al ciclo infinito
    for (i = 0; i < cantidadHijos; i++) {
        
        printf("Cancelando Hijo:  %d\n", pids[i]);
        kill(pids[i], SIGKILL);
        wait(&status);
    }

    

/*    if (argc < 3) {
        fprintf(stderr, "Uso: %s 'expresion_regular' archivo1 archivo2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *pattern = argv[1];
    long int posicion = 10;
    for (int i = 2; i < argc; i++) {
        grep(pattern, argv[i], posicion );
    }*/

    return EXIT_SUCCESS;
}

