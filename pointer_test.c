#include<stdio.h>

int main() {
    char *p1 = NULL;
    int *p2 = NULL;
    int i = 1;
    p1 = (char*)&i;
    p2 = &i;
    printf("p1 %p\n",p1);
    printf("p2 %p\n",p2);
    return 0;
}
