// Raytracer.cpp : Defines the entry point for the console application.
#define _CRT_SECURE_NO_WARNINGS // for Visual Studio 2017 (maybe 2015 as well)

// Imports

#include <iostream>
#include <vector>

#include "Vector.h"
#include "Ray.h"
#include "Sphere.h"
#include "Scene.h"

using namespace std;

// Fonctions importées

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

//// Fonctions

// Renvoie la couleur (récursivement, pour gérer les miroirs)
Vector getColor(Ray &rayon, Scene &Sc, int nbrebonds)
{

    if (nbrebonds == 0)
    {
        return Vector(0, 0, 0);
    }

    Vector P, N; // Vecteur point et vecteur normal

    // si on déclare ici L et d, ça ne marche pas.
    // Vector L = (position_lumiere - P).getNormalized(); // Vector unitaire towards the light
    // double d = (position_lumiere - P).getNorm();       // distance à la lumière

    int sphere_id;
    double t;
    bool has_inter = Sc.intersect(rayon, P, N, sphere_id, t);

    Vector intensite_pixel(0, 0, 0);

    if (has_inter)
    {
        if (Sc.spheres[sphere_id].miroir)
        {
            Vector direction_miroir = rayon.dir - N * 2 * rayon.dir.dot(N);
            Ray rayon_miroir(P + N * 0.01, direction_miroir);
            intensite_pixel = getColor(rayon_miroir, Sc, nbrebonds - 1);
        }
        else if (Sc.spheres[sphere_id].transparent)
        {
            double n1 = 1.0;
            double n2 = 1.3;

            // on inverse si on sort de la sphere
            Vector normale_pour_transparence(N);
            if (rayon.dir.dot(N) > 0)
            {
                n1 = 1.3;
                n2 = 1.0;

                normale_pour_transparence = N * (-1);
            }

            // si le radical est inférieur, réflexion totale
            double radical = 1 - std::pow((n1 / n2), 2) * (1 - std::pow(rayon.dir.dot(normale_pour_transparence), 2));
            if (radical > 0)
            {
                Vector direction_refract = (rayon.dir - normale_pour_transparence * rayon.dir.dot(normale_pour_transparence)) * n1 / n2 - normale_pour_transparence * sqrt(radical);
                Ray rayon_refract(P - normale_pour_transparence * 0.01, direction_refract); // on décale dans l'autre sens
                intensite_pixel = getColor(rayon_refract, Sc, nbrebonds - 1);
            }
        }
        else
        {

            // pour décoller un peu l'origine du rayon (probleme numérique)
            Ray ray_light(P + N * 0.01, (Sc.position_lumiere - P).getNormalized());
            Vector P_light, N_light;
            int sphere_id_light;
            double t_light;

            bool has_inter_light = Sc.intersect(ray_light, P_light, N_light, sphere_id_light, t_light);

            // valeurs au carré pour éviter les calculs de sqrt
            double d_light2 = (Sc.position_lumiere - P).getNorm2();
            if (has_inter_light && t_light * t_light < d_light2)
            {
                // ombre
                intensite_pixel = Vector(0, 0, 0);
            }
            else
            {
                // max pour pas considérer les cas ou la lumière est derrière
                intensite_pixel = Sc.spheres[sphere_id].albedo * Sc.intensite_lumiere * std::max(0., (Sc.position_lumiere - P).getNormalized().dot(N)) / (Sc.position_lumiere - P).getNorm2();
            }
        }
    };

    return intensite_pixel;
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

    // objets (miroir ou transp)
    Vector couleur(1, 0, 0);
    double r = 10;
    Sphere s1(Vector(-15, 0, -55), couleur, r, false, true); //transp
    Sphere s2(Vector(15, 0, -55), couleur, r, true);

    // plans = grandes sphères

    // sol blanc (à -2000, avec un rayon de 2000 - le rayon : la boule est posée sur le sol)
    Sphere sol(Vector(0, -2000 - 20, 0), Vector(1, 1, 1), 2000);

    // plafond blanc (à +2000, avec un rayon de 2000 + un peu d'espace (100)
    Sphere plafond(Vector(0, 2000 + 100, 0), Vector(1, 1, 1), 2000);

    // murs vert et bleu
    Sphere mur_gauche(Vector(-2000 - 50, 0, 0), Vector(0, 1, 0), 2000);
    Sphere mur_droit(Vector(+2000 + 50, 0, 0), Vector(0, 0, 1), 2000);
    Sphere mur_fond(Vector(0, 0, -2000 - 100), Vector(0, 1, 1), 2000);

    // scène (avec lumière)

    Scene Sc;
    Sc.addSphere(s1);
    Sc.addSphere(s2);
    Sc.addSphere(sol);
    Sc.addSphere(plafond);
    Sc.addSphere(mur_gauche);
    Sc.addSphere(mur_droit);
    Sc.addSphere(mur_fond);
    Sc.position_lumiere = Vector(15, 70, -40);
    Sc.intensite_lumiere = 1000000000; // correction gamma implique une intensité nécessaire plus forte

    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            Vector origin = Vector(0, 0, 0);
            Vector dir(j - W / 2, i - H / 2, -W / (2 * tan(fov / 2)));
            dir.normalize();

            Ray rayon(origin, dir);

            // 5 rebonds maxs (ex: deux miroirs face a face = bug)
            Vector color = getColor(rayon, Sc, 5);

            // correction gamma = std.pow
            img[((H - i - 1) * W + j) * 3 + 0] = std::min(std::pow(color.x, 1 / 2.2), 255.); // has_inter ? 255 : 0; // coordo rouge
            img[((H - i - 1) * W + j) * 3 + 1] = std::min(std::pow(color.y, 1 / 2.2), 255.); // has_inter ? 255 : 0; // coordo verte
            img[((H - i - 1) * W + j) * 3 + 2] = std::min(std::pow(color.z, 1 / 2.2), 255.); // has_inter ? 255 : 0; // coordo bleue
        }
    }

    stbi_write_png("Sphere.png", W, H, 3, &img[0], 0);

    return 0;
}
