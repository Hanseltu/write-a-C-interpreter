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
    printf("i = %d  j = %d   k = %d \n", i, j ,k);
    t = (i-j) ? j : k;
    printf("t=2 \n");
    printf("expr ? a : b t = %d \n",t);

    t = i && j; //bug
    printf("t=1\n");
    printf("i && j = %d \n", t);

    t = i || j;
    printf("t=1\n");
    printf("i || j = %d \n", t);

    t = !1;
    printf("t=0\n");
    printf("i ! j = %d \n", t);

    t = i ^ j;
    printf("t=3\n");
    printf("i ^ j = %d \n", t);

    t = i & j;
    printf("t=0\n");
    printf("i & j = %d \n", t);

    t = i | j;
    printf("t=3\n");
    printf("i | j = %d \n", t);

    t = i << 1;
    printf("t=2\n");
    printf("i << 1 = %d \n", t);

    t = i >> 1;
    printf("t=0\n");
    printf("i >> 1 = %d \n", t);

    t = (i + j) * (j + k) /5 -i; // (1 + 2) * (2 + 3)/5 - 1 = 2
    printf("t=2\n");
    printf("+-*/ t = %d \n", t);
    return 0;
}

int add (int a, int b) {
    printf("sum is %d\n", a + b);
    return 0;
}

int ident(char *a, char b) {
    printf("char *a = %c, char b = %c", *a, b);
    return 0;
}

int sizeOf(int *a){
   // printf("sizeof(int) = %d\n sizeof(char) = %d\n sizeof(*int) = %d\n", (int)sizeof(int), (int)sizeof(char), (int)sizeof(a));
  // printf("%d \n",sizeof(a));
    return 0;
}
int main() {
    int a[1];
    *a = 'c';
    test();
    add(1,3);
    //sizeOf(a);
    //ident(a,'b');
    printf("%d\n",*a);
    return 0;
}
