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
  int numeroProceso;
  int numeroArchivo;
  struct buffer buffer;
} msg;

int flagSleepP0 = 1;
int flagLeerP0 = 0;
int flagRegexP0 = 0;
int flagSleepP1 = 1;
int flagLeerP1 = 0;
int flagRegexP1 = 0;
int flagSleepP2 = 1;
int flagLeerP2 = 0;
int flagRegexP2 = 0;

bool finArchivo = false;

void leer( const char *filename, long int posicion) {
    printf("leer %li\n", posicion);
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
    msg.type = 4;
    
    // Lee el archivo carácter por carácter
    char caracter;
    long int indice = 0;
    while ((caracter = fgetc(file)) != EOF) {
        if (caracter == '\n') {
            msg.type = 5;
            break;
        } 
        msg.buffer.data[indice] = caracter;
        indice++;

        // Verifica si el buffer se llena
        if (indice >= sizeof(msg.buffer.data) - 1) {
            perror("Buffer de línea lleno");
            msg.type = 5;
            break;
        }
    }
    if ((caracter = fgetc(file)) == EOF){
        msg.type = 4;
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
                if (msgrcv(msqid, &msg, sizeof(struct message), childNumber, 0) > 0) {
                    printf("Hijo %d: Me llegó un mensaje en posición: %ld\n", childNumber, msg.posicion);
                    printf("Hijo %d: filename num %i\n", childNumber, msg.numeroArchivo);
                    leer(argv[msg.numeroArchivo], msg.posicion);
                    printf("despues leer\n");
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    sleep(1);
                    if(childNumber == 1){
                         flagSleepP0 = 1;
                     }
                     else if(childNumber == 2){
                        flagSleepP1 = 1;
                     }
                     else if(childNumber ==3){
                         flagSleepP2  = 1;
                     }
                }
                else if(msgrcv(msqid, &msg, sizeof(struct message), 6, 0)){
                    //printf(msg.type);
                    printf("type %li", msg.type);
                }
            }
            exit(0);
        }
    }

    sleep(2); //esperar que los hijos entren al ciclo infinito
    /* Crear Pool de Hijos */
    for (int a = 2; a < argc; a++) {
        //filename=argv[a];
        // grep(pattern, argv[i], posicion );
    //    finArchivo = false;
        int childNumber;
       if (flagSleepP0 == 1){
            flagSleepP0 = 0; 
            flagLeerP0 = 1;
            childNumber = 1;
            
        }
        else if (flagSleepP1 == 1){ 
            flagSleepP1 = 0; 
            flagLeerP1 = 1;
            childNumber = 2;
              
        }
        else{ 
            flagSleepP2 = 0;
            flagLeerP2 = 1;
            childNumber = 3;
            
        }

        msg.type = childNumber;
        msg.posicion = 0;
        msg.numeroArchivo = a;
        msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);

        while (finArchivo==false){
             if(msgrcv(msqid, &msg, sizeof(struct message) , 5, 0)){ 
                if (flagSleepP0 == 1){
                    flagSleepP0 = 0;
                    flagLeerP0 = 1;
                    childNumber = 1;
                    }
           
                else if (flagSleepP1 == 1){ 
                    flagSleepP1 = 0;
                    flagLeerP1 = 1;
                    childNumber = 2;
                }
                else if (flagSleepP2 == 1){ 
                    flagSleepP2 = 0;
                    flagLeerP2 = 1;
                    childNumber = 3;
                }
                msg.type = childNumber;
                msg.numeroArchivo = a;
                msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                
            } else if (msgrcv(msqid, &msg, sizeof(struct message) , 4, 0)){
                printf("final archivo");
                finArchivo = true;
                msg.type = 6;
                msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                /* code */
            }
            
        }
        
    }

    // const char * file = argv[2];
    // leer(file,0);
    for (int i = 0; i < cantidadHijos; i++) {
        wait(NULL);
    }

    return EXIT_SUCCESS;
}