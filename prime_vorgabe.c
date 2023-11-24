#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool isPrime(unsigned int number)
{
    if(number < 2)
        return false;

    for(int i = 2; i * i <= number; i++)
    {
        if((number % i) == 0)
        {
            return false;
        }
    }
    return true;
}

int main(int argc, char const *argv[])
{
    return EXIT_SUCCESS;
}
