#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct buffer {
    char data[100];
};

struct message {
    int type;
    int pid;
    struct buffer buffer;
};

int main() {
    FILE *archivo;
    struct message msg;
    long int posicion; // Variable para almacenar la posición en el archivo
    
    // Abre el archivo en modo lectura
    archivo = fopen("DonQuijote.txt", "r");

    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return 1;
    }

    // Inicializa el buffer en cada mensaje
    msg.type = 0;
    msg.pid = 0;
    memset(msg.buffer.data, 0, sizeof(msg.buffer.data)); // Inicializa el buffer con ceros

    // Lee el archivo carácter por carácter
    char caracter;
    int indice = 0;
    while ((caracter = fgetc(archivo)) != EOF) {
        // Si el carácter es un salto de línea, termina la lectura
        if (caracter == '\n') {
            break;
        }

        // Almacena el carácter en el buffer
        msg.buffer.data[indice] = caracter;
        indice++;

        // Verifica si el buffer se llena
        if (indice >= sizeof(msg.buffer.data) - 1) {
            perror("Buffer de línea lleno");
            break;
        }
    }

    // Obtiene la posición actual en el archivo
    posicion = ftell(archivo); //Luego se puede usar fseek para leer apartir de esa posición o desde posicion+1 si hay que saltarse el salto de linea
    
    // Imprime la línea leída y la posición en el archivo
    printf("Línea leída: %s\n", msg.buffer.data);
    printf("Posición en el archivo: %ld\n", posicion);

    // Cierra el archivo
    fclose(archivo);

    return 0;
}
