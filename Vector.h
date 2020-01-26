#pragma once

#include <iostream>
#include <cmath>
using namespace std;

class Vector
{
public:
    double x, y, z;

    // constructeur
    Vector(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z){};

    // affichage
    void print()
    {
        cout << "x: " << x << "\ty: " << y << "\tz: " << z << endl;
    }

    // opérateurs simples
    Vector operator+(const Vector &a)
    {
        return Vector(x + a.x, y + a.y, z + a.z);
    };

    Vector operator-(const Vector &a)
    {
        return Vector(x - a.x, y - a.y, z - a.z);
    };

    Vector operator*(double a)
    {
        return Vector(a * x, a * y, a * z);
    };

    Vector operator/(double a)
    {
        return Vector(x / a, y / a, z / a);
    };

    // opérateurs produits
    double dot(const Vector &a)
    {
        return a.x * x + a.y * y + a.z * z;
    };

    double getNorm()
    {
        return sqrt(x * x + y * y + z * z);
    }

    double getNorm2()
    {
        return (x * x + y * y + z * z);
    }

    void normalize()
    {
        double norm = getNorm();
        x /= norm;
        y /= norm;
        z /= norm;
    }

    Vector getNormalized()
    {
        Vector result(*this);
        result.normalize();
        return result;
    }
};