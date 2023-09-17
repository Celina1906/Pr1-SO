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
    printf("leer");
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
    msg.type = 2;
    memset(msg.buffer.data, 0, sizeof(msg.buffer.data)); // Inicializa el buffer con ceros

    // Lee el archivo carácter por carácter
    char caracter;
    int indice = 0;
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
    fclose(file);
    

}

void revisionExpresion(const char *pattern) {

}



// void grep(const char *pattern, const char *filename, long int posicion) {
//     FILE *file = fopen(filename, "r");
//     struct message msg;
//     if (file == NULL) {
//         fprintf(stderr, "No se pudo abrir el archivo '%s'\n", filename);
//         exit(EXIT_FAILURE);
//     }
//      if (fseek(file, posicion, SEEK_SET) != 0) {
//         fprintf(stderr, "No se pudo establecer la posición de lectura\n");
//         exit(EXIT_FAILURE);
//     }

//     char line[1024];
//     regex_t regex;
//     int reti;

//     reti = regcomp(&regex, pattern, 0);
//     if (reti) {
//         fprintf(stderr, "No se pudo compilar la expresión regular\n");
//         exit(EXIT_FAILURE);
//     }
//     msg.type = 0;
//     msg.pid = 0;
//     memset(msg.buffer.data, 0, sizeof(msg.buffer.data)); // Inicializa el buffer con ceros

//     // Lee el archivo carácter por carácter
//     char caracter;
//     int indice = 0;
//     while ((caracter = fgetc(file)) != EOF) {
        
//             if (regexec(&regex, line, 0, NULL, 0) == 0) {
//                 printf("%s", line);
//                 break;
//             }
        
//         msg.buffer.data[indice] = caracter;
//         indice++;

//         // Verifica si el buffer se llena
//         if (indice >= sizeof(msg.buffer.data) - 1) {
//             perror("Buffer de línea lleno");
//             break;
//         }
//     }

//     fclose(file);
//     regfree(&regex);
//     // Imprime la línea leída y la posición en el archivo
//     printf("Línea leída: %s\n", msg.buffer.data);
// }


