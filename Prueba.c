#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/msg.h>

#define BUFFER_SIZE 8192

struct buffer {
    char *data;
    int size;
};

struct message {
    int type;
    int pid;
    struct buffer buffer;
};

int main(int argc, char *argv[]) {
    // Validar los argumentos
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <expresion_regular>\n", argv[0]);
        exit(1);
    }

    // Crear la expresión regular
    regex_t re;
    int ret = regcomp(&re, argv[1], REG_EXTENDED);
    if (ret != 0) {
        fprintf(stderr, "Error al compilar la expresión regular: %s\n", regerror(ret, &re, NULL, 0));
        exit(1);
    }

    // Abrir el archivo
    int fd = open(argv[2], O_RDONLY);
    if (fd < 0) {
        perror("Error al abrir el archivo");
        exit(1);
    }

    // Crear la estructura de datos circular para los buffers
    struct buffer *buffers = malloc(sizeof(struct buffer) * BUFFER_SIZE);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffers[i].data = malloc(BUFFER_SIZE);
        buffers[i].size = 0;
    }

    // Crear la cola de mensajes
    int qid = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    if (qid < 0) {
        perror("Error al crear la cola de mensajes");
        exit(1);
    }

    // Crear los procesos hijos
    pid_t pids[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            // Código del proceso hijo
            process_loop(fd, buffers, qid, i);
            exit(0);
        }
    }

    // Iniciar la búsqueda
    struct message msg;
    msg.type = 1;
    msg.pid = getpid();
    msgsnd(qid, &msg, sizeof(msg), 0);

    // Esperar a que los procesos terminen
    for (int i = 0; i < BUFFER_SIZE; i++) {
        wait(NULL);
    }

    // Liberar la memoria
    for (int i = 0; i < BUFFER_SIZE; i++) {
        free(buffers[i].data);
    }
    free(buffers);

    // Cerrar la cola de mensajes
    msgctl(qid, IPC_RMID, NULL);

    // Destruir la expresión regular
    regfree(&re);

    return 0;
}

void process_loop(int fd, struct buffer *buffers, int qid, int pid) {
    // Recibir el mensaje de inicio
    struct message msg;
    msgrcv(qid, &msg, sizeof(msg), 2, 0);

    // Leer el archivo
    while (1) {
        // Solicitar un nuevo buffer
        msg.type = 3;
        msgsnd(qid, &msg, sizeof(msg), 0);

        // Leer una línea del buffer
        int size = read(fd, buffers[pid].data, BUFFER_SIZE);
        if (size == 0) {
            // Fin del archivo
            break;
        }

        // Enviar la línea al padre
        msg.type = 4;
        msg.buffer = buffers[pid];
        msg.buffer.size = size;
        msgsnd(qid, &msg, sizeof(msg), 0);
    }
