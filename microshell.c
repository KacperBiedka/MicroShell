#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

/* REPL loop */
void iterate(void) {
  /* Get environment values */
	char* user_name = getenv("USER");
	char* path = getenv("PWD");
	char* machine_name = getenv("COMPUTERNAME");
    printf("%s@%s:%s$ ", user_name,machine_name,path);
}

int main(int argc, char **argv) {

  iterate();

  return EXIT_SUCCESS;
}