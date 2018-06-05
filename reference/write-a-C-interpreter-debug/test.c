#include<stdio.h>

int main(){
    int i;
    int j;
    int *p;
    p = &i;
    i = 1;
    j = 2;
    printf("i = %d , j = %d \n", i, j);
    printf("i && j = %d \n", i&&j);
    printf("pointer is %p\n",p);
    return 0;
}
