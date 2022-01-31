/*
 * UCSB CS 170
 *   (Derived from NYU CS 202.)
 *   (Derived from Eddie Kohler's UCLA shell lab.)
 * Skeleton code for Lab 2 - Shell processing
 * This file contains skeleton code for executing parsed commands.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include "cmdparse.h"
#include "cmdrun.h"

#define TBC printf("To be implemented %s: %d\n", __FILE__, __LINE__);abort();
#define WRITE 1
#define READ 0

/* cmd_exec(cmd, pass_pipefd)
 *
 *   Execute the single command specified in the 'cmd' command structure.
 *
 *   The 'pass_pipefd' argument is used for pipes.
 *
 *     On input, '*pass_pipefd' is the file descriptor that the
 *     current command should use to read the output of the previous
 *     command. That is, it's the "read end" of the previous
 *     pipe, if there was a previous pipe; if there was not, then
 *     *pass_pipefd will equal STDIN_FILENO.
 *
 *     On output, cmd_exec should set '*pass_pipefd' to the file descriptor
 *     used for reading from THIS command's pipe (so that the next command
 *     can use it). If this command didn't have a pipe -- that is,
 *     if cmd->controlop != PIPE -- then this function should set
 *     '*pass_pipefd = STDIN_FILENO'.
 *
 *   Returns the process ID of the forked child, or < 0 if some system call
 *   fails.
 *
 *   Besides handling normal commands, redirection, and pipes, you must also
 *   handle three internal commands: "cd", "exit", and "our_pwd".
 *   (Why must "cd", "exit", and "our_pwd" (a version of "pwd") be implemented
 *   by the shell, versus simply exec()ing to handle them?)
 *
 *   Note that these special commands still have a status!
 *   For example, "cd DIR" should return status 0 if we successfully change
 *   to the DIR directory, and status 1 otherwise.
 *   Thus, "cd /tmp && echo /tmp exists" should print "/tmp exists" to stdout
 *      if and only if the /tmp directory exists.
 *   Not only this, but redirections should work too!
 *   For example, "cd /tmp > foo" should create an empty file named 'foo';
 *   and "cd /tmp 2> foo" should print any error messages to 'foo'.
 *
 *   Some specifications:
 *
 *       --subshells:
 *         the exit status should be either 0 (if the last
 *         command would have returned 0) or 5 (if the last command
 *         would have returned something non-zero). This is not the
 *         behavior of bash.
 *
 *       --cd:
 *
 *          this builtin takes exactly one argument besides itself (this
 *          is also not bash's behavior). if it is given fewer
 *          ("cd") or more ("cd foo bar"), that is a syntax error.  Any
 *          error (syntax or system call) should result in a non-zero
 *          exit status. Here is the specification for output:
 *
 *                ----If there is a syntax error, your shell should
 *                display the following message verbatim:
 *                   "cd: Syntax error! Wrong number of arguments!"
 *
 *                ----If there is a system call error, your shell should
 *                invoke perror("cd")
 *
 *       --our_pwd:
 *
 *          This stands for "our pwd", which prints the working
 *          directory to stdout, and has exit status 0 if successful and
 *          non-zero otherwise. this builtin takes no arguments besides
 *          itself. Handle errors in analogy with cd. Here, the syntax
 *          error message should be:
 *
 *              "pwd: Syntax error! Wrong number of arguments!"
 *
 *       --exit:
 *
 *          As noted in the lab, exit can take 0 or 1 arguments. If it
 *          is given zero arguments (besides itself), then the shell
 *          exits with command status 0. If it is given one numerical
 *          argument, the shell exits with that numerical argument. If it
 *          is given one non-numerical argument, do something sensible.
 *          If it is given more than one argument, print an error message,
 *          and do not exit.
 *
 *
 *   Implementation hints are given in the function body.
 */
