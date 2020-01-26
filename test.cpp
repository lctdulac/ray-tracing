#include <iostream>
#include "Vector.h"

using namespace std;

int main()
{
    Vector dir(2, 2, 3);
    Vector B(2, 2, 4);

    Vector C = dir + B;
}
