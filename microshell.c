/* Kacper Biedka
   UAM 
   12.01.2020 
   Microshell project
   Documented thoroughly so that I have a clue what stuff does when I come back to it :D 
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

/* Declare buffer sizes and other sys values */

/* CAT */
#define CAT_BUFFER_SIZE 1024

/* READLINE */
#define LINE_VALUES_BUFFER_SIZE 1024

/* Value Separator */
#define VALUE_SEPARATOR_BUFFER_SIZE 64

/* Delimeter for strtok */
#define VALUE_SEPARATOR_DELIMITER " \t\r\n\a"

/* Declare commands */

int sh_cd(char **args);
int sh_exit(char **args);
int sh_help(char **args);
int sh_cat(char **args);
int sh_mv(char **args);

/* String values for the commands */

char *default_str[] = {
  "cd",
  "exit",
  "help",
  "cat",
  "mv"
};

/* Functions linked to the command string values */

int (*default_func[]) (char **) = {
  &sh_cd,
  &sh_exit,
  &sh_help,
  &sh_cat,
  &sh_mv
};


/* Based on https://repl.it/@samgomena/Colored-Output */

/* Available Colors */

#define COLOR_NORM "\x1B[0m"
#define COLOR_CYAN "\x1B[36m"
#define COLOR_WHITE "\x1B[37m"
#define COLOR_GREEN "\x1B[32m"
#define COLOR_YELLOW "\x1B[33m"
#define COLOR_RED "\x1b[31m"

/* Print error and exit */

void showError () {
    fprintf(stderr, "%ssh: problem przy alokacji\n", COLOR_RED);
    exit(EXIT_FAILURE);
}

/* shell number values()
   return the number of the available commands
*/
int shell_number_values() {
  return sizeof(default_str) / sizeof(char *);
}

/* ----------- CUSTOM COMMANDS ----------- */

/* base command cd */

int sh_cd(char **args)
{	
  /* handle empty input */
  if (args[1] == NULL) {
    fprintf(stderr, "%ssh: podaj argument dla \"cd\"\n", COLOR_RED);
  } else {
  	/* change the directory based on the argument */
    if (chdir(args[1]) != 0) {
      perror("sh");
    }
  }
  return 1;
}

/* Show list of available commands */

int sh_help(char **args) {
  int i;
  printf("Kacper Biedka - microshell\n");
  printf("Podaj komende i wcisnij enter\n");
  printf("Dostepne komendy:\n");
  /* Go through the list of commands and print them in the terminal */
  for (i = 0; i < shell_number_values(); i++) {
    printf(" - %s\n", default_str[i]);
  }

  return 1;
}

/* CAT goes throught the file and writes out its content */
int sh_cat(char **args) {

  char buffer[CAT_BUFFER_SIZE];
  int inner_desc;
  int helper_num;
  char* file = args[1];

  /* Handle empty input */
  if (args[1]==NULL) {
    fprintf(stderr, "%ssh: potrzebny argument dla \"cat\"\n", COLOR_RED);
  } else {
    inner_desc=open(file,O_RDONLY);
    /* Read file an print it to the terminal */
    printf("%s[cat] %s%s zawiera:%s \n\n", COLOR_YELLOW, COLOR_GREEN, file, COLOR_WHITE);
    while ((helper_num = read(inner_desc, &buffer, CAT_BUFFER_SIZE)) > 0) {
      write(STDOUT_FILENO, &buffer, helper_num);
    }
    close(inner_desc);
  }
  return 1;
}

/* MV implementation */

int sh_mv(char **args) {
  /* Handle wrong/empty arguments */
  if (args[1] == NULL || args[2] == NULL) {
    fprintf(stderr, "%ssh: potrzebne 2 argumenty dla \"mv\"\n", COLOR_RED);
  } else {
    if (args[1] && args[2]) {
      /* If renaming the file went wrong return an error */
      if (rename(args[1], args[2]) == -1) {
         fprintf(stderr, "%ssh: nie można przenieść %s do %s\n", COLOR_RED, args[1], args[2]);
      } else {
      	printf("%s[mv] %sprzeniesiono %s do %s\n", COLOR_YELLOW, COLOR_GREEN, args[1], args[2]);
      }
    } else {
      fprintf(stderr,"%sSYNTAX ERROR: \n argumenty [stara_nazwa] [nowa_nazwa]", COLOR_RED);
    }
  }
  return 1;
}

/* Simple exit implementation */