static pid_t
cmd_exec(command_t *cmd, int *pass_pipefd)
{
  (void)pass_pipefd;      // get rid of unused warning
	pid_t pid = -1;		// process ID for child
	int pipefd[2];		// file descriptors for this process's pipe

	/* EXERCISE: Complete this function!
	 * We've written some of the skeleton for you, but feel free to
	 * change it.
	 */

	// Create a pipe, if this command is the left-hand side of a pipe.
	// Return -1 if the pipe fails.
	if (cmd->controlop == CMD_PIPE) {
		if(pipe(pipefd) < 0){
			return -1;
		}
	}


	// Fork the child and execute the command in that child.
	// You will handle all redirections by manipulating file descriptors.
	//
	// This section is fairly long.  It is probably best to implement this
	// part in stages, checking it after each step.  For example:
        //
	//   --First implement just the fork and the execute in the
	//   child (as we saw in class).  This should allow you to
	//   execute simple commands like 'ls'. Some of the tests in
	//   "make grade" ought to pass.
	//   --Next, add support for redirections: as a simple test,
	//   'ls > foo' followed by 'cat < foo' should display the
	//   directory listing. A few more of the tests in "make grade"
	//   should pass.
	//   --Next, support parentheses (a few more tests should pass,
	//     etc.)
	//   --Then pipes (if you use our pseudocode from class as
	//   inspiration, NOTE that the logic is a bit different in this lab)
	//   --And finally the internal commands 'cd', 'exit', and
	//   'our_pwd'.
	//
	//  Throughout, you will be using system calls: fork(), dup2(),
	//  close(), open(), execvp() or execve(), exit(), chdir(), etc.
	//
	//  You can also use perror() to handle error conditions
	//  concisely (type "$ man 3 perror" in a Unix terminal to get
	//  documentation).
	//
	//  You will need to do real C programming here.
	//  Tools like gdb (the debugger) may be useful.
	//
	//  You will also find functions like strcmp() and strtol()
	//  useful. Type "man strcmp", etc. for more information.
	//
	// In the child, you should:
	//    1. Set up stdout to point to this command's pipe, if necessary;
	//       close some file descriptors, if necessary (which ones?)
	//    2. Set up stdin to point to the PREVIOUS command's pipe (that
	//       is, *pass_pipefd), if appropriate; close a file
	//       descriptor (which one?)
	//    3. Set up redirections. Use the open() system call.
	//       Hints:
	//          --For input redirections, the oflag should be O_RDONLY.
	//          --For output redirections (stdout and stderr), what
	//          oflags do you want?
	//          --Set the mode argument of open() to be 0666
	//    4. Execute the command.
	//       There are some special cases:
	//       a. Parentheses.  Execute cmd->subshell. How? Also,
	//          you won't be invoking exec() in this special case,
	//          so think carefully about how this path "ends". (What
	//          would happen if we were to return from the current
	//          function?)
	//       b. A null command (no subshell, no arguments).
	//          Exit with status 0.
	//       c. "exit".
	//       d. "cd".
	//       e. "our_pwd".
	//       f. Remember to make your implementation
	//       consistent with the specification above!
	//
	// In the parent, you should:
	//    1. Close some file descriptors.  Hint: Consider the write end
	//       of this command's pipe, and one other fd as well.
	//    2. Handle the special "exit", "cd", and "our_pwd" commands.
	//    3. Set *pass_pipefd as appropriate.
	//
	// "cd","exit","our_pwd" Hints:
	//    Recall from the comments earlier that you need to return a status,
	//    and do redirections. How can you do these things for a
	//    command executed in the parent shell?
	//    Answer: It's easest if you fork a child ANYWAY!
        //    You should divide functionality between the parent and the child.
        //    Some functions will be executed in each process. For example,
        //         --in the case of "exit", both parent and child
        //         need to exit, but only the parent's exit code "matters",
        //         --in the case of "cd", both parent and child should
        //         try to change directory (use chdir()), but only the child
        //         should print error messages
	//
	//    For the "cd" command, you should change directories AFTER
	//    the fork(), not before it.  Why?
	//    Design some tests with 'bash' that will tell you the answer.
	//    For example, try "cd /tmp ; cd $HOME > foo".  In which directory
	//    does foo appear, /tmp or $HOME?  If you chdir() BEFORE the fork,
	//    in which directory would foo appear, /tmp or $HOME?
	//
	//    Our "cd" solution changes the
	//    directory both in the parent process and in the child process.
	//    This introduces a potential race condition.
	//    Explain what that race condition is, and fix it.
	//    Hint: Investigate fchdir().
	pid = fork();
	if(pid < 0){
		perror("Cannot fork");
		abort();
	}
	if(pid == 0){  // Child
		// 1. If left side of pipe, redirect stdout to pipe
		if(cmd->controlop == CMD_PIPE){
			dup2(pipefd[WRITE], STDOUT_FILENO);
			close(pipefd[READ]);
			close(pipefd[WRITE]);
		}

		// 2. Setup stdin pipe
		dup2(*pass_pipefd, STDIN_FILENO);

		// 3. Set up redirect_filename
		redirect_fds(cmd);

		// 4. Execute
		if(!cmd->argv[0] && !cmd->subshell){  // null command
			exit(0);
		}
		handle_subshell(cmd);  // Subshell
		handle_builtin_commands(cmd, 1);  // Built-in commands

		// Normal case
		if(execvp(cmd->argv[0], cmd->argv)){
			perror("execvp");
			abort();
		}
	}
	else{  // Parent's duty
		// If pipe is established, close the write end of it
		if(cmd->controlop == CMD_PIPE){
			close(pipefd[WRITE]);
		}

		handle_builtin_commands(cmd, 0);

		// 3. Set *pass_pipefd
			if(cmd->controlop == CMD_PIPE){
				*pass_pipefd = pipefd[READ];
			}
			else{
				*pass_pipefd = STDIN_FILENO;
			}
	}
	// return the child process ID
	return pid;
}

