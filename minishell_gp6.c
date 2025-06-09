/*Integrantes Grupo 6:
João Victor Vasconcelos Junqueira Criscuolo - 22024547
João Vitor de Moraes Marcelino França - 20068995
Vinicius Felippe Dan Albieri - 23017528
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_LINE 1024
#define MAX_ARGS 100

char *path[MAX_ARGS] = {NULL};  //Caminhos definidos pelo comando `path`
char cwd[MAX_LINE];

// Exibe a mensagem de ajuda com os comandos disponíveis
void show_help() {
    printf("---------------------------------------------------------------------------\n");
    printf("Bem vindo ao MiniShell do Grupo 6!\n");
    printf("Comandos disponíveis:\n");
    printf("  exit                  - Sair do Shell\n");
    printf("  cd <path>             - Altera o diretório atual\n");
    printf("  path <dir> [<dir>...] - Define caminho(s) para busca de executáveis\n");
    printf("  pwd                   - Exibe o diretório atual\n");
    printf("  cat <file>            - Exibe o conteúdo de um arquivo\n");
    printf("  ls [-l] [-a]          - Lista o conteúdo do diretório atual\n");
    printf("  help                  - Lista todas as funcionalidades disponíveis\n");
    printf("Você pode executar programas externos se estiverem no path definido ou pelo caminho completo.\n");
}

//Faz o parsing da entrada do usuário, dividindo-a em argumentos
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

// Libera a memória de caminhos anteriores do path
void clear_path() {
    for (int i = 0; path[i] != NULL; i++) {
        free(path[i]);
        path[i] = NULL;
    }
}

// Executa comandos internos do shell
int execute_internal_command(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL || args[2] != NULL) {
            fprintf(stderr, "cd: insira um argumento válido\n");
        } else if (chdir(args[1]) != 0) {
            perror("cd");
        }
        return 1;
    } else if (strcmp(args[0], "path") == 0) {
        clear_path(); // limpar caminhos antigos
        for (int i = 1; i < MAX_ARGS && args[i] != NULL; i++) {
            path[i - 1] = strdup(args[i]);
        }
        return 1;
    } else if (strcmp(args[0], "pwd") == 0) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            // printf("---------------------------------------------------------------------------\n");
            printf("%s\n", cwd);
            // printf("---------------------------------------------------------------------------\n");

        } else {
            perror("pwd");
        }
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
            if (strcmp(args[i], "-l") == 0) l_flag = 1;
            else if (strcmp(args[i], "-a") == 0) a_flag = 1;
        }

        //estrutura de diretório
        struct dirent *de;

        //diretório atual (".")
        DIR *dr = opendir(".");
        if (dr == NULL) {
            perror("ls");  //exibe erro 
            return 1;
        }

        //le entrada do diretório atual
        while ((de = readdir(dr)) != NULL) {
            //se a opção -a nao foi usada e o arquivo é oculto = pula
            if (!a_flag && de->d_name[0] == '.') continue;

            //se a opção -l
            if (l_flag) {
                struct stat st;

                //obter informações sobre o arquivo
                if (stat(de->d_name, &st) == -1) {
                    perror("stat");
                    continue;
                }

                //tipo de arquivo: diretório ('d') ou arquivo comum ('-')
                printf("%c", S_ISDIR(st.st_mode) ? 'd' : '-');

                //Permissões de usuário, grupo e outros
                printf("%c", (st.st_mode & S_IRUSR) ? 'r' : '-');
                printf("%c", (st.st_mode & S_IWUSR) ? 'w' : '-');
                printf("%c", (st.st_mode & S_IXUSR) ? 'x' : '-');
                printf("%c", (st.st_mode & S_IRGRP) ? 'r' : '-');
                printf("%c", (st.st_mode & S_IWGRP) ? 'w' : '-');
                printf("%c", (st.st_mode & S_IXGRP) ? 'x' : '-');
                printf("%c", (st.st_mode & S_IROTH) ? 'r' : '-');
                printf("%c", (st.st_mode & S_IWOTH) ? 'w' : '-');
                printf("%c", (st.st_mode & S_IXOTH) ? 'x' : '-');

                //num de links para o arquivo (para diretórios)
                printf(" %lu", st.st_nlink);

                //tamanho do arquivo em bytes
                printf(" %lu", st.st_size);

                //Data/hora da última modificação formatada
                char time[20];
                strftime(time, sizeof(time), "%b %d %H:%M", localtime(&st.st_mtime));
                printf(" %s", time);

                //nome do arquivo
                printf(" %s\n", de->d_name);
            } else {
                //se não pediu -l, imprime o nome do arquivo
                printf("%s\n", de->d_name);
            }
        }
        closedir(dr);
        return 1;
    }
    return 0;
}

//Executa comandos externos
void execute_external_command(char **args) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        char full_path[MAX_LINE];
        int found = 0;

        for (int i = 0; path[i] != NULL; i++) {
            snprintf(full_path, sizeof(full_path), "%s/%s", path[i], args[0]);
            if (access(full_path, X_OK) == 0) {
                execv(full_path, args);
                perror("execv");  // se falhar
                exit(1);
            }
        }

        //se não encontrado nos caminhos definidos, tenta diretamente
        execv(args[0], args);
        perror("execv");
        exit(1);
    } else {
        waitpid(pid, NULL, 0);
    }
}

// Modo interativo do shell
void interactive_mode() {
    char input[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        printf("---------------------------------------------------------------------------\n");
        printf("shell> ");
        if (fgets(input, MAX_LINE, stdin) == NULL) {
            printf("\n\n");
            break;
        }

        parse_input(input, args);

        if (args[0] == NULL) continue;

        if (!execute_internal_command(args)) {
            execute_external_command(args);
        }
    }
}

//batch do shell
void batch_mode(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }

    char input[MAX_LINE];
    char *args[MAX_ARGS];

    while (fgets(input, sizeof(input), file) != NULL) {
        parse_input(input, args);

        if (args[0] == NULL) continue;

        printf(">> %s", input);

        if (!execute_internal_command(args)) {
            execute_external_command(args);
        }
    }

    fclose(file);
}

//inicializa o shell
int main(int argc, char *argv[]) {
    show_help();
    if (argc == 1) {
        interactive_mode();
    } else if (argc == 2) {
        batch_mode(argv[1]);
    } else {
        fprintf(stderr, "Uso: %s [batchfile]\n", argv[0]);
        exit(1);
    }

    return 0;
}
