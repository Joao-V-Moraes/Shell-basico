/*Integrantes do grupo 16
Bruno Lima Murakami                     RA:20124673
João Vitor de Moraes Marcelino França   RA:20068995
Lucas Henrique Koda da Silva            RA:20018156
Matheus Augusto Costa                   RA:19713866
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_LINE 1024
#define MAX_ARGS 100

char *path[MAX_ARGS] = {NULL};  // Caminhos definidos pelo comando `path`
char cwd[MAX_LINE];

// Exibe a mensagem de ajuda com os comandos disponíveis
void show_help() {
    printf("Bem vindo ao Shell do Grupo 16!\n");
    printf("Comando disponiveis:\n");
    
    printf("  exit                  - Sair do Shell\n");
    printf("  cd <path>             - Altera o diretorio atual para o <path> inserido\n");
    printf("  path <dir> [<dir>...] - Define caminho(s) para busca de executáveis\n");
    printf("  dir                   - Lista todos os diretorios possiveis a partir do atual\n");
    printf("  cat <file>            - Exibe o conteúdo do <file>\n");
    printf("  ls [-l] [-a]          - Lista o conteúdo do diretório atual com opções\n");
    printf("  help                  - Lista todas as funcionalidades disponíveis\n");

    printf("Você pode executar programas externos caso seja passado todo o caminho.\n");
}

// Faz o parsing da entrada do usuário, dividindo-a em argumentos
void parse_input(char *input, char **args) {
    char *token;
    int i = 0;

    token = strtok(input, " \t\n");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

// Executa comandos internos do shell
int execute_internal_command(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: argumento faltando \n");
        } else if (chdir(args[1]) != 0) {
            perror("cd");
        }
        return 1;
    } else if (strcmp(args[0], "path") == 0) {
        for (int i = 1; i < MAX_ARGS; i++) {
            if (args[i] == NULL) {
                path[i - 1] = NULL;
                break;
            }
            path[i - 1] = strdup(args[i]);
        }
        printf("Caminho adicionado com sucesso!\n");
        return 1;
    } else if (strcmp(args[0], "dir") == 0) {
        getcwd(cwd, sizeof(cwd));
        printf("Diretorio atual: %s \n", cwd);
        printf("--------------------------------------------------------------------------------\n"); 
        system("ls");
        printf("--------------------------------------------------------------------------------\n"); 
        return 1;
    } else if (strcmp(args[0], "help") == 0) {
        show_help();
        return 1;
    } else if (strcmp(args[0], "cat") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cat: argumento faltando\n");
        } else {
            FILE *file = fopen(args[1], "r");
            if (file == NULL) {
                perror("cat");
                return 1;
            }
            char buffer[MAX_LINE];
            while (fgets(buffer, MAX_LINE, file) != NULL) {
                printf("%s", buffer);
            }
            fclose(file);
        }
        return 1;
    } else if (strcmp(args[0], "ls") == 0) {
        int l_flag = 0;
        int a_flag = 0;
        for (int i = 1; args[i] != NULL; i++) {
            if (strcmp(args[i], "-l") == 0) {
                l_flag = 1;
            } else if (strcmp(args[i], "-a") == 0) {
                a_flag = 1;
            }
        }
        struct dirent *de;
        DIR *dr = opendir(".");
        if (dr == NULL) {
            perror("ls");
            return 1;
        }
        while ((de = readdir(dr)) != NULL) {
            if (!a_flag && de->d_name[0] == '.') {
                continue;
            }
            if (l_flag) {
                struct stat st;
                if (stat(de->d_name, &st) == -1) {
                    perror("stat");
                    continue;
                }
                printf("%c", S_ISDIR(st.st_mode) ? 'd' : '-');
                printf("%c", (st.st_mode & S_IRUSR) ? 'r' : '-');
                printf("%c", (st.st_mode & S_IWUSR) ? 'w' : '-');
                printf("%c", (st.st_mode & S_IXUSR) ? 'x' : '-');
                printf("%c", (st.st_mode & S_IRGRP) ? 'r' : '-');
                printf("%c", (st.st_mode & S_IWGRP) ? 'w' : '-');
                printf("%c", (st.st_mode & S_IXGRP) ? 'x' : '-');
                printf("%c", (st.st_mode & S_IROTH) ? 'r' : '-');
                printf("%c", (st.st_mode & S_IWOTH) ? 'w' : '-');
                printf("%c", (st.st_mode & S_IXOTH) ? 'x' : '-');
                printf(" %lu", st.st_nlink);
                printf(" %lu", st.st_size);
                char time[20];
                strftime(time, 20, "%b %d %H:%M", localtime(&(st.st_mtime)));
                printf(" %s", time);
                printf(" %s\n", de->d_name);
            } else {
                printf("%s\n", de->d_name);
            }
        }
        closedir(dr);
        return 1;
    }
    return 0;
}

// Executa comandos externos
void execute_external_command(char **args) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) {
        char cmd[MAX_LINE] = "";
        char full_path[MAX_LINE];
        int success = 0;

        // Concatena os argumentos em uma única string
        for (int i = 0; args[i] != NULL; i++) {
            strcat(cmd, args[i]);
            strcat(cmd, " ");
        }

        // Tenta encontrar o executável nos diretórios especificados no path
        for (int i = 0; path[i] != NULL; i++) {
            snprintf(full_path, sizeof(full_path), "%s/%s", path[i], args[0]);
            if (access(full_path, X_OK) == 0) {
                success = 1;
                break;
            }
        }

        // Se não encontrar no path, tenta diretamente
        if (!success) {
            strcpy(full_path, args[0]);
        }

        execvp(full_path, args);
        perror("execvp");
        exit(1);
    } else {
        wait(NULL);
    }
}

// Modo interativo do shell
void interactive_mode() {
    char input[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        printf("shell> ");
        if (fgets(input, MAX_LINE, stdin) == NULL) {
            perror("fgets falhou");
            exit(1);
        }

        parse_input(input, args);

        if (args[0] == NULL) {
            continue; // Comando vazio
        }

        if (!execute_internal_command(args)) {
            execute_external_command(args);
        }
    }
}

// Modo batch do shell
void batch_mode(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen falhou");
        exit(1);
    }

    char input[MAX_LINE];
    char *args[MAX_ARGS];

    while (fgets(input, MAX_LINE, file) != NULL) {
        parse_input(input, args);

        if (args[0] == NULL) {
            continue; // Comando vazio
        }

        if (!execute_internal_command(args)) {
            execute_external_command(args);
        }
    }

    fclose(file);
}

// Função principal que inicializa o shell
int main(int argc, char *argv[]) {
    show_help();
    if (argc == 1) {
        // Modo interativo
        interactive_mode();
    } else if (argc == 2) {
        // Modo batch
        batch_mode(argv[1]);
    } else {
        fprintf(stderr, "Uso: %s [batchfile]\n", argv[0]);
        exit(1);
    }

    return 0;
}
