#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include "Vector.h"
#include "Sphere.h"
#include "Ray.h"
using namespace std;

class Scene
{
public:
    std::vector<Sphere> spheres;
    Vector position_lumiere;
    double intensite_lumiere;

    // constructeur
    Scene(){};

    // methods

    void addSphere(const Sphere &s)
    {
        spheres.push_back(s);
    }

    // methods

    bool intersect(Ray &ray, Vector &P, Vector &N, int &sphere_id, double &min_t)
    {

        bool has_inter = false;
        min_t = 1E99;
        // pour toutes les spheres de la scene

        for (int i = 0; i < spheres.size(); i++)
        {
            Vector localP, localN; // les plus proches
            double t;
            bool local_has_inter = spheres[i].intersect(ray, localP, localN, t);
            if (local_has_inter)
            {
                has_inter = true;
                if (t < min_t)
                {
                    min_t = t;
                    P = localP;
                    N = localN;
                    sphere_id = i;
                }
            }
        };

        return has_inter;
    };
};