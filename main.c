#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

char ** tokenArray(char str[], char * tok); 
int normExec(char str[]);
int pipeExec(char str[]);
void alterExec(char str[], char* sep);
int redirectExec(char str[], char* sym);
int execFlow(char str[]);
int pipeProc(int in, int out, char** args);
int getSize(char ** ar);
char * checkStr(char str[]); 
void freeMem(char** args);
// void pipeArray(char ** args);
// firstSymbol: returns int index, switch statement that calls proc
// implment the funciton by itself first
    

int main() {
    // array of symbols < > |
   // char builtin[5] = {"
    char str[100];
    char ** args;
    char * sep; //need mem aloc?
    char s[100];

    while (true) {
        fseek(stdin,0,SEEK_END);
        printf("%.4s", "psp:"); //
        printf("%s", getcwd(s,100));
        printf("$ ");
        gets(str);
        if(feof(stdin)) {
	    exit(0);
	}
        while ((sep = checkStr(str)) != NULL) {
            if (strcmp(sep, "&&") == 0) {
                alterExec(str, "&&");
            } else if (strcmp(sep, "||") == 0) {
                alterExec(str, "||");
            } else if (strcmp(sep, ";") == 0) {
                alterExec(str, ";");
            }
        }
        if (strcmp(str, "") != 0) {  
            execFlow(str);
        }
    }
}

void alterExec(char str[], char* sep) { //execs first of sequential when string 
    //contains ; or || or &&
    char * token;
    char * token2;
    int status;
    token = strtok(str, sep);
    token2 = strtok(NULL, ""); 
    status = execFlow(token); //execs first part of command; putting token through execflow messes it up, qed new pointer
    strcpy(str, token2); //removes first command of array
    if (str[0] == '&') { //tokenize keeps one &, this removes space and symbol
        memmove(str, str+2, strlen(str));
        //strcpy(str, str + 2);
    } else if (str[0] == '|') { 
        memmove(str, str+2, strlen(str));
        //strcpy(str, str + 2);
    }
//    printf("%s", str);
 //   printf("%s", "\n");
    if ((sep == "||" && status == 0) || (sep == "&&" && status != 0)) { //&& and || cond
        sep = checkStr(str);
        if (sep == NULL) { //set str to empty if only one command after this one
            strcpy(str, "");
        } else {
        //look for ||, &&, and ; in string if there remove next string 
            token = strtok(str,sep);
            token = strtok(NULL, "");
            strcpy(str, token);
        }

    }

}

int execFlow(char str[]) { //checks if command needs to be redirected, pipelined, 
    //or other altercations
    int status;
    if (strstr(str, "|")) {
        status = pipeExec(str);
    } else if (strstr(str, ">>")) {
        status = redirectExec(str, ">>");
    } else if (strstr(str, "<>")) {

    } else if (strstr(str, "<&")) {

    } else if (strstr(str, "&>")) {

    } else if (strstr(str, ">")) {
        status = redirectExec(str, ">");
    } else if (strstr(str, "<")) {
        status = redirectExec(str, "<");
    } else {
        status = normExec(str);
    } 
    return status;

}

int normExec(char str[]) {
    int fildes[2];
    int pid;
    int status;
    char** args2;
    args2 = tokenArray(str, " ");
    if (strcmp(args2[0], "cd") == 0) {
        status = 0;
        if (chdir(args2[1]) != 0) {
            printf(args2[0]);
            printf(": ");
            printf(args2[1]);
            printf(": No such file or directory\n");
            status = 1;
        }
    } else {
        pid = fork();
        if (pid == 0) { //forks and runs command
            if (execvp(args2[0], args2) < 0){
                printf(args2[0]);
                printf(": command not found\n");
            }
            exit(0);
        }

    waitpid(pid, &status, 0);
    }
    freeMem(args2);
    //printf("status\n");
    //printf("%d", status);
    return status;
}

void freeMem(char** args) {
    for (int i =0; i < 2; i++) { //fix this!!!!!!!!!!
        free(args[i]);
    }
    free(args);
}

int pipeProc(int in, int out, char** args) { //switches input and output of pipe by passing in input as out 
// and reset output/write of pipe as out
    pid_t pid;
    if ((pid = fork()) == 0) {
        if (in != 0) {
            dup2 (in, 0); //set input to be stdin
            close (in);
        }
        if (out != 1) { //set output to be stdout
            dup2 (out, 1);
            close(out);
            //close (out);
        }
        if (execvp(args[0], args) < 0) {
            fprintf(stderr, args[0]);
            fprintf(stderr, ": command not found\n");
        }
        exit(0);
    }
    //waitpid(pid, 0, 0);
    return pid;
}
char ** tokenArray(char str[], char * tok) {
    char ** args2;
    char * token;
    int i = 0;
    token = strtok(str,tok); //tokenizes getline
    char* args[10] = { NULL }; 
    /*for (int j = 0; j < 10; j++) { //initalize dummy array
        strcpy(args[j], NULL);
    */
    while (token != NULL) { //puts args in the dummy array and counts real args
        args[i] = token;
        token = strtok(NULL, tok);
        i = i + 1;
    }
    i = i + 1;
    args2 = (char **) malloc(i * sizeof(char*));
    for (int j = 0; j < i - 1; j++) { //makes dynamic array of correct size and fills it up
        args2[j] = (char *) malloc(101 * sizeof(char));
        strcpy(args2[j], args[j]); 
    }
    args2[i-1] = NULL;
//    strcpy(args2[i-1], NULL);
    return args2;
}
   
