//$x ./cbang
//$ gcc cbang.c -o cbang && cbang
//$c gcc cbang.c -o cbang

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// linux-specific
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

#define CMDLEN 256
static char buildcmd[CMDLEN] = {0};
static char execcmd[CMDLEN] = {0};
static char defaultcmd[CMDLEN] = {0};


void usage(FILE *f, const char *program) {
     fprintf(f, "USAGE: %s <build|exec> [file]", program);
}



int main(int argc, char **argv) {
     const char *program_name = *argv++;
     int building = 0, executing = 0;

     if (!*argv) {
	  usage(stderr, program_name);
	  return -1;
     }
     
     if (strcmp(*argv, "build") == 0) {
	  building = 1;
	  argv++;
     } else if (strcmp(*argv, "exec") == 0) {
	  executing = 1;
	  argv++;
     }
     // hopefully *argv now is a valid filename
     if (!*argv) {
	  assert((building || executing) && "something is buggy");
	  
	  fprintf(stderr, "missing argument(s) to flag \"%s\"\n", argv[-1]);
	  return -1;
     }

     
     int fd = open(*argv, O_RDONLY);
     if (fd < 0) {
	  fprintf(stderr, "invalid file name: \"%s\"\n", *argv);
	  perror("open");
	  return -1;
     }
     
     struct stat st;
     if (fstat(fd, &st) < 0) {
	  perror("fstat");
	  if (close(fd) < 0) 
	       perror("close");
	  
	  return -1;
     }

     if (st.st_size == 0) {
	  fprintf(stderr, "empty file\n");
	  if (close(fd) < 0) 
	       perror("close");
	  return -1;
     }
     
     char *src = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
     if (src == MAP_FAILED) {
	  perror("mmap");
	  if (close(fd) < 0) 
	       perror("close");

	  return -1;	  
     }
     
     char *end = src + st.st_size;
     char *iter = src;
     for (int first = 1; iter < end; first = 0) {
	  char *tmp = memchr(iter, '\n', end - iter);
	  
	  if (!first && tmp) {
	       iter = tmp;
	       iter++;
	  }
	  
	  if (memcmp(iter, "//$", 3) == 0) {
	       char *command = iter + 3;
	       if (isspace(*command)) {
		    command++;
		    int n = 0;
		    while (n < CMDLEN && *command != '\n')
			 defaultcmd[n++] = *command++;
	       }
	       if (*command == 'x') {
		    command++;
		    if (!isspace(*command)) {
			 printf("unknown flag\n");
			 return -1;
		    }
		    command++;
		    int n = 0;
		    while (n < CMDLEN && *command != '\n')
			 execcmd[n++] = *command++;
		    
	       }
	       if (*command == 'c') {
		    command++;
		    if (!isspace(*command)) {
			 printf("unknown flag\n");
			 return -1;
		    }
		    command++;
		    int n = 0;
		    while (n < CMDLEN && *command != '\n')
			 buildcmd[n++] = *command++;
		    
	       }
	       
	  }
     }
     
     // printf("buildcmd = %s\nexeccmd = %s\ndefaultcmd = %s", buildcmd, execcmd, defaultcmd);
     
     const char *cmd =
	  building ? buildcmd :
	  executing ? execcmd :
	  defaultcmd;

     if (system(cmd)) {
	  perror("system");
	  return -1;
     }
}
