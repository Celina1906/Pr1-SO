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
    char data[8100];
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
    //printf("leer %li\n", posicion);
    memset(msg.buffer.data, 0, sizeof(msg.buffer.data)); // Inicializa el buffer con ceros
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo '%s'\n", filename);
        exit(EXIT_FAILURE);
    }
     if (fseek(file, posicion, SEEK_SET) != 0) {
        fprintf(stderr, "No se pudo establecer la posición de lectura\n");
        exit(EXIT_FAILURE);
    }
    
    // Lee el archivo carácter por carácter
    char caracter;
    long int indice = 0;
    while ((caracter = fgetc(file)) != EOF) {
        //if (caracter == '\n') {
           // msg.tipoAccion = 1;
           // break;
        //} 
        msg.buffer.data[indice] = caracter;
        indice++;

        // Verifica si el buffer se llena
        if (indice >= sizeof(msg.buffer.data) - 1) {
            //perror("Buffer de línea lleno");
            msg.tipoAccion = 1;
            break;
        }
    }
    if ((caracter = fgetc(file)) == EOF){
        printf("Fin de archivo\n");
        msg.tipoAccion = 2;
       
    }
    
    msg.posicion = ftell(file);
    // Imprime la línea leída y la posición en el archivo
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

    regex_t regex;
    int reti;

    reti = regcomp(&regex, argv[1], 0);
    for (i = 0; i < cantidadHijos; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            int childNumber = i + 1;
            while (1) {
                if (msgrcv(msqid, &msg, sizeof(struct message), childNumber, 0) > 0) { //Revisar proceso hijo
                    leer(argv[msg.numeroArchivo], msg.posicion);
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
                    
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            
                    if (regexec(&regex, msg.buffer.data, 0, NULL, 0) == 0) {
                        msg.tipoAccion = 3;
                        msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    }


                    
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
        msgrcv(msqid, &msg, sizeof(struct message) , 4, 0);
        sleep(1);
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
            exit(0);  
            // return 0;

            
        }
        else if(msg.tipoAccion ==3){
            printf("%s\n", msg.buffer.data);
        }
        
    }
    
 
    for (int i = 0; i < cantidadHijos; i++) {
        wait(NULL);
    }

    return EXIT_SUCCESS;
}