int main(int argc, char *argv[]) {
    /* Revisar parametros */
    
    printf("revision");
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
    printf("patron");
    /* Crear Pool de Hijos */
    int cantidadHijos=3;
    int status, i;
    long int posicionFinal;
    pid_t pids[3];

    //Necesario para enviar mensajes
    key_t msqkey = 999;
    int msqid = msgget(msqkey, IPC_CREAT | S_IRUSR | S_IWUSR);
    printf("antes pool");
    for (i=0; i<cantidadHijos; i++) {
        pids[i] = fork();
        if (pids[i]==0) {
            while (1){
                // Falta agregar las acciones a realizar segun el mensaje recibido
                if(msgrcv(msqid, &msg, sizeof(struct message) , 1, 0)){
                    printf("Me llego un mensaje de despertar: %d\n",getpid() );  
                }
                 if(msgrcv(msqid, &msg, sizeof(struct message) , 2, 0)){
                    printf("Me llego un mensaje de leer en posicion: %d\n", getpid());
                    leer(argv[msg.numeroArchivo], msg.posicion);
                    // sleep(1);
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    // sleep(1);
                    // Devolver posicion
                    // Regex()
                }
                if(msgrcv(msqid, &msg, sizeof(struct message) , 3, 0)){
                    printf("Me llego un mensaje de regex: %d\n",getpid() );
                }
                
            }
            sleep(1);
            exit(0);
        }
    }    

    sleep(1); //esperar que los hijos entren al ciclo infinito
    /* Crear Pool de Hijos */
    printf("luego pool");

    /* LECTURA DE ARCHIVO(S) */
    /*
        FLUJO DE LECTURA DE ARCHIVO(S)
            For archivo in archivos
                finArchivo = false
                if x3 flagSleepP0 == 1
                    msgsnd( posicion == 0 )
                while(finArchivo==false){
                    ~ 1 --> return posicion --> asignar sig
                        
                    ~ 2 --> imprimirlo 
                    if(tipo mensaje == 1){
                        if x3 flagSleepP0 == 1
                    }
                }

        */
    long int posicion = 10;
    i=0;
    bool finArchivo;
    printf("Antes de Leer");
    for (int a = 2; a < argc; a++) {
        // grep(pattern, argv[i], posicion );
       finArchivo = false;
       if (flagSleepP0 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
            printf("P0 Leer");
            msg.type = 2;
            msg.posicion = 0;
            msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            sleep(1);
            flagSleepP0 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
            flagLeerP0 = 1;
        }
        else if (flagSleepP1 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
            msg.type = 2;
            printf("P1 Leer");
            msg.posicion = 0;
            msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            sleep(1);
            flagSleepP0 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
            flagLeerP0 = 1;
        }
        else if (flagSleepP2 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
            msg.type = 2;
            printf("P2 Leer");
            msg.posicion = 0;
            msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            sleep(1);
            flagSleepP0 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
            flagLeerP0 = 1;
        }
        while (finArchivo==false){
            printf("dentro while");
            if(msgrcv(msqid, &msg, sizeof(struct message) , 1, 0)){ 
                /* Asignar Siguiente */
                // printf("Me llego un mensaje de despertar: %d\n",getpid() );  
                if (flagSleepP0 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
                    msg.type = 2;
                    msg.posicion = 0;
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    sleep(1);
                    flagSleepP0 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
                    flagLeerP0 = 1;
                    }
                else if (flagSleepP1 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
                    msg.type = 2;
                    msg.posicion = 0;
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    sleep(1);
                    flagSleepP0 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
                    flagLeerP0 = 1;
                }
                else if (flagSleepP2 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
                    msg.type = 2;
                    msg.posicion = 0;
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    sleep(1);
                    flagSleepP0 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
                    flagLeerP0 = 1;
                }
                
            }

            else if(msgrcv(msqid, &msg, sizeof(struct message) , 2, 0)){
                /* Termino archivo*/
                finArchivo = true;
                printf("Termino lectura de archivos");
            }
            // if(msgrcv(msqid, &msg, sizeof(struct message) , 3, 0)){
            //     /*Imprimir resultado*/
            //     printf("Me llego un mensaje de regex: %d\n",getpid() );
            // }

            // if (i==0){ //Segun lo que entiendo este if se deberia ir porque todos los hijos deberian estar ocupados al mismo tiempo
            //     printf("Hola proceso a %d \n", pids[i]);
            //     if (flagSleepP0 == 1){ //if ((flagSleepP0 == 1 && flagSleepP1 == 0 && flagSleepP2 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 1 && flagSleepP1 == 0) || (flagSleepP0 == 0 && flagSleepP1 == 0 && flagSleepP2 == 1) )
            //         msg.type = 1;
            //         printf("Enviando mensaje de despertar a %d \n", pids[i]);
            //         msg.posicion = 58;
            //         msg.posicion = rand() % 30;
            //         msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            //         flagSleepP0 = 0; //Habria que cambiar esto con las condiciones de arriba para saber cual esta en true y false
            //         flagLeerP0 = 1;
            // }
            //     if (flagLeerP0 == 1){
            //         msg.type = 2;
            //         printf("Enviando mensaje de leer a %d \n", pids[i]);
            //         msg.posicion = 58;
            //         msg.posicion = rand() % 30;
            //         msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            //         flagSleepP0 = 0;
            //         flagLeerP0 = 0;
            //         flagRegexP0 = 1;
            //     }
            //     if (flagRegexP0 == 1){
            //         msg.type = 3;
            //         printf("Enviando mensaje de regex a %d \n", pids[i]);
            //         msg.posicion = 58;
            //         msg.posicion = rand() % 30;
            //         msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            //         flagSleepP0 = 1;
            //         flagLeerP0 = 0;
            //         flagRegexP0 = 0;
            //     }
            // }
            // if (i==1){//Segun lo que entiendo este if se deberia ir 
            //     printf("Hola proceso a %d \n", pids[i]);
            //     if (flagSleepP1 == 1){
            //         msg.type = 1;
            //         printf("Enviando mensaje de despertar a %d \n", pids[i]);
            //         msg.posicion = 58;
            //         msg.posicion = rand() % 30;
            //         msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            //         flagSleepP1 = 0;
            //         flagLeerP1 = 1;
            // }
            //     if (flagLeerP1 == 1){
            //         msg.type = 2;
            //         printf("Enviando mensaje de leer a %d \n", pids[i]);
            //         msg.posicion = 58;
            //         msg.posicion = rand() % 30;
            //         msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            //         flagSleepP1 = 0;
            //         flagLeerP1 = 0;
            //         flagRegexP1 = 1;
            //     }
            //     if (flagRegexP1 == 1){
            //         msg.type = 3;
            //         printf("Enviando mensaje de regex a %d \n", pids[i]);
            //         msg.posicion = 58;
            //         msg.posicion = rand() % 30;
            //         msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            //         flagSleepP1 = 1;
            //         flagLeerP1 = 0;
            //         flagRegexP1 = 0;
            //     }
            // }
            // if (i==2){//Segun lo que entiendo este if se deberia ir 
            //     printf("Hola proceso a %d \n", pids[i]);
            //     if (flagSleepP2 == 1){
            //         msg.type = 1;
            //         printf("Enviando mensaje de despertar a %d \n", pids[i]);
            //         msg.posicion = 58;
            //         msg.posicion = rand() % 30;
            //         msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            //         flagSleepP2 = 0;
            //         flagLeerP2 = 1;
            // }
            //     if (flagLeerP2 == 1){
            //         msg.type = 2;
            //         printf("Enviando mensaje de leer a %d \n", pids[i]);
            //         msg.posicion = 58;
            //         msg.posicion = rand() % 30;
            //         msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            //         flagSleepP2 = 0;
            //         flagLeerP2 = 0;
            //         flagRegexP2 = 1;
            //     }
            //     if (flagRegexP2 == 1){
            //         msg.type = 3;
            //         printf("Enviando mensaje de regex a %d \n", pids[i]);
            //         msg.posicion = 58;
            //         msg.posicion = rand() % 30;
            //         msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
            //         flagSleepP2 = 1;
            //         flagLeerP2 = 0;
            //         flagRegexP2 = 0;
            //     }
            // }
            // i++;
            /*if (i==3){
                i=0;
            }*/
            }
    }
    
    //while (flagFin == 0)
    
        
        sleep(1);
    /* LECTURA DE ARCHIVO(S) */

    /* Acabar con procesos hijos*/
        kill(pids[i], SIGKILL);
        wait(&status);
    /* Acabar con procesos hijos*/

    return EXIT_SUCCESS;
}

