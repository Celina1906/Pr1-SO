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
#include <time.h>

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
    
    memset(msg.buffer.data, 0, sizeof(msg.buffer.data)); // Inicializa el buffer con ceros
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo '%s'\n", filename);
        exit(EXIT_FAILURE);
    }
     if (fseek(file, posicion, SEEK_SET) != 0) {
        //Revisa que la posicion dada como parametro sea valida para iniciar la lectura del proceso
        fprintf(stderr, "No se pudo establecer la posición de lectura\n");
        exit(EXIT_FAILURE);
    }
    
    // Lee el archivo carácter por carácter
    char caracter;
    long int indice = 0; //indice mantiene un conteo de la cantidad de caracteres que va leyendo el proceso
    while ((caracter = fgetc(file)) != EOF) {
        
        msg.buffer.data[indice] = caracter;
        indice++;

        // Verifica si el buffer se llena
        if (indice >= sizeof(msg.buffer.data) - 1) {
            
            msg.tipoAccion = 1;
            break;
        }
    }
    if ((caracter = fgetc(file)) == EOF){
        msg.tipoAccion = 2;
        //Si el ultimo caracter marca final de archivo se debe de terminar con los procesos y dar por finalizado el programa
            //Accion que se realizara dentro del proceso padre tras la revision de tipoAccion
    }
    
    msg.posicion = ftell(file);
    // Imprime la línea leída y la posición en el archivo
    fclose(file);

}

int main(int argc, char *argv[]) {
    printf("Inicio del programa\n");
    printf("\n");
    time_t begin = time(NULL);
    /* Revisar parametros */
    if (argc < 3) {
        fprintf(stderr, "Uso: %s 'expresion_regular' archivo1 archivo2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    regex_t regex;
    int reti;
    reti = regcomp(&regex, argv[1], REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "No se pudo compilar la expresión regular\n");
        exit(EXIT_FAILURE);
    }
    /* Revisar parametros */
    /* Crear Pool de Hijos */
    int cantidadHijos=3;
    int status, i;
    pid_t pids[3];
    key_t msqkey = 999;
    int msqid = msgget(msqkey, IPC_CREAT | S_IRUSR | S_IWUSR);
    for (i = 0; i < cantidadHijos; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            int childNumber = i + 1;
            while (1) {
                if (msgrcv(msqid, &msg, sizeof(struct message), childNumber, 0) > 0) { //Revisar proceso hijo
                    leer(argv[msg.numeroArchivo], msg.posicion);
                    msg.type =4; //definimos mensaje type 4 para comunicar con proceso padre.
                    msg.numeroProceso = i;
                    //Se define el numero de proceso para que el padre defina, a partir de quien termino de leer, quien puede continuar con la lectura
                    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);
                    //Se envia un mensaje comunicando la posicion de lectura y quien estuvo leyendo
                    if (regexec(&regex, msg.buffer.data, 0, NULL, 0) == 0) {
                        //se analiza el segmento leido y si se encuentra una coincidencia con el patron de expresion regular se notifica al padre
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

    /* Proceso Padre */
    //Inicializa el proceso 0 para comenzar con la lectura
    int childNumber = 1;
    msg.type = childNumber;
    msg.numeroProceso = 0;
    msg.posicion = 0;
    msg.numeroArchivo = 2; 
    msgsnd(msqid, (void *)&msg, sizeof(struct message) , IPC_NOWAIT);

    while(!finArchivo){
        msgrcv(msqid, &msg, sizeof(struct message) , 4, 0);
        sleep(1);
        /*
        Tipos de Accion segun mensaje type 4
            - 1 = Continuar lectura --> No se ha llegado al final del archivo por lo que se debe asignar un proceso para continuar desde la ultima posicion
            - 2 = Se llego al final del archivo, por lo que se debe de finalizar el proceso y sus hijos.
            - 3 = Alguno de los procesos hijos encontro una coincidencia con el patron de expresiones regulares y se lo comunica al padre para su impresion
        */        
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
            time_t end = time(NULL);
            printf("\n");
            printf("Fin del programa\n");
            printf("El programa tomó %ld segundos en ejecutarse.\n", (end - begin));
            finArchivo = true;
            break;  
            // return 0;

            
        }
        else if(msg.tipoAccion ==3){
            printf("%s\n", msg.buffer.data);
        }
        
    }

    for (int i = 0; i < cantidadHijos; i++) {
        //Inicia la accion para finalizar  a los procesos hijos
        kill(pids[i], SIGKILL);
        wait(&status);
    }
    /* Proceso Padre */
    
    return EXIT_SUCCESS;
}