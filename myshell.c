#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_rhistory(char **args);
int j, k;

char rhistory[1000];
int recordok;

int hcnt = 0, st = 0;
char *builtin_str[] = {
   "cd",
   "help",
   "exit",
   "rhistory"
};

int(*builtin_func[]) (char **) = {
   &lsh_cd,
   &lsh_help,
   &lsh_exit,
   &lsh_rhistory
};

int lsh_num_builtins() {
   return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args)
{
   if (args[1] == NULL) {
      fprintf(stderr, "lsh: expected argument to \"cd\"\n");
   }
   else {
      if (chdir(args[1]) != 0) {
         perror("lsh");
      }
      system("pwd");
      system("ls");
   }
   return 1;
}

int lsh_help(char **args)
{
   int i;
   printf("Stephen Brennan's LSH\n");
   printf("Type program names and arguments, and hit enter.\n");
   printf("The following are built in:\n");

   for (i = 0; i < lsh_num_builtins(); i++) {
      printf("  %s\n", builtin_str[i]);
   }

   printf("Use the man command for information on other programs.\n");
   return 1;
}

int lsh_rhistory(char **args){
   if (strcmp(args[0], "rhistory") == 0) {
      st = 1;
      printf("(!Notice)[History_For_Your_Commends] : ");
      for (k = 0; k < hcnt; k++) {
         if (rhistory[k] == ';' && k + 1 < hcnt) {
            printf("\n");
            printf("[%d] : ", st++);
         }
         else if (rhistory[k] != ';')
            printf("%c", rhistory[k]);
      }
      printf("\n");
   }
   return 1;
}

int lsh_exit(char **args)
{
   return 0;
}

int lsh_launch(char **args)
{
   pid_t pid;
   int status;

   pid = fork();
   if (pid == 0) {

      if (execvp(args[0], args) == -1) {
         perror("lsh");
      }
      exit(EXIT_FAILURE);
   }
   else if (pid < 0) {

      perror("lsh");
   }
   else {

      do {
         waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
   }

   return 1;
}

int lsh_execute(char **args)
{
   int i;

   if (args[0] == NULL) {

      return 1;
   }

   for (i = 0; i < lsh_num_builtins(); i++) {
      if (strcmp(args[0], builtin_str[i]) == 0) {
         return (*builtin_func[i])(args);
      }
   }

   return lsh_launch(args);
}

#define LSH_RL_BUFSIZE 1024

char *lsh_read_line(void)
{
   int bufsize = LSH_RL_BUFSIZE;
   int position = 0;
   char *buffer = malloc(sizeof(char) * bufsize);
   int c;

   if (!buffer) {
      fprintf(stderr, "lsh: allocation error\n");
      exit(EXIT_FAILURE);
   }

   while (1) {

      c = getchar();

      if (c == EOF) {
         exit(EXIT_SUCCESS);
      }
      else if (c == '\n') {
         buffer[position] = '\0';
         return buffer;
      }
      else {
         buffer[position] = c;
      }
      position++;

      if (position >= bufsize) {
         bufsize += LSH_RL_BUFSIZE;
         buffer = realloc(buffer, bufsize);
         if (!buffer) {
            fprintf(stderr, "lsh: allocation error\n");
            exit(EXIT_FAILURE);
         }
      }
   }
}
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char **lsh_split_line(char *line)
{
   int bufsize = LSH_TOK_BUFSIZE, position = 0;
   char **tokens = malloc(bufsize * sizeof(char*));
   char *token, **tokens_backup;

   if (!tokens) {
      fprintf(stderr, "lsh: allocation error\n");
      exit(EXIT_FAILURE);
   }

   token = strtok(line, LSH_TOK_DELIM);
   while (token != NULL) {
      tokens[position] = token;
      position++;

      if (position >= bufsize) {
         bufsize += LSH_TOK_BUFSIZE;
         tokens_backup = tokens;
         tokens = realloc(tokens, bufsize * sizeof(char*));
         if (!tokens) {
            free(tokens_backup);
            fprintf(stderr, "lsh: allocation error\n");
            exit(EXIT_FAILURE);
         }
      }

      token = strtok(NULL, LSH_TOK_DELIM);
   }
   tokens[position] = NULL;
   return tokens;
}

void lsh_loop(void)
{
   char *line;
   char **args;
   int status;
   
   do {
      printf("> ");
      line = lsh_read_line();
      args = lsh_split_line(line);
      
      for (k = 0; k<100; k++) {
         if (line[k] != '\0') {
            rhistory[++hcnt] = line[k];
         }
      }
      rhistory[0]=';';
      rhistory[++hcnt]=';';
      status = lsh_execute(args);

      free(line);
      free(args);
   } while (status);
}

int main(int argc, char **argv)
{

   lsh_loop();

   return EXIT_SUCCESS;
}