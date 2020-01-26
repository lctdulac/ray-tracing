#pragma once

#include <iostream>
#include <cmath>
#include "Vector.h"
using namespace std;

class Sphere
{
public:
    Vector center;
    Vector albedo;
    double r;
    bool miroir;
    bool transparent;

    // constructeur
    Sphere(Vector c, Vector couleur, double r, bool mirror = false, bool transp = false) : center(c), albedo(couleur), r(r), miroir(mirror), transparent(transp){};

    // methods

    bool intersect(Ray &ray, Vector &P, Vector &N, double &t) // ici t est en référence, donc pas besoin de le return, sa valeur sera modifiée
    {
        // résout a*t^2 + b^t + c = 0

        double a = 1;
        double b = 2 * ray.dir.dot(ray.origin - center);
        double c = (ray.origin - center).getNorm2() - r * r;

        double delta = b * b - 4 * a * c;

        if (delta < 0)
            return false;

        double t1 = (-b - sqrt(delta)) / (2 * a);
        double t2 = (-b + sqrt(delta)) / (2 * a);
        if (t2 < 0)
            return false;

        if (t1 > 0)
            t = t1;
        else
            t = t2;

        P = ray.origin + (ray.dir * t);
        N = (P - center).getNormalized();

        return true;
    }
};