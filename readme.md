# A-Simple-Shell

**Author:** Mohammad Jahiruddin Ansari

## Description
TechShell is a simple command-line shell written in C for CSC 222. It displays the current directory, reads commands, launches external programs with `fork()` and `execvp()`, and waits for commands to finish. It implements `cd` and `exit` internally, supports input/output redirection, and recognizes quoted arguments and escaped spaces.

## Clone, compile, and run

```bash
git clone https://github.com/Ansari765/A-Simple-Shell.git
cd A-Simple-Shell.git
gcc -Wall -Wextra -std=c11 techshell.c -o techshell
./techshell
```

## Sample commands

```text
pwd
ls -l
echo "hello world"
echo hello > output.txt
cat < output.txt
wc -l < output.txt > count.txt
cat count.txt
cd ..
exit
```

## Limitations
This is a minimal educational shell, not a full Bash replacement. It does not implement pipelines, background jobs, shell variable expansion, or wildcard expansion. These are not listed among the required autograder features. Check the results of the supplied tests and Gradescope before claiming that all required features work.
