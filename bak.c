/* bak.c - Create numbered bak files before editing them
   Usage: bak [file...]
 */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef int fd;
int cp(fd, fd); 

int main(int argc, char **argv) {
     if (argc < 2)
	  return -1;     

     (void)argc;


     
     char namebuf[PATH_MAX + sizeof "bak.99"];
     
     const char *program = *argv++;
     for (const char *arg; (arg = *argv) != NULL; argv++) {
	  char path[PATH_MAX];
	  if (realpath(arg, path) == NULL)
	       return -1;
	  
	  if (arg[0] == '-') {
	       // parse_command(arg);
	  }
	  
	  int i = 0;
	  snprintf(namebuf, sizeof(namebuf), "%s.bak.%d", path, i);
	  while (access(namebuf, F_OK) == 0 && i < 100) {
	       i++;
	       snprintf(namebuf, sizeof(namebuf), "%s.bak.%d", path, i);
	  }

	  fd out = creat(namebuf, 0644);
	  if (out < 0)
	       return -1;

	  fd in = open(path, O_RDONLY);
	  if (in < 0) {
	       close(out);
	       return -1;
	  }

	  
	  if (cp(in, out) < 0)
	       return -1;
	  close(in);
	  close(out);

     }
     return 0;
}

int cp(fd in, fd out) {
    char buf[1024];

    for (;;) {
        ssize_t n = read(in, buf, sizeof buf);

        if (n == 0)
            return 0;

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        ssize_t off = 0;

        while (off < n) {
            ssize_t w = write(out, buf + off, (size_t)(n - off));

            if (w < 0) {
                if (errno == EINTR)
                    continue;
                return -1;
            }

            if (w == 0) {
                errno = EIO;
                return -1;
            }

            off += w;
        }
    }
}