int getSize(char ** ar) {
    int size;
    for (size = 0; ar[size + 1]; size++);
    return size + 1;
}


char * checkStr(char str[]) { //goes through string and if it contains ; or 
    // && or ||, then will return the char * of first occurance of in string.
    bool semi = false;
    bool or = false;
    bool and = false;
    char * semiStr;
    char * andStr;
    char * orStr;
    int semiLen = 0, andLen = 0, orLen = 0;
    if(semiStr = strstr(str, ";")) {
        semi = true;
        semiLen = strlen(semiStr); 
    } else if (andStr = strstr(str, "&&")) {
        and = true;
        andLen = strlen(andStr);
    } else if (orStr = strstr(str, "||")) {
        or = true;
        orLen = strlen(orStr);
    } else {
        return NULL;
    }
    if (semi && !or && !and) {
        return ";";
    } else if (!semi && or && !and) {
        return "||";
    } else if (!semi && !or && and) {
        return "&&";
    } else if(semi && or && !and) {
        if (semiLen > orLen) {
            return ";"; 
        } else {
            return "||";
        }
    } else if (semi && !or && and) {
        if (semiLen > andLen) {
            return ";";
        } else {
            return "&&";
        }
    } else if (!semi && or && and) {
        if (orLen > andLen) {
            return "||"; 
        } else {
            return "&&";
        }
    } else if (semi && or && and) {
        if (semiLen > orLen) {
            if (semiLen > andLen) {
                return ";";
            } else {
                return "&&";
            }
        } else if (orLen > andLen) {
            return "||";
        } else if (andLen > orLen) {
            return "&&";
        }
    }
}


int redirectExec(char str[], char* sym) {
    int redId;
    int savedStdout = dup(STDOUT_FILENO);
    int savedStdin = dup(STDIN_FILENO);
    int size; 
    char ** args;
    char ** args2;
    args = tokenArray(str, sym);
    pid_t pid;
    int status;
    if ((pid = fork()) == 0) {
        if (sym == ">>") {
            redId = open(args[1], O_CREAT | O_APPEND | O_WRONLY);
            printf("wow its here");
            dup2(redId, 1);
            close(redId);
        } else if (sym == ">") {
            redId = open(args[1], O_CREAT | O_TRUNC | O_WRONLY);
            dup2(redId, 1);
            close(redId);
        } else if (sym == "<") {
            redId = open(args[1], O_RDONLY);
            dup2(redId, 0);
            close(redId);
        }
        args2 = tokenArray(args[0], " ");
        if (execvp(args2[0], args2) < 0) {
            fprintf(stderr, args2[0]);
            fprintf(stderr, ": command not found\n");
        }
        exit(0);
//start here
    }
    waitpid(pid, &status, 0);
    freeMem(args);
    //freeMem(args2);
    dup2(savedStdin, STDIN_FILENO); //restore stdout and stdin
    close(savedStdin);
    dup2(savedStdout, STDOUT_FILENO);
    close(savedStdout);
    return status;
}



int pipeExec(char str[]) {
    char ** args;
    char ** args2;
    int status;
    int size;
    int fildes[2];
    int in = 0;   
    int pid;
    int savedStdout = dup(STDOUT_FILENO);
    int savedStdin = dup(STDIN_FILENO);
    //new funct
    args = tokenArray(str, "|");
    size = getSize(args);
    int i;
    for (i = 0; i < size - 1; i++) {
	args2 = tokenArray(args[i], " ");
	status = pipe(fildes);
	if (status == -1 ) {
	    // an error occurred 
	    printf("pipe error");
	    break;
	}
	pipeProc(in, fildes[1], args2);
	close(fildes[1]); //closes previous pipe write
	in = fildes[0]; //makes input the read/output of previous pipe. Saves input for next loop
	freeMem(args2);
    }
    if (in != 0) { //set stdin to be read/output of pipe
	dup2(in, 0);
    }
    args2 = tokenArray(args[i], " ");
    pid = fork();
    if (pid == 0) { //forks and runs command at end of pipe
	if (execvp(args2[0], args2) < 0){
	    printf(args2[0]);
	    printf(": command not found\n");
	}
	exit(0);
    }
    waitpid(pid, &status, 0);
    freeMem(args);
    freeMem(args2);
    dup2(savedStdin, STDIN_FILENO); //restore stdout and stdin
    close(savedStdin);
    dup2(savedStdout, STDOUT_FILENO);
    close(savedStdout);
    return status;
}

