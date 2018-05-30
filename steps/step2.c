#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<memory.h>
#include<fcntl.h>
#include<unistd.h>

int token; //current token
int token_val; //value of current token, mainly for numbers
char *src, *old_src; //pointer to source code string
int poolsize; //default size of text/data/stack
int line; //line number

int *text; //code segment
int *old_text; //for dump the code segment
int *stack;
char *data; //data segment
int *pc,*bp, *sp, ax, cycle; //virtual machine registers

int *current_id; //currend parsed ID
int *symbols; //symbol table
int *idmain; //the main function

//x86 instruction sets
enum{LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, PUSH,
     OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
     OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT};


//tokens and classes
enum{ Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Else, Enum, If, Int, Return, Sizeof, While,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Ge,
    Le, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak};

//field of identifier
enum{Token, Hash, Name, Type, Class, Value, BType, BClass, Bvalue, IdSize};

//types of varible/function
enum{CHAR, INT, PTR};

void next(){
    char *last_pos;
    int hash;

    while(token == *src){
        ++src;

        //parse token
        if(token == '\n'){
            ++line;
        }
        else if(token == '#'){
            //not support, just skip
            while(*src != 0 && *src != '\n'){
                src ++;
            }
        }
        else if((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')){
            last_pos = src - 1;
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')){
                hash = hash*147 + *src;
                src ++;
            }
            //look for existing identifier,linear search
            current_id = symbols;
            while (current_id[Token]){
                if (current_id[Hash] == hash && !memcmp((char*)current_id[Name], last_pos, src - last_pos)){
                    //found success!
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }

            //store New ID
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
        else if (token >= '0' && token <= '9'){
            //parse numbers, support dec(123), hex(0x123), oct(017)
            token_val = token - '0';
            if (token_val > 0){
                //deal with dec from1-9
                while (*src >= '0' && *src <= '9'){
                    token_val = token_val*10 + *src++ - '0';
                }
            }
            else {
                //start with 0
                if (*src == 'x' || *src == 'X'){
                    //deal with hex
                    token = *++src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'F')){
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                        token = *src++;
                    }
                }
                else {
                    //deal with oct
                    while (*src >= '0' && *src <= '7'){
                        token_val = token * 8 + *src++ - '0';
                    }
                }
            }
            token = Num;
            return;
        }

        else if (token == '"' || token == '\''){
            //parse string literal, only supportd escape character is '\n'
            last_pos = data;
            while (*src != 0 && *src != token){
                token_val = *src++;
                if (token_val == '\\'){
                    //escape character
                    token_val = *src++;
                    if (token_val == 'n'){
                        token_val = '\n';
                    }
                }
                if (token == '"'){
                    *data++ = token_val;
                }
            }
            src ++;
            //if it is a single character, return Num token
            if (token == '"'){
                token_val = (int)(last_pos);
            }
            else {
                token = Num;
            }
            return;
        }
        else if (token == '/'){
            if (*src == '/'){
                //skip the comments
                while (*src != 0 && *src != '\n'){
                    ++src;
                }
            }
            else {
                //divide operation
                token = Div;
                return;
            }
        }
        else if (token == '='){
            //parse '==' and '='
            if (*src == '='){
                src ++;
                token = Eq;
            }
            else {
                token = Assign;
            }
            return;
        }
        else if(token == '+'){
            //parse '++' and '+'
            if (*src == '+'){
                src ++;
                token = Inc;
            }
            else {
                token = Add;
            }
            return ;
        }
        else if (token == '-'){
            //parse '--' and '-'
            if (*src == '-'){
                src ++;
                token = Dec;
            }
            else {
                token = Sub;
            }
            return ;
        }
        else if (token == '!'){
            //parse '!='
            if (*src == '='){
                src ++;
                token = Ne;
            }
            return ;
        }
        else if (token == '<'){
            //parse '<=' and '<<' and '<'
            if (*src == '='){
                src ++;
                token = Le;
            }
            else if (*src == '<'){
                src ++;
                token = Shl;
            }
            else {
                token = Lt;
            }
            return;
        }
        else if(token == '>'){
            //parse '>=' and '>>' and '>'
            if (*src == '='){
                src ++;
                token = Ge;
            }
            else if (*src == '>'){
                src ++;
                token = Shr;
            }
            else {
                token = Gt;
            }
            return ;
        }
        else if (token == '|'){
            //parse '|' and '||'
            if (*src == '|'){
                src ++;
                token = Lor;
            }
            else {
                token = Or;
            }
            return ;
        }
        else if (token == '&'){
            //parse '&' and '&&'
            if (*src == '&'){
                src ++;
                token = Lan;
            }
            else {
                token = And;
            }
            return ;
        }
        else if (token == '^'){
            token = Xor;
            return;
        }
        else if (token == '%'){
            token = Mod;
            return ;
        }
        else if (token == '*'){
            token = Mul;
            return;
        }
        else if (token == '['){
            token = Brak;
            return ;
        }
        else if (token == '?'){
            token = Cond;
            return ;
        }
        else if (token == '~' || token == ';' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':'){
            //directly return the character as token
            return ;
        }
    }
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
            printf("exit(%d) \n", *sp);
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

    //*******ut: caculate 10+20*****
    i=0;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;
    pc = text;
    //******************************
    program();
    //printf("The int size is %d\n",sizeof(int));
    //printf("The long size is %d\n",sizeof(long));
    return eval();

}
