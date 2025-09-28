#include "swap.h"

void Swap(char *left, char *right)
{
	char temp = *left;   // Сохраняем значение left во временной переменной
	*left = *right;      // Копируем значение right в left
	*right = temp;	
}

