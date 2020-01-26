// Raytracer.cpp : Defines the entry point for the console application.
#define _CRT_SECURE_NO_WARNINGS // for Visual Studio 2017 (maybe 2015 as well)

// Imports

#include <iostream>
#include <vector>

#include "Vector.h"
#include "Ray.h"
#include "Sphere.h"

using namespace std;

// Fonctions importées

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// Fonctions

bool intersect(Ray &ray, Sphere &sph, Vector &P, Vector &N)
{
    // résout a*t^2 + b^t + c = 0

    double a = 1;
    double b = 2 * ray.dir.dot(ray.origin - sph.center);
    double c = (ray.origin - sph.center).getNorm2() - sph.r * sph.r;

    double delta = b * b - 4 * a * c;

    if (delta < 0)
        return false;

    double t1 = (-b - sqrt(delta)) / (2 * a);
    double t2 = (-b + sqrt(delta)) / (2 * a);
    if (t2 < 0)
        return false;

    double t;

    if (t1 > 0)
        t = t1;
    else
        t = t2;

    P = ray.origin + (ray.dir * t);
    N = (P - sph.center).getNormalized();

    return true;
}

// Main

int main()
{
    // paramètres
    int W = 1024;
    int H = 1024;
    double fov = 60 * M_PI / 180;

    // image
    std::vector<unsigned char>
        img(W * H * 3);

    // objet
    Vector center(0, 0, -55);
    double r = 20;
    Sphere s(center, r);

    // lumière
    Vector position_lumiere(15, 60, -40);
    double intensite_lumiere = 1000000;

    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            Vector origin = Vector(0, 0, 0);
            Vector dir(j - W / 2, i - H / 2, -W / (2 * tan(fov / 2)));
            dir.normalize();

            Ray rayon(origin, dir);
            Vector P, N; // Vecteur point et vecteur normal

            // si on déclare ici L et d, ça ne marche pas.
            // Vector L = (position_lumiere - P).getNormalized(); // Vector unitaire towards the light
            // double d = (position_lumiere - P).getNorm();       // distance à la lumière

            bool has_inter = intersect(rayon, s, P, N);

            double intensite_pixel = 0;

            if (has_inter)
            {
                intensite_pixel = intensite_lumiere * std::max(0., (position_lumiere - P).getNormalized().dot(N)) / (position_lumiere - P).getNorm2(); // max pour pas considérer les cas ou la lumière est derrière
            }

            img[((H - i - 1) * W + j) * 3 + 0] = std::min(intensite_pixel, 255.); // has_inter ? 255 : 0; // coordo rouge
            img[((H - i - 1) * W + j) * 3 + 1] = std::min(intensite_pixel, 255.); // has_inter ? 255 : 0; // coordo verte
            img[((H - i - 1) * W + j) * 3 + 2] = std::min(intensite_pixel, 255.); // has_inter ? 255 : 0; // coordo bleue
        }
    }

    stbi_write_png("Sphere.png", W, H, 3, &img[0], 0);

    return 0;
}
