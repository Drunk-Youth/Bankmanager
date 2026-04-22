#include <stdio.h>
#include <stdlib.h>
#include "bank.h"

void ClearInput()
{
    while(getchar() != '\n');
}

int GetInputInt(const char *prompt) 
{
    int value;
    printf("%s", prompt);
    while(scanf("%d", &value) != 1) {
        ClearInput();
        printf("输入错误，请重新输入: ");
    }
    return value;
}

double GetInputDouble(const char *prompt) 
{
    double value;
    printf("%s", prompt);
    while(scanf("%lf", &value) != 1) {
        ClearInput();
        printf("输入错误，请重新输入: ");
    }
    return value;
}
