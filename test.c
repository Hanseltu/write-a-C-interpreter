//This file use to test the function in the c interpreter.
//The function includes:
//                      1. expression
//                      2. function
//                      3. identifier
//                      4. statement
//                      5. computation
//                      6. comments
//                      7. escape \n
//
#include<stdio.h>

int test(){
    int i, j, k,t;
    i = 1;
    j = 2;
    k = 3;

    t = (i-j) ? j : k;
    printf("t=3 \n");
    printf("expr ? a : b t = %d \n",t);

    t = i && j;
    printf("t=1\n");
    printf("i && j = %d \n", t);

    t = i || j;
    printf("t=1\n");
    printf("i || j = %d \n", t);

    t = i ^ j;
    printf("t=3\n");
    printf("i ^ j = %d \n", t);

    t = i & j;
    printf("t=0\n");
    printf("i & j = %d \n", t);

    t = i | j;
    printf("t=3\n");
    printf("i | j = %d \n", t);

    t = (i + j) * (j + k) /5 -i; // (1 + 2) * (2 + 3)/5 - 1 = 2
    printf("t=2\n");
    printf("+-*/ t = %d \n", t);



    return 0;
}

int main() {
    test();
    return 0;
}
