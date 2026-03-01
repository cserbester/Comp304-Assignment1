// cut.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static int parse_fields(const char *s, int **out_fields) { // to parse comma fields
    int count = 1; // comma count
    for (const char *p = s; *p; p++){
        if (*p == ','){
            count++;
        }
    }

    int *fields = malloc(sizeof(int) * count); //alocate an array to stroe field incdicies
    if (!fields) {
        return -1;
    }

    char *copy = strdup(s); // create a copy
    if (!copy) {
        free(fields);
        return -1;
    }

    int n = 0;
    for (char *tok = strtok(copy, ","); tok; tok = strtok(NULL, ",")) { // split by comma
        char *end;
        long v = strtol(tok, &end, 10);
        if (*tok == '\0' || *end != '\0' || v <= 0) {
            free(copy);
            free(fields);
            return -1;
        }
        fields[n++] = (int)v;
    }

    free(copy);
    *out_fields = fields; // return the array
    return n; // return the count
}

int main(int argc, char **argv) {
    char delim = '\t';            // default delimiter tab
    const char *fields_str = NULL;

    static struct option long_opts[] = { // other options
        {"delimiter", required_argument, 0, 'd'},
        {"fields",    required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "d:f:", long_opts, NULL)) != -1) { //parse options
        if (opt == 'd') {
            
            if (!optarg || optarg[0] == '\0' || optarg[1] != '\0'){
                return 1;
            }
            delim = optarg[0];
        } else if (opt == 'f') {
            fields_str = optarg;
        } else {
            return 1;
        }
    }

    if (!fields_str) {
        return 1;
    }

    int *fields = NULL;
    int field_count = parse_fields(fields_str, &fields); // parse the field list
    if (field_count <= 0) {
        return 1;
    }

    char *line = NULL;
    size_t cap = 0;

    while (getline(&line, &cap, stdin) != -1) { // read the lines
        // remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n'){
            line[len - 1] = '\0';
        }

        // split into fields
        int parts_cap = 16;
        int parts_len = 0;
        char **parts = malloc(sizeof(char*) * parts_cap);
        if (!parts) {
            free(fields);
            free(line);
            return 1;
        }

        parts[parts_len++] = line;
        for (char *p = line; *p; p++) {
            if (*p == delim) {
                *p = '\0';
                if (parts_len == parts_cap) {
                    parts_cap *= 2;
                    char **tmp = realloc(parts, sizeof(char*) * parts_cap);
                    if (!tmp) {
                        free(parts);
                        free(fields);
                        free(line);
                        return 1;
                    }
                    parts = tmp;
                }
                parts[parts_len++] = p + 1;
            }
        }

        // print only requested fields in the order specified
        for (int i = 0; i < field_count; i++) {
            int idx = fields[i] - 1;
            if (i > 0) {
                putchar(delim);
            }
            if (idx >= 0 && idx < parts_len){
                fputs(parts[idx], stdout);
            }
        }
        putchar('\n');

        free(parts);
    }

    free(fields);
    free(line);
    return 0;
}
