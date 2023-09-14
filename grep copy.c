#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

void grep(const char *pattern, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "No se pudo abrir el archivo '%s'\n", filename);
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

    while (fgets(line, sizeof(line), file)) {
        if (regexec(&regex, line, 0, NULL, 0) == 0) {
            printf("%s", line);
        }
    }

    fclose(file);
    regfree(&regex);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s 'expresion_regular' archivo1 archivo2 ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *pattern = argv[1];

    for (int i = 2; i < argc; i++) {
        grep(pattern, argv[i]);
    }

    return EXIT_SUCCESS;
}

