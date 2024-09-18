#include "microshell.h"

#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

void err(char *str)
{
	while (*str)
		write(2, str++, 1);
	return (1);
}

int cd(char **argv, int i)
{
	if (i != 2)
		return err("error: cd: bad arguments\n");
	if (chdir(argv[1]) == -1)
		return err("error: cd: cannot change directory to "), err(argv[1]), err("\n");
	return 0;
}

void set_pipe(int has_pipe, int *fd, int end)
{
	if (has_pipe && (dup2(fd[end], end) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		err("error: fatal\n");
}

int	exec(char **argv, int i, char **envp)
{
	int has_pipe, fd[2], pid, status;
	has_pipe = argv[i] && !strcmp(argv[i], "|");

	// if (!has_pipe && !strcmp(*argv, "cd"))
	// 	return cd(argv, i);
	if (has_pipe && pipe(fd) == -1)
		return err("error: fatal\n");
	if ((pid = fork()) == -1)
		return err("error: fatal\n");
	if (!pid)
	{
		argv[i] = 0;
		set_pipe(has_pipe, fd, 1);
		execve(*argv, argv, envp);
		return err("error: cannot execute "), err(*argv), err("\n");
	}
	waitpid(pid, &status, 0);
	set_pipe(has_pipe, fd, 0);
	return WIFEXITED(status) && WEXITSTATUS(status);
}

int main(int argc, char **argv, char **envp)
{
	int i = 0, status = 0;
	if (argc > 1)
	{
		while (argv[i] && argv[++i]) 
		{
			argv += i;
			i = 0;
			while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
				i++;
			if (!strcmp(*argv, "cd"))
				status = cd(argv, i);
			else if (i)
				status = exec(argv, i, envp);
		}
		return status;

	}
}

