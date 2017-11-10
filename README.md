#Project Name: pshell (pronounced P-shell)

#Project Goal: implement a simple shell for unix based systems in C90

##Description of pshell:

I created pshell using the C programming language (specifically the C90 spec) using the C standard library and unix system calls like fork(), execvpe(), and pipe(). It provides the most basic functionality of a shell, allowing users to run programs sequentially or asynchronously and to pipe programs together. The parsing and grammar system for the shell is very simplistic and would be bettered by changing over to parsing using an Abstract Syntax Tree but that was beyond the scope of this project.

##Description of pshell grammar:

The shell defines three main types of sequences:
 - synchronous sequences are sequences of asynchronous sequences that must be run one after another and they are separated by a semicolon (;)
 - asynchronous sequences are sequences of pipelines that may be run in any order and they are separated by an ampersand (&)
 - pipelines are sequences of commands that must be pipe()ed into each other pipe->stdin and stdout->pipe and they are separated by a pipe (|)

For example:

Take a look at the following command:
`echo a & echo b ; ls -l | grep five`

The first asynchronous sequence is `echo a & echo b`; it will execute `echo a` and `echo b` in any order and then it will return the PID of `echo b` and the next asynchronous sequence will wait on the PID of `echo b` before executing in order to make the operation sequential

The second asynchronous sequence is `ls -l | grep five` and it will execute after `echo b` because of the semicolon separating the two sections. However, `ls -l | grep five` is a pipeline so it will spawn both `ls -l` and `grep five` but make sure they are piped together so the stdout of `ls -l` is sent to the `stdin` of `grep five`

##Internal operation:

The shell is composed of three main sections:
 - pshell.c is where the main() function of the program is located and is the part of the program that implements the read line, parse, and execute loop that forms the base of the shell; it also handles running synchronous sequences of commands one after another using wait()
 - process-helper.c is where the program handles running asynchronous sequences of commands and actually building and running pipelines of commands; running asynchronous sequences is fairly simple in that it simply loops over the pipelines to run and executes them without any sort of wait()s; however, building and running pipelines is much more complex - the gist of it is that a loop is used to create n - 1 pipe()s where n is the number of commands being strung together in the pipeline and then the shell fork()s out n child and then the children and shell close the ends of the pipes they will not use.
 - tokenizer.c and splitter.c is where the program handles parsing the input lines to determine what the shell user wants the shell to do (it handles the grammar)

##TODO:
 - add builtin commands to pshell like `cd` and `exit` so it is more useable as an actual shell
 - add a variable system to pshell and variable substitution so it can be used to create scripts
