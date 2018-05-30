#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<memory.h>
#include<fcntl.h>
#include<unistd.h>

int token; //current token
char* src, *old_src; //pointer to source code string
int poolsize; //default size of text/data/stack
int line; //line number

void next(){
    token = *src++;
    return ;
}

void experssion(int level){
    //pass
}

void program(){
    next(); //get next token
    while(token > 0){
        printf("token is : %c \n", token);
        next();
    }
}

int eval(){
    //pass
    return 0;
}

int main(int argc, char** argv)
{
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 256;
    line = 1;

    if((fd = open(*argv,0)) < 0){
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if(!(src = old_src = malloc(poolsize))){
        printf("could not malloc(%d) fir source area. \n", poolsize);
        return -1;
    }

    //read the source file
    if ((i = read(fd, src, poolsize-1)) <= 0){
        printf("read() returned %d.\n",i);
        return -1;
    }

    src[i] = 0;
    close(fd);

    program();
    return eval();

}