static void redirect_fds(command_t* cmd){
	// STDIN
	if(cmd->redirect_filename[STDIN_FILENO]){
		int stdin_fd = open(cmd->redirect_filename[STDIN_FILENO], O_RDONLY, 0666);
		if(stdin_fd < 0){
			perror("open");
			abort();
		}
		dup2(stdin_fd, STDIN_FILENO);
	}

	// STDOUT
	if(cmd->redirect_filename[STDOUT_FILENO]){
		int stdout_fd = open(cmd->redirect_filename[STDOUT_FILENO], O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if(stdout_fd < 0){
			perror("open");
			abort();
		}
		dup2(stdout_fd, STDOUT_FILENO);
	}

	// STDERR
	if(cmd->redirect_filename[STDERR_FILENO]){
		int stderr_fd = open(cmd->redirect_filename[STDERR_FILENO], O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if(stderr_fd < 0){
			perror("open");
			abort();
		}
		dup2(stderr_fd, STDERR_FILENO);
		close(stderr_fd);
	}
}

static void handle_builtin_commands(command_t* cmd, int is_child){
	if(!cmd->argv[0]){
		return;
	}
	if(strcmp(cmd->argv[0], "exit") == 0){
		if(cmd->argv[2]){  // It should not have more than 1 argument
			if(is_child){
				write(STDERR_FILENO, "exit: too many arguments\n", 26);
				exit(1);  // Child returns 1
			}
			else{
				return;
			}
		}

		if(cmd->argv[1]){
			char *endptr, *str;
			long val;
			str = cmd->argv[1];
			errno = 0;
			val = strtol(str, &endptr, 10);

			if (errno != 0) {
				if(is_child){
					write(STDERR_FILENO, "numeric argument required\n", 27);
				}
				exit(2);  // Child returns 2
			}
			exit(val);  // Parent exit(val)
		}
		exit(0);  // // Parent exit(0)
	}
	// TODO: Potential race condition
	if(strcmp(cmd->argv[0], "cd") == 0){
		if(cmd->argv[1] && !cmd->argv[2]){  // Only takes one argument
			if(is_child){
				int status = chdir(cmd->argv[1]);
				if(status < 0){
					perror("cd");
					exit(1);  // Child returns 1 if chdir fails
				}
				exit(0);  // Child returns 0 if normal
			}
			chdir(cmd->argv[1]);
			return;
		}
		else{  // Syntax err
			if(is_child){
				write(STDERR_FILENO, "cd: Syntax error! Wrong number of arguments!\n", 45);
				exit(1);  // Child returns
			}
			return;
		}
	}
	if(strcmp(cmd->argv[0], "our_pwd") == 0){
		if(!is_child){  // Parent doesn't need to do anything
			return;
		}

		if(cmd->argv[1]){
			write(STDERR_FILENO, "pwd: Syntax error! Wrong number of arguments!\n", 46);
			exit(1);  // Child returns
		}

		char* buffer = malloc(1024);
		#ifdef __linux__
		if(syscall(__NR_getcwd, buffer, 1024) < 0){
			perror("getcwd");
			free(buffer);
			exit(1);
		}
		#else
		SYSCALL LEVEL GETCWD
		#endif

		puts(buffer);
		free(buffer);
		exit(0);
	}
}

static void handle_subshell(command_t* cmd){
	if(!cmd->subshell){  // Not a subshell
		return;
	}
	exit(cmd_line_exec(cmd->subshell));
}

/* cmd_line_exec(cmdlist)
 *
 *   Execute the command list.
 *
 *   Execute each individual command with 'cmd_exec'.
 *   String commands together depending on the 'cmdlist->controlop' operators.
 *   Returns the exit status of the entire command list, which equals the
 *   exit status of the last completed command.
 *
 *   The operators have the following behavior:
 *
 *      CMD_END, CMD_SEMICOLON
 *                        Wait for command to exit.  Proceed to next command
 *                        regardless of status.
 *      CMD_AND           Wait for command to exit.  Proceed to next command
 *                        only if this command exited with status 0.  Otherwise
 *                        exit the whole command line.
 *      CMD_OR            Wait for command to exit.  Proceed to next command
 *                        only if this command exited with status != 0.
 *                        Otherwise exit the whole command line.
 *      CMD_BACKGROUND, CMD_PIPE
 *                        Do not wait for this command to exit.  Pretend it
 *                        had status 0, for the purpose of returning a value
 *                        from cmd_line_exec.
 */
int
cmd_line_exec(command_t *cmdlist)
{
	int cmd_status = 0;	    // status of last command executed
	int pipefd = STDIN_FILENO;  // read end of last pipe

	while (cmdlist) {
		int wp_status;	    // Use for waitpid's status argument!
				    // Read the manual page for waitpid() to
				    // see how to get the command's exit
				    // status (cmd_status) from this value.

		// EXERCISE: Fill out this function!
		// If an error occurs in cmd_exec, feel free to abort().

		switch(cmdlist->controlop){
			case CMD_END:
			case CMD_SEMICOLON:{
				pid_t child = cmd_exec(cmdlist, &pipefd);
				waitpid(child, &wp_status, 0);
				cmd_status = WEXITSTATUS(wp_status);
				break;
			}
			case CMD_AND:{
				pid_t child = cmd_exec(cmdlist, &pipefd);
				waitpid(child, &wp_status, 0);
				cmd_status = WEXITSTATUS(wp_status);
				if(cmd_status != 0)
					goto done;
				break;
			}
			case CMD_OR:{
				pid_t child = cmd_exec(cmdlist, &pipefd);
				waitpid(child, &wp_status, 0);
				cmd_status = WEXITSTATUS(wp_status);
				if(cmd_status == 0)
					goto done;
				break;
			}
			case CMD_BACKGROUND:
			case CMD_PIPE:{
				pid_t child = cmd_exec(cmdlist, &pipefd);
				cmd_status = 0;
				break;
			}
			default:{
				abort();
			}
		}

		cmdlist = cmdlist->next;
	}

  while (waitpid(0, 0, WNOHANG) > 0);

done:
	return cmd_status;
}