int sh_exit(char **args) {
	char* user = getenv("USER");
	printf("%sNa razie, %s%s%s :) ! \n\n", COLOR_GREEN, COLOR_YELLOW, user, COLOR_GREEN);
  return 0;
}


int sh_start(char **args) {
  /* process id */
  pid_t pid;
  int state;
  /* Create child process */
  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      /* If execvp returns, it must have failed. */
      perror("sh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    /* fork error */
    perror("sh");
  } else {
    /* parent process */
    do {
      waitpid(pid, &state, WUNTRACED);
      /* WIFEXITED returns a nonzero value if the child process terminated normally with exit or _exit. */
      /* WIFESIGNALER return nonzero value if the child process terminated because it received a signal that was not handled. */
    } while (!WIFEXITED(state) && !WIFSIGNALED(state));
  }
  return 1;
}

/* Command execute handler */

int sh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    /* Return when the command is empty */
    return 1;
  }
  /* iterate through the commands array */
  for (i = 0; i < shell_number_values(); i++) {
    /* Compare passed argument with defined functions and execute if true */
    if (strcmp(args[0], default_str[i]) == 0) {
      return (*default_func[i])(args);
    }
  }

  return sh_start(args);
}

/* Read input */

char *get_line_values(void)
{
  int character;
  int buffer_size = LINE_VALUES_BUFFER_SIZE;
  int posCount = 0;
  /* allocate memory based on buffer value and char size */
  char *buffer = malloc(sizeof(char) * buffer_size);
  if (!buffer) {
    showError();
  }

  while (1) {
    /* Get character */
    character = getchar();

    if (character == EOF) {
      exit(EXIT_SUCCESS);
    /* Check if the character is not a line break */
    } else if (character == '\n') {
      buffer[posCount] = '\0';
      return buffer;
    } else {
      buffer[posCount] = character;
    }
    posCount++;

    /* Realocate when the posCount exceeds buffer_size */
    if (posCount >= buffer_size) {
      /* Get defined buffer size and reallocate */
      buffer_size += LINE_VALUES_BUFFER_SIZE;
      buffer = realloc(buffer, buffer_size);
      if (!buffer) {
        /* Handle error and dispatch */
        showError();
      }
    }
  }
}

/* separate_values() 
   this function gets words from args that will be later compared to default command in different function
*/

char **separate_values(char *line) {
  int bufsize = VALUE_SEPARATOR_BUFFER_SIZE, position = 0;
  /* Assign memory based on buffer size and character size */
  char **values = malloc(bufsize * sizeof(char*));
  char *value, **values_temp;
  if (!values) {
    /* Handle empty value */
    showError();
  }
  /* Get words from string that will be later compared to function names */ 
  value = strtok(line, VALUE_SEPARATOR_DELIMITER);
  /* Iterate through word as long as the character at given index is not empty*/
  while (value != NULL) {
    /* Increment and go again */
    values[position] = value;
    position++;
    if (position >= bufsize) {
      bufsize += VALUE_SEPARATOR_BUFFER_SIZE;
      values_temp = values;
      /* Change memory size that was alocated earlier by malloc */
      values = realloc(values, bufsize * sizeof(char*));
      if (!values) {
        /* Free memory of uneccessary values */
		    free(values_temp);
        showError();
      }
    }
    /* Get words from string that will be later compared to function names */ 
    value = strtok(NULL, VALUE_SEPARATOR_DELIMITER);
  }
  values[position] = NULL;
  return values;
}

/* Console display with custom colors */

/* REPL loop */
void iterate(void) {
  /* Passed values */
  char *currentLine;
  char **args;
  int state;
  /* Get environment values */
	char* user_name = getenv("USER");
	char* path = getenv("PWD");
	char* machine_name = getenv("COMPUTERNAME");
	/* This will run if the passed argument matches any of the defined fuction names */
  do {
    /* Print the environment values follower by the $ sign */
    printf("%s%s@%s%s:%s%s%s$ ",COLOR_YELLOW,user_name,machine_name,COLOR_NORM,COLOR_CYAN,path,COLOR_WHITE);
    /* Handle passed character */
    currentLine = get_line_values();
    /* Separates passed string and assigns memory */
    args = separate_values(currentLine);
    /* Check if the value corresponds to any declared function name */
    state = sh_execute(args);

    /* Free the memory */ 
    free(currentLine);
    free(args);
  } while (state);
}


int main(int argc, char **argv) {

  /* Run iterate function that will listen for inputs and compare them with declared function names (REPL loop) */
  iterate();
  return EXIT_SUCCESS;
}
