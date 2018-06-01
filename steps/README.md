## Main instructions for each step

### Contents

* step0 Framework to c interpreter

* step1 Virtual machine design

* step2 Lexcial analysis

* step3 Grammar analysis


### step0 : Framework to c interpreter

For function in the framework

next() lexcial analysis part, get next token, remove the withespace automatically.

program() grammer analysis part, process the whole c codes.

expression(int level) prase part, deal with the expressions.

eval() entrace of virtual machine, translate the AST to object assmebly code。



### step1 : Virtual machine design

Three parts should be considered.

1.Memory

* code segment

* data segment

* stack

2.Register

* PC

* SP

* BP

* AX

3.x86 Instruction sets



### step2 : Lexcial analysis

1.method

* Input: source code

* Output: token stream -->> (token, token value)

    Point:analyze different identifiers and stroe them in symbol table

2.Identifiers

Symbol table:

`token hash name type class value btype bclass bvalue`

3.Lex framework

```c
void next(){
    char *last_pos;
    int hash;

    while(token = *src){
            ++src;
            //parse token
        }
        return;
    }
```

### step3 : Grammar analysis

1.Iteration decline

2.Variable declaration

3.Function declaration

4.Statment processing

5.Experssion processing







