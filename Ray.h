#pragma once

#include <iostream>
#include <cmath>
#include "Vector.h"
using namespace std;

class Ray
{
public:
    Vector origin, dir;

    // constructeur
    Ray(Vector O, Vector u) : origin(O), dir(u){};

    //
};