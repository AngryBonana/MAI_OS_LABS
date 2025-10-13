#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


static char SERVER_PROGRAM_NAME[] = "server";

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        char msg[1024];
        uint32_t len = snprintf(msg, sizeof(msg) - 1, "usage: %s filename\n", argv[0]);
        write(STDERR_FILENO, msg, len);
        exit(EXIT_FAILURE);
    }

    char progpath[1024];
    {
        ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
        
        if (len == -1) {
            const char msg[] = "error: failed to read full program path\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }
        
        while (progpath[len] != '/') {
            --len;
        }
        
        progpath[len] = '\0';
    }

    int pipe_client_to_child[2];
    int pipe_child_to_client[2];

    if (pipe(pipe_client_to_child) == -1)
    {
        const char msg[] = "error: failed to create pipe client to server\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    if (pipe(pipe_child_to_client) == -1)
    {
        const char msg[] = "error: failed to create pipe server to client\n";
        write(STDERR_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }

    const pid_t child = fork();

    switch (child) {
        case -1:
        {
            const char msg[] = "error: failed to spawn new process\n";
		    write(STDERR_FILENO, msg, sizeof(msg));
		    exit(EXIT_FAILURE);
        }
        break;

        case 0: 
        {
            close(pipe_client_to_child[1]);
		    close(pipe_child_to_client[0]);

		    dup2(pipe_client_to_child[0], STDIN_FILENO);
		    close(pipe_client_to_child[0]);

		    dup2(pipe_child_to_client[1], STDOUT_FILENO);
		    close(pipe_child_to_client[1]);

		    {
			    char path[1024];
			    snprintf(path, sizeof(path) - 1, "%s/%s", progpath, SERVER_PROGRAM_NAME);

			    char *const args[] = {SERVER_PROGRAM_NAME, argv[1], NULL};

			    int32_t status = execv(path, args);

			    if (status == -1)
                {
			        const char msg[] = "error: failed to exec into new exectuable image\n";
			        write(STDERR_FILENO, msg, sizeof(msg));
				    exit(EXIT_FAILURE);
			    }
            }
        }
        break;

        default:
        {
            {
			    pid_t pid = getpid();

			    char msg[200];
			    const int32_t length = snprintf(msg, sizeof(msg),
                "%d: I'm a parent, my child has PID %d\nUsing: 'num num ... num <endline>'\nTo exit press Enter on empty line\n",
                pid, child);
			    write(STDOUT_FILENO, msg, length);
		    }

		    close(pipe_client_to_child[0]);
		    close(pipe_child_to_client[1]);

		    char buf[4096];
		    ssize_t bytes;

		    while ((bytes = read(STDIN_FILENO, buf, sizeof(buf)))) {
			    if (bytes < 0) {
				    const char msg[] = "error: failed to read from stdin\n";
				    write(STDERR_FILENO, msg, sizeof(msg));
				    exit(EXIT_FAILURE);
			    } else if (buf[0] == '\n')
                {
				    break;
			    }

			write(pipe_client_to_child[1], buf, bytes);

			bytes = read(pipe_child_to_client[0], buf, sizeof(buf));
			write(STDOUT_FILENO, buf, bytes);
		}

		    close(pipe_client_to_child[1]);
		    close(pipe_child_to_client[0]);

		    wait(NULL);
        }
        break;

    }
}