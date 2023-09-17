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

int flagFin = 0;

void leer( const char *filename, long int posicion) {
    printf("leer %li /n", posicion);
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo '%s'\n", filename);
        exit(EXIT_FAILURE);
    }
     if (fseek(file, posicion, SEEK_SET) != 0) {
        fprintf(stderr, "No se pudo establecer la posición de lectura\n");
        exit(EXIT_FAILURE);
    }
    msg.type = 2;
    memset(msg.buffer.data, 0, sizeof(msg.buffer.data)); // Inicializa el buffer con ceros

    // Lee el archivo carácter por carácter
    char caracter;
    long int indice = 0;
    while ((caracter = fgetc(file)) != EOF) {
        if (caracter == '\n') {
            msg.type = 1;
            break;
        }
        
        msg.buffer.data[indice] = caracter;
        indice++;

        // Verifica si el buffer se llena
        if (indice >= sizeof(msg.buffer.data) - 1) {
            perror("Buffer de línea lleno");
            msg.type = 1;
            break;
        }
    }
    msg.posicion = indice;
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
    
    /* Verificar el patron de la expresion regular */ 
    const char *pattern = argv[1];
    regex_t regex;
    int reti;

    reti = regcomp(&regex, pattern, 0);
    if (reti) {
        fprintf(stderr, "No se pudo compilar la expresión regular\n");
        exit(EXIT_FAILURE);
    }
    /* Verificar el patron de la expresion regular */ 
    /* Crear Pool de Hijos */
    int cantidadHijos=3;
    int status, i;
    long int posicionFinal;
    pid_t pids[3];

    //Necesario para enviar mensajes
    key_t msqkey = 999;
    int msqid = msgget(msqkey, IPC_CREAT | S_IRUSR | S_IWUSR);
    for (i=0; i<cantidadHijos; i++) {
        pids[i] = fork();
        if (pids[i]==0) {
            while (1){
    
                // Falta agregar las acciones a realizar segun el mensaje recibido
                if(msgrcv(msqid, &msg, sizeof(struct message) , 2, 0)){
                    printf("Me llego un mensaje de leer en posicion: %d\n", getpid());
                    leer(argv[msg.numeroArchivo], msg.posicion);
                    sleep(1);
                    msg.type = 1;
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    sleep(1);
                }
                else if(msgrcv(msqid, &msg, sizeof(struct message) , 3, 0)){
                    printf("Me llego un mensaje de regex: %d\n",getpid() );
                }
                
            }
            sleep(1);
            exit(0);
        }
    }    

    sleep(2); //esperar que los hijos entren al ciclo infinito
    /* Crear Pool de Hijos */
    /* LECTURA DE ARCHIVO(S) */
    long int posicion = 0;
    i=0;
    bool finArchivo;
    for (int a = 2; a < argc; a++) {
        // grep(pattern, argv[i], posicion );
       finArchivo = false;
       if (flagSleepP0 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
            flagSleepP0 = 0; 
            flagLeerP0 = 1;
            printf("P0 Leer\n");
            msg.type = 2;
            msg.posicion = 0;
            msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            sleep(1);
            printf("P0 Leer2\n");
        }
        else if (flagSleepP1 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
            flagSleepP1 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
            flagLeerP1 = 1;
            msg.type = 2;
            printf("P1 Leer");
            msg.posicion = 0;
            msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            sleep(1);
        }
        else if (flagSleepP2 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
            flagSleepP2 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
            flagLeerP2 = 1;
            msg.type = 2;
            printf("P2 Leer");
            msg.posicion = 0;
            msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            sleep(1);
        }
        sleep(1);
        while (finArchivo==false){
            //printf("dentro while\n");
            //sleep(1);
           if(msgrcv(msqid, &msg, sizeof(struct message) , 1, 0)){ 
                /* Asignar Siguiente */
                //printf("dentro mensaje\n");
                // printf("Me llego un mensaje de despertar: %d\n",getpid() );  
                if (flagSleepP0 == 1){
                    printf("dentro P0\n");//if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
                    flagSleepP0 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
                    flagLeerP0 = 1;
                    msg.type = 2;
                    msg.posicion = msg.posicion;
                     msg.numeroArchivo = a - 2;
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    sleep(1);
                    }
           
                else if (flagSleepP1 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
                    msg.type = 2;
                    msg.posicion = msg.posicion;
                    msg.numeroArchivo = a - 2;
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    sleep(1);
                    flagSleepP0 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
                    flagLeerP0 = 1;
                }
                else if (flagSleepP2 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
                    msg.type = 2;
                    msg.posicion = msg.posicion;
                    msg.numeroArchivo = a - 2;
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    sleep(1);
                    flagSleepP0 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
                    flagLeerP0 = 1;
                }
                
            }

            else if(msgrcv(msqid, &msg, sizeof(struct message) , 2, 0)){
                /*Termino archivo*/
                /*finArchivo = true;
                printf("Termino lectura de archivos");*/
            } 
           else{
                printf("no recibí ni mierda\n");
          
            }
            }
    }
    
    //while (flagFin == 0)
    
        
        sleep(1);
    /* LECTURA DE ARCHIVO(S) */


    return EXIT_SUCCESS;
    }