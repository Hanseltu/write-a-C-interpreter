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
enum{Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};

//types of varible/function
enum{CHAR, INT, PTR};

int basetype; //the type of a declaration, make it global for convenience
int expr_type; //the type of an expression

int index_of_bp; //index of bp pointer on stack

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
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')){
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
        else if (token == '+'){
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
        else if (token == '>'){
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

void match(int tk) {
    if (token == tk) {
        next();
    }
    else {
        printf("%d: expected token: %d\n", line, tk);
        exit(-1);
    }
}

void expression(int level){
    //two parts: unit and operator

    //unit_unary()
    int *id;
    int tmp;
    int *addr;
    {
        if (!token) {
            printf("%d: unexpected token EOF of expression\n", line);
            exit(-1);
        }
        if (token == Num) {
            match(Num);
            //emit code
            *++text = IMM;
            *++text = token_val;
            expr_type = INT;
        }
        else if (token == '"') {
            //continous string "abc" "abc"

            //emit code
            *++text = IMM;
            *++text = token_val;

            match('"');

            //store the rest strings
            while (token == '"') {
                match('"');
            }
            //append the end of string character '\0', all the data are default to 0
            //just move data one position forward.
            data = (char*)(((int)data + sizeof(int)) & (-sizeof(int)));
            expr_type = PTR;
        }
        else if (token == Sizeof) {
            //'sizeof(int)' 'sizeof(char)' 'sizeof(*ptr)' are supported.
            match(Sizeof);
            match('(');
            expr_type = INT;

            if (token == Int) {
                match(Int);
            }
            else if (token == Char) {
                match(Char);
                expr_type = CHAR;
            }
            while (token == Mul) {
                match(Mul);
                expr_type = expr_type + PTR;
            }
            match(')');

            //emit code
            *++text = IMM;
            *++text = (expr_type == CHAR) ? sizeof(char) : sizeof(int);
            expr_type = INT;
        }
        else if (token == Id) {
            //1.function call
            //2.enum variable
            //3.global/local variable
            match(Id);
            id = current_id;
            if (token == '(') {
                //function call
                match('(');

                //arguments
                tmp = 0;
                while (token != ')') {
                    expression(Assign);
                    *++text = PUSH;
                    tmp ++;

                    if (token == ',') {
                        match(',');
                    }
                }
                match(')');

                //emit code
                if (id[Class] == Sys) {
                    //system function
                    *++text = id[Value];
                    }
                else if (id[Class] == Fun) {
                    //function call
                    *++text = CALL;
                    *++text = id[Value];
                }
                else {
                    printf("%d: bad function call\n",line);
                    exit(-1);
                }

                //clean the stack for arguments
                if (tmp > 0) {
                    *++text = ADJ;
                    *++text = tmp;
                }
                expr_type = id[Type];
            }
            else if (id[Class] == Num) {
            //enum variable
            *++text = IMM;
            *++text = id[Value];
            expr_type = INT;
            }
            else {
                //variable
                if (id[Class] == Loc) {
                    *++text = LEA;
                    *++text = index_of_bp - id[Value];
                }
                else if (id[Class] == Glo) {
                    *++text = IMM;
                    *++text = id[Value];
                }
                else {
                    printf("%d:undefined variable\n",line);
                    exit(-1);
                }

                //emit code
                //load the value of the address which is stored in 'ax'
                expr_type = id[Type];
                *++text = (expr_type == Char) ? LC : LI;
            }
        }
        else if (token == '(') {
            //cast or parenthesis
            match('(');
            if (token == Int || token == Char) {
                tmp = (token == Char) ? CHAR : INT;
                match(token);
                while (token == Mul) {
                    match(Mul);
                    tmp = tmp + PTR;
                }

                match(')');
                expression(Inc);
                expr_type = tmp;
            }
            else {
                //normal parenthesis
                expression(Assign);
                match(')');
            }
        }
        else if (token == Mul) {
            //dereference *<addr>
            match(Mul);
            expression(Inc);

            if (expr_type >= PTR) {
                expr_type = expr_type - PTR;
            }
            else {
                printf("%d: bad address of \n", line);
                exit(-1);
            }
            expr_type = expr_type + PTR;
        }
        else if (token == '!') {
            //not
            match('!');
            expression(Inc);

            //emit code,use <expr> == 0
            *++text = PUSH;
            *++text = IMM;
            *++text = 0;
            *++text = EQ;

            expr_type = INT;
        }
        else if (token == '~') {
            //bitwise not
            match('~');
            *++text = PUSH;
            *++text = IMM;
            *++text = -1;
            *++text = XOR;

            expr_type = INT;
        }
        else if (token == Add) {
            //+var
            match(Add);
            expression(Inc);
            expr_type = INT;
        }
        else if (token == Sub) {
            //-var
            match(Sub);
            if (token == Num) {
                *++text = IMM;
                *++text = -token_val;
                match(Num);
            }
            else {
                *++text = IMM;
                *++text = -1;
                *++text = PUSH;
                expression(Inc);
                *++text = MUL;
            }
            expr_type = INT;
        }
        else if (token == Inc || token == Dec) {
            tmp = token;
            match(token);
            expression(Inc);
            if (*text == LC) {
                *text = PUSH; //to duplicate the address
                *++text = LC;
            }
            else if (*text == LI) {
                *text = PUSH;
                *++text = LI;
            }
            else {
                printf("%d: bad lvalue of pre-increment \n", line);
                exit(-1);
            }
            *++text = PUSH;
            *++text = IMM;
            *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
            *++text = (expr_type == CHAR) ? SC : SI;
        }
        else {
            printf("%d:bad expression\n",line);
            exit(-1);
        }
    }

    //binary operation and postfix operators.
    {
        while (token >= level) {
            //handle according to current operator's precedence
            tmp = expr_type;
            if (token == Assign) {
                //var
                match(Assign);
                if (*text == LC || *text == LI) {
                    *text = PUSH;  //save the lvalue's pointer
                }
                else {
                    printf("%d: bad lvalue in assignment\n", line);
                    exit(-1);
                }
                expression(Assign);
                expr_type = tmp;
                *++text = (expr_type == CHAR) ? SC : SI;
            }
            else if (token == Cond) {
                //expr ? a : b;
                match(Cond);
                *++text = JZ;
                addr = ++text;
                expression(Assign);
                if (token == ':') {
                    match(':');
                }
                else {
                    printf("%d:missing colon in conditional\n", line);
                    exit(-1);
                }
                *addr = (int)(text + 3);
                *++text = JMP;
                addr = ++text;
                expression(Cond);
                *addr = (int)(text + 1);
            }
            else if (token == Lor) {
                //logic or
                match(Lor);
                *++text = JNZ;
                addr = ++text;
                expression(Lan);
                *addr = (int)(text + 1);
                expr_type = INT;
            }
            else if (token == Lan) {
                //logic and
                match(Lan);
                *++text = JZ;
                addr = ++text;
                expression(Or);
                *addr = (int)(text  + 1);
                expr_type = INT;
            }
            else if (token == Or) {
                //bitwise or
                match(Or);
                *++text = PUSH;
                expression(Xor);
                *++text = OR;
                expr_type = INT;
            }
            else if (token == And) {
                //bitwise and
                match(And);
                *++text = PUSH;
                expression(Eq);
                *++text = AND;
                expr_type = INT;
            }
            else if (token == Eq) {
                //equall ==
                match(Eq);
                *++text = PUSH;
                expression(Ne);
                *++text = EQ;
                expr_type = INT;
            }
            else if (token == Ne) {
                //not equal !=
                match(Ne);
                *++text = PUSH;
                expression(Lt);
                *++text = NE;
                expr_type = INT;
            }
            else if (token == Lt) {
                //less than
                match(Lt);
                *++text = PUSH;
                expression(Shl);
                *++text = LT;
                expr_type = INT;
            }
            else if (token == Gt) {
                //greater than
                match(Gt);
                *++text = PUSH;
                expression(Shl);
                *++text = GT;
                expr_type = INT;
            }
            else if (token == Shl) {
                //shift left
                match(Shl);
                *++text = PUSH;
                expression(Add);
                *++text = SHL;
                expr_type = INT;
            }
            else if (token == Shr) {
                //shift right
                match(Shr);
                *++text = PUSH;
                expression(Add);
                *++text = SHR;
                expr_type = INT;
            }
            else if (token == Add) {
                //add
                match(Add);
                *++text = PUSH;
                expression(Mul);

                expr_type = tmp;
                if (expr_type > PTR) {
                    //pointer type, and not 'char*'
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                }
                *++text = ADD;
            }
            else if (token == Sub) {
                //sub
                match(Sub);
                *++text = PUSH;
                expression(Mul);
                if (tmp > PTR && tmp == expr_type) {
                    //pointer substraction
                    *++text = SUB;
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    expr_type = INT;
                }
                else if (tmp > PTR) {
                    //pointer movement
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                    *++text = SUB;
                    expr_type = tmp;
                }
                else {
                    //numeral substraction
                    *++text = SUB;
                    expr_type = tmp;
                }
            }
            else if (token == Mul) {
                //multiply
                match(Mul);
                *++text = PUSH;
                expression(Inc);
                *++text = MUL;
                expr_type = tmp;
            }
            else if (token == Div) {
                match(Div);
                *++text = PUSH;
                expression(Inc);
                *++text = MOD;
                expr_type = tmp;
            }
            else if (token == Inc || token == Dec) {
                //postifx ++ and --
                if (*text == LI) {
                    *text = PUSH;
                    *++text = LI;
                }
                else if (*text == LC) {
                    *text = PUSH;
                    *++text = LC;
                }
                else {
                    printf("%d:bad value in increment\n",line);
                    exit(-1);
                }
                *++text = PUSH;
                *++text = IMM;
                *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
                *++text = (token == Inc) ? ADD : SUB;
                *++text = PUSH;
                *++text = IMM;
                *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
                *++text = (token == Inc) ? SUB : ADD;
                match(token);
            }
            else if (token == Brak) {
                //arry access
                match(Brak);
                *++text = PUSH;
                expression(Assign);
                match(']');

                if (tmp > PTR) {
                    //pointer, not char*
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                }
                else if (tmp < PTR) {
                    printf("%d:pointer type expected\n",line);
                    exit(-1);
                }
                expr_type = tmp - PTR;
                *++text = ADD;
                *++text = (expr_type == CHAR) ? LC : LI;
            }
            else {
                printf("%d:compier error, token = %d\n", line,token);
                exit(-1);
            }
        }
    }
}

void enum_declaration(){
    //parse enum
    int i;
    i=0;
    while (token != '}') {
        if (token != Id) {
            printf("%d:bad enum identifier %d\n",line, token);
            exit(-1);
        }
        next();
        if (token == Assign) {
            //{a=10}
            next();
            if (token != Num) {
                printf("%D: bad enum initializer\n",line);
                exit(-1);
            }
            i = token_val;
            next();
        }
        current_id[Class] = Num;
        current_id[Type] = INT;
        current_id[Value] = i++;

        if (token == ',') {
            next();
        }
    }
}

void function_parameter() {
    int type;
    int params;
    params = 0;
    while (token != ')') {
        //int type
        type = INT;
        if (token == INT) {
            match(Int);
        }
        else if (token == Char) {
            type = CHAR;
            match(Char);
        }

        //pointer type
        while (token == Mul){
            match(Mul);
            type = type + PTR;
        }

        //parameter name
        if (token != Id) {
            printf("%d: bad parameter declaration\n", line);
            exit(-1);
        }
        if (current_id[Class] == Loc) {
            printf("%d: duplicate parameter declaration\n",line);
            exit(-1);
        }

        match(Id);
        //store the local variable
        current_id[BClass] = current_id[Class];
        current_id[Class] = Loc;
        current_id[BType] = current_id[Type];
        current_id[Type] = type;
        current_id[BValue] = current_id[Value];
        current_id[Value] = params++;

        if (token == ','){
            match(',');
        }
    }
    index_of_bp = params + 1;
}

void function_body() {
    //1. local declaration
    //2. statments
    int pos_local;
    int type;
    pos_local = index_of_bp;

    while (token == Int || token == Char){
        //local variable declaration,like global
        basetype = (token == Int) ? INT : CHAR;
        match(token);

        while (token != ';') {
            type = basetype;
            while (token == Mul) {
                match(Mul);
                type = type + PTR;
            }

            if (token != Id) {
                printf("%d: bad local declaration\n",line);
                exit(-1);
            }
            if (current_id[Class] == Loc) {
                //identifier exists
                printf("%d: duplicate local declaration\n",line);
                exit(-1);
            }
            match(Id);

            //store the local variable
            current_id[BClass] = current_id[Class];
            current_id[Class] = Loc;
            current_id[BType] = current_id[Type];
            current_id[Type] = type;
            current_id[BValue] = current_id[Value];
            current_id[Value] = ++pos_local;

            if (token == ','){
                match(',');
            }
        }
        match(';');
    }

    //save the stack size for local variable
    *++text = ENT;
    *++text = pos_local - index_of_bp;

    //statements
    while (token != '}') {
        statement();
    }
    //emit code for leaving the sub function
    *++text = LEV;
}

void function_declaration() {
    //type func_name (...) {...}
    //              | this part
     match('(');
     function_parameter();
}
void global_declaration() {
    //global_declaration ::= enum_decl | varible_decl | function_decl
    int type; //tmp, actual type for variable
    //int i;

    basetype = INT;

    //parse enum
    if (token == Enum) {
        //enum[id]{a = 10, b = 20, ...}
        match(Enum);
        if (token != '{') {
            match(Id); //skip the [id] part
        }
        if (token == '{') {
            //parse the assign part
            match('{');
            enum_declaration();
            match('}');
        }
        match(';');
        return;
    }

    //parse type information
    if (token == Int) {
        match(Int);
    }
    else if (token == Char) {
        match(Char);
        basetype = CHAR;
    }

    //parse the comma seperated variable declaration
    while (token != ';' && token != '}') {
        type = basetype;
        //parse pointer type ,how to deal with `int ****x`?
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }
        if (token != Id) {
            printf("%d: bad global declaration\n",line);
            exit(-1);
        }
        if (current_id[Class]) {
            printf("%d: duplicate global declaration\n",line);
            exit(-1);
        }
        match(Id);
        current_id[Type] = type;

        if (token == '(') {
            current_id[Class] = Fun;
            current_id[Value] = (int)(text + 1);
            function_declaration();
        }
        else {
            //variable decl
            current_id[Class] = Glo;
            current_id[Value] = (int)data;
            data = data + sizeof(int);
        }

        if (token == ',') {
            match(',');
        }
    }
    next();
}

void program(){
    next(); //get next token
    while (token > 0){
        global_declaration();
    }
}

void statement(){
    //
    //6 statments
    //1. if else
    //2. while
    //3. {  }
    //4. return
    //5. empty statment
    //6. expression end with
    //
    int *a, *b;
    if (token == If) {
        match(If);
        match('(');
        expression(Assign);
        match(')');

        //emit code for if
        *++text = JZ;
        b = ++text;

        statement();

        if (token == Else) {
            match(Else);

            //emit code for JMP b
            *b = (int)(text + 3);
            *++text = JMP;
            b = ++text;

            statement();
        }
        *b = (int)(text + 1);
    }
    else if (token == While) {
        match(While);
        a = text + 1;
        match('(');
        expression(Assign);
        match(')');

        *++text = JZ;
        expression(Assign);
        match(')');

        *++text = JZ;
        b = ++text;

        statement();

        *++text = JMP;
        *++text = (int)a;
        *b = (int)(text + 1);
    }
    else if (token == '{') {
        match('{');
        while (token != '}') {
            statment();
        }

        match('}');
    }
    else if (token == Return) {
        match(Return);
        if (token != ';') {
            expression(Assign);
        }
        match(';');
        *++text = LEV;
    }
    else if (token == ';') {
        match(';');
    }
    else {
        expression(Assign);
        match(';');
    }
}


int eval(){
    int op, *tmp;
    while (1) {
        op = *pc++;//get next operation
        //IMM
        if (op == IMM){
            ax = *pc++;
        }
        //LC
        else if (op == LC){
            ax = * (char*)ax;
        }
        //LI
        else if (op == LI){
            ax = * (int *)ax;
        }
        //SC
        else if (op == SC){
            ax = *(char *)*sp++ = ax;
        }
        //PUSH
        else if (op == PUSH){
            *--sp = ax;
        }
        //JMP
        else if (op == JMP){
            pc = (int *)*pc;
        }
        //JZ
        else if (op == JZ){
            pc = ax ? pc + 1 : (int *)*pc;
        }
        //JNZ
        else if (op == JNZ){
            pc = ax ? (int *)*pc : pc + 1;
        }
        //CALL
        else if (op == CALL){
            *--sp = (int)(pc + 1);
            pc = (int *)*pc;
        }
        //ENT
        else if (op == ENT){
            *--sp = (int)bp;
            bp = sp;
            sp = sp - *pc++;
        }
        //ADJ
        else if (op == ADJ){
            sp = sp + *pc++;
        }
        //LEV
        else if (op == LEV){
            sp = bp;
            bp = (int *)(*sp++);
            pc = (int *)*sp++;
        }
        //LEA
        else if (op == LEA){
            ax = (int)(bp + *pc++);
        }

        //OR
        else if (op == OR)
            ax = *sp++ | ax;
        //XOR
        else if (op == XOR)
            ax = *sp++ ^ ax;
        //AND
        else if (op == AND)
            ax = *sp++ & ax;
        //EQ
        else if (op == EQ)
            ax = *sp++ == ax;
        //NE
        else if (op == NE)
            ax = *sp++ != ax;
        //LT
        else if (op == LT)
            ax = *sp++ < ax;
        //GT
        else if (op == GT)
            ax = *sp++ > ax;
        //SHL
        else if (op == SHL)
            ax = *sp++ << ax;
        //SHR
        else if (op == SHR)
            ax = *sp++ >> ax;
        //ADD
        else if (op == ADD)
            ax = *sp++ + ax;
        //SUB
        else if (op == SUB)
            ax = *sp++ - ax;
        //MUL
        else if (op == MUL)
            ax = *sp++ * ax;
        //DIV
        else if (op == DIV)
            ax = *sp++ / ax;
        //MOD
        else if (op == MOD)
            ax = *sp++ %  ax;


        //EXIT
        else if (op == EXIT){
            printf("exit(%d) \n", *sp);
            return *sp;
        }
        //OPEN
        else if (op == OPEN){
            ax = open((char*)sp[1], sp[0]);
        }
        //CLOS
        else if (op == CLOS){
            ax = close(*sp);
        }
        //READ
        else if(op == READ){
            ax = read(sp[2],(char *)sp[1], sp[0]);
        }
        //PRTF
        else if (op == PRTF){
            tmp = sp + pc[1];
            ax = printf((char*)tmp[-1],tmp[-2],tmp[-3],tmp[-4],tmp[-5],tmp[-6]);
        }
        //MALC
        else if (op == MALC){
            ax = (int)malloc(*sp);
        }
        //MSET
        else if (op == MSET){
            ax = (int)memset((char*)sp[2], sp[1], *sp);
        }
        //MCMP
        else if (op == MCMP){
            ax = memcmp((char*)sp[2], (char*)sp[1], *sp);
        }

        else {
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

    if ((fd = open(*argv,0)) < 0){
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    //allocate memory for virtual machine
    if (!(text = old_text = malloc(poolsize))){
        printf("could not malloc(%d) for code segment area.\n",poolsize);
        return -1;
    }
    //allocate memory for data segment
    if (!(data = malloc(poolsize))){
        printf("could not malloc(%d) for data segment area.\n", poolsize);
        return -1;
    }
    //allocate memory for stack
    if (!(stack = malloc(poolsize))){
        printf("could not malloc(%d) for stack area.]\n", poolsize);
        return -1;
    }
    //allocate memory for symble table
    if (!(symbols = malloc(poolsize))){
        printf("could not malloc(%d) for symbol table.\n", poolsize);
        return -1;
    }

    memset(text,0,poolsize);
    memset(data,0,poolsize);
    memset(stack,0,poolsize);
    memset(symbols, 0,poolsize);
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

    src = "char else enum if int return sizeof while "
          "open read close printf malloc memset memcmp exit void main";

    //add keywords to symbol table
    i = Char;
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    //add library to symbol table
    i = OPEN;
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }
    //handle void type
    next();
    current_id[Token] = Char;
    //keep track of main
    next();
    idmain = current_id;

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
    //******ut
    //program();
    //printf("The int size is %d\n",sizeof(int));
    //printf("The long size is %d\n",sizeof(long));
    return eval();

}
