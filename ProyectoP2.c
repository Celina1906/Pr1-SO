#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/msg.h>

struct buffer {
    char data[100];
};

struct message {
  long type;
  long int posicion;
  int tipoAccion;
  int numeroProceso;
  int numeroArchivo;
  struct buffer buffer;
} msg;


bool finArchivo = false;

void leer( const char *filename, long int posicion) {
    printf("leer %li\n", posicion);
    memset(msg.buffer.data, 0, sizeof(msg.buffer.data)); // Inicializa el buffer con ceros
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo '%s'\n", filename);
        exit(EXIT_FAILURE);
    }
     if (fseek(file, posicion-1, SEEK_SET) != 0) {
        fprintf(stderr, "No se pudo establecer la posición de lectura\n");
        exit(EXIT_FAILURE);
    }
    
    // Lee el archivo carácter por carácter
    char caracter;
    long int indice = 0;
    while ((caracter = fgetc(file)) != EOF) {
        if (caracter == '\n') {
            msg.tipoAccion = 1;
            break;
        } 
        msg.buffer.data[indice] = caracter;
        indice++;

        // Verifica si el buffer se llena
        if (indice >= sizeof(msg.buffer.data) - 1) {
            perror("Buffer de línea lleno");
            msg.tipoAccion = 1;
            break;
        }
    }
    if ((caracter = fgetc(file)) == EOF){
        msg.tipoAccion = 2;
        //finArchivo = true;
    }
    
    msg.posicion = ftell(file);
    // Imprime la línea leída y la posición en el archivo
    printf("Línea leída: %s\n", msg.buffer.data);
    
    fclose(file);

}

int main(int argc, char *argv[]) {
    /* Revisar parametros */
    if (argc < 3) {
        fprintf(stderr, "Uso: %s 'expresion_regular' archivo1 archivo2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }
    /* Revisar parametros */
    /* Crear Pool de Hijos */
    int cantidadHijos=3;
    int status, i;
    long int posicionFinal;
    pid_t pids[3];
     //Necesario para enviar mensajes
    key_t msqkey = 999;
    int msqid = msgget(msqkey, IPC_CREAT | S_IRUSR | S_IWUSR);
    for (i = 0; i < cantidadHijos; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            int childNumber = i + 1;
            while (1) {
                if (msgrcv(msqid, &msg, sizeof(struct message), childNumber, 0) > 0) { //Revisar proceso hijo
                        printf("Hijo %d: Me llegó un mensaje en posición: %ld\n", childNumber, msg.posicion);
                         printf("Hijo %d: filename num %i\n", childNumber, msg.numeroArchivo);
                    leer(argv[msg.numeroArchivo], msg.posicion);
                     printf("despues leer\n");
                    if (msg.tipoAccion == 1){
                        msg.type =4;
                    }
                    else if(msg.tipoAccion == 2){
                        msg.type = 4;
                    }
                    if (childNumber == 1){
                        msg.numeroProceso = 0;
                    }
                    else if(childNumber ==2){
                        msg.numeroProceso = 1;
                    }
                    else if (childNumber ==3){
                        msg.numeroProceso = 2;
                    }
                    
                     printf("Tipo accion %i \n", msg.tipoAccion);
                     printf("Tipo mensaje %li \n", msg.type);
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    sleep(1);
                    
                }
                
            }
            exit(0);
        }
    }

    sleep(2); //esperar que los hijos entren al ciclo infinito
    /* Crear Pool de Hijos */
    int childNumber = 1;
    msg.type = childNumber;
    msg.numeroProceso = 0;
    msg.posicion = 0;
    msg.numeroArchivo = 2; //Posiblemente corregir en parametros
    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);

    while(1){
         printf("dentro while");
        msgrcv(msqid, &msg, sizeof(struct message) , 4, 0);
        if(msg.tipoAccion == 1){ 
            
            if(msg.numeroProceso == 0){
                msg.type = 2;
            }
            else if(msg.numeroProceso ==1){
                msg.type = 3;
            }
            else if(msg.numeroProceso == 2){
                msg.type = 1;
            }
            msg.numeroArchivo = 2;
            
            msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            
            sleep(1);
            
            
        } 
        else if (msg.tipoAccion==2){
            printf("final archivo2\n");
             printf("fin de archivo2 %d \n", finArchivo);
            exit(0);  
        }
        else if(msg.tipoAccion ==3){
            printf("imprimir resultado \n");
        }
        
    }

    for (int i = 0; i < cantidadHijos; i++) {
        wait(NULL);
    }

    return EXIT_SUCCESS;
}