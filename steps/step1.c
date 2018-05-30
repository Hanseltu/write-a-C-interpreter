#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<memory.h>
#include<fcntl.h>
#include<unistd.h>

int token; //current token
char *src, *old_src; //pointer to source code string
int poolsize; //default size of text/data/stack
int line; //line number

int *text; //code segment
int *old_text; //for dump the code segment
int *stack;
char *data; //data segment
int *pc,*bp, *sp, ax, cycle; //virtual machine registers

//x86 instruction sets
enum{LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, PUSH,
     OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
     OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT};

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
    int op, *tmp;
    while(1){
        op = *pc++;//get next operation
        //IMM
        if (op == IMM){
            ax = *pc++;
        }
        //LC
        else if(op == LC){
            ax = * (char*)ax;
        }
        //LI
        else if(op == LI){
            ax = * (int *)ax;
        }
        //SC
        else if(op == SC){
            ax = *(char *)*sp++ = ax;
        }
        //PUSH
        else if(op == PUSH){
            *--sp = ax;
        }
        //JMP
        else if(op == JMP){
            pc = (int *)*pc;
        }
        //JZ
        else if(op == JZ){
            pc = ax ? pc + 1 : (int *)*pc;
        }
        //JNZ
        else if(op == JNZ){
            pc = ax ? (int *)*pc : pc + 1;
        }
        //CALL
        else if(op == CALL){
            *--sp = (int)(pc + 1);
            pc = (int *)*pc;
        }
        //ENT
        else if(op == ENT){
            *--sp = (int)bp;
            bp = sp;
            sp = sp - *pc++;
        }
        //ADJ
        else if (op == ADJ){
            sp = sp + *pc++;
        }
        //LEV
        else if(op == LEV){
            sp = bp;
            bp = (int *)(*sp++);
            pc = (int *)*sp++;
        }
        //LEA
        else if(op == LEA){
            ax = (int)(bp + *pc++);
        }

        //OR
        else if (op == OR)
            ax = *sp++ | ax;
        //XOR
        else if(op == XOR)
            ax = *sp++ ^ ax;
        //AND
        else if(op == AND)
            ax = *sp++ & ax;
        //EQ
        else if(op == EQ)
            ax = *sp++ == ax;
        //NE
        else if(op == NE)
            ax = *sp++ != ax;
        //LT
        else if(op == LT)
            ax = *sp++ < ax;
        //GT
        else if(op == GT)
            ax = *sp++ > ax;
        //SHL
        else if(op == SHL)
            ax = *sp++ << ax;
        //SHR
        else if(op == SHR)
            ax = *sp++ >> ax;
        //ADD
        else if(op == ADD)
            ax = *sp++ + ax;
        //SUB
        else if(op == SUB)
            ax = *sp++ - ax;
        //MUL
        else if(op == MUL)
            ax = *sp++ * ax;
        //DIV
        else if(op == DIV)
            ax = *sp++ / ax;
        //MOD
        else if(op == MOD)
            ax = *sp++ %  ax;


        //EXIT
        else if(op == EXIT){
            printf("exit(%d)", *sp);
            return *sp;
        }
        //OPEN
        else if(op == OPEN){
            ax = open((char*)sp[1], sp[0]);
        }
        //CLOS
        else if(op == CLOS){
            ax = close(*sp);
        }
        //READ
        else if(op == READ){
            ax = read(sp[2],(char *)sp[1], sp[0]);
        }
        //PRTF
        else if(op == PRTF){
            tmp = sp + pc[1];
            ax = printf((char*)tmp[-1],tmp[-2],tmp[-3],tmp[-4],tmp[-5],tmp[-6]);
        }
        //MALC
        else if(op == MALC){
            ax = (int)malloc(*sp);
        }
        //MSET
        else if(op == MSET){
            ax = (int)memset((char*)sp[2], sp[1], *sp);
        }
        //MCMP
        else if(op == MCMP){
            ax = memcmp((char*)sp[2], (char*)sp[1], *sp);
        }

        else{
            printf("unknow instruction:%d\n", op);
        }
    }
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
        printf("could not malloc(%d) for source area. \n", poolsize);
        return -1;
    }

    //read the source file
    if ((i = read(fd, src, poolsize-1)) <= 0){
        printf("read() returned %d.\n",i);
        return -1;
    }

    src[i] = 0; //add EOF character
    close(fd);

    //allocate memory for virtual machine
    if(!(text = old_text = malloc(poolsize))){
        printf("could not malloc(%d) for code segment area.\n",poolsize);
        return -1;
    }

    if(!(data = malloc(poolsize))){
        printf("could not malloc(%d) for data segment area.\n", poolsize);
        return -1;
    }

    if(!(stack = malloc(poolsize))){
        printf("could not malloc(%d) for stack area.]\n", poolsize);
        return -1;
    }

    memset(text,0,poolsize);
    memset(data,0,poolsize);
    memset(stack,0,poolsize);
    bp = sp = (int *)((int)stack + poolsize);
    ax = 0;

    //ut: caculate 10+20
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;

    pc = text;
    program();
    return eval();

}
