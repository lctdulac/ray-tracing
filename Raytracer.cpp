// Raytracer.cpp : Defines the entry point for the console application.
#define _CRT_SECURE_NO_WARNINGS // for Visual Studio 2017 (maybe 2015 as well)

// Imports

#include <iostream>
#include <vector>

#include <random>
std::default_random_engine engine;
std::uniform_real_distribution<double> uniform(0, 1);

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

Vector random_cos(Vector &N)
{
    // renvoie direction aléatoire en cos autour d'une normale
    // génération gausienne aléatoire (méthode de Box-Muller)

    double r1 = uniform(engine);
    double r2 = uniform(engine);
    Vector direction_aleatoire_repere_local(cos(2 * M_PI * r1) * sqrt(1 - r2),
                                            sin(2 * M_PI * r1) * sqrt(1 - r2),
                                            sqrt(r2));

    Vector aleatoire(uniform(engine), uniform(engine), uniform(engine));
    // produit vectoriel
    Vector tangent1 = N.cross(aleatoire);
    tangent1.normalize();
    Vector tangent2 = tangent1.cross(N);

    Vector direction_aleatoire = N * direction_aleatoire_repere_local.z +
                                 tangent1 * direction_aleatoire_repere_local.x +
                                 tangent2 * direction_aleatoire_repere_local.y;
    return direction_aleatoire;
};

// Renvoie la couleur (récursivement, pour gérer les miroirs et éclairage indirect)
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
        // // gestion naive de l'éclairage diffu
        // if (sphere_id == 0) // premiere sphère = lumière. on renvoie la couleur de la lumière si on l'intersecte
        // {
        //     return Sc.lumiere->albedo * Sc.intensite_lumiere / 4 * M_PI * Sc.lumiere->r * Sc.lumiere->r;
        // };

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

            // // contribution directe (commenté car on gère maintenant les sources étendues)

            // // pour décoller un peu l'origine du rayon (probleme numérique)
            // Ray ray_light(P + N * 0.01, (Sc.position_lumiere - P).getNormalized());
            // Vector P_light, N_light;
            // int sphere_id_light;
            // double t_light;

            // bool has_inter_light = Sc.intersect(ray_light, P_light, N_light, sphere_id_light, t_light);

            // // valeurs au carré pour éviter les calculs de sqrt
            // double d_light2 = (Sc.position_lumiere - P).getNorm2();
            // if (has_inter_light && t_light * t_light < d_light2)
            // {
            //     // ombre
            //     intensite_pixel = Vector(0, 0, 0);
            // }
            // else
            // {
            //     // max pour pas considérer les cas ou la lumière est derrière
            //     intensite_pixel = Sc.spheres[sphere_id].albedo * Sc.intensite_lumiere * std::max(0., (Sc.position_lumiere - P).getNormalized().dot(N)) / (Sc.position_lumiere - P).getNorm2();
            // }

            // contrib directe non naive
            Vector axeOP = (P - Sc.lumiere->center).getNormalized();

            Vector dir_aleatoire = random_cos(axeOP);
            Vector point_aleatoire = dir_aleatoire * Sc.lumiere->r + Sc.lumiere->center;
            Vector wi = (point_aleatoire - P).getNormalized();
            double d_light2 = (point_aleatoire - P).getNorm2();
            Vector N_ter = dir_aleatoire;

            Ray ray_light(P + N * 0.01, wi);
            Vector P_light, N_light;
            int sphere_id_light;
            double t_light;

            bool has_inter_light = Sc.intersect(ray_light, P_light, N_light, sphere_id_light, t_light);
            if (has_inter_light && t_light * t_light < d_light2 * 0.99) // petit hack pour être sur que ce n'est pas la sphere de lumiere elle meme
            {
                // ombre
                intensite_pixel = Vector(0, 0, 0);
            }
            else
            {
                intensite_pixel = Sc.spheres[sphere_id].albedo * (Sc.intensite_lumiere / (4 * M_PI * d_light2) * std::max(0., N.dot(wi)) * N_ter.dot(wi * (-1)) / axeOP.dot(dir_aleatoire));
            };

            // contribution indirecte(même code que miroir)

            Vector direction_aleatoire = random_cos(N);
            Ray rayon_aleatoire(P + N * 0.01, direction_aleatoire);
            intensite_pixel += getColor(rayon_aleatoire, Sc, nbrebonds - 1) * Sc.spheres[sphere_id].albedo;
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

    // rayons
    const int nrays = 20;

    // image
    std::vector<unsigned char>
        img(W * H * 3);

    // objets (miroir ou transp)
    // bool mirror = false, bool transp = false
    // false, false = sphere diffuse
    Vector couleur_1(1, 1, 1);
    Vector couleur_2(1, 0, 0);
    double r = 10;
    Sphere s1(Vector(0, 0, -55), couleur_1, r);
    Sphere s1b(Vector(-15, 0, -35), couleur_1, r, false, true); // transp
    Sphere s1c(Vector(15, 0, -75), couleur_1, r, true);         // mirror
    // Sphere s2(Vector(15, 0, -55), couleur_2, r);

    // plans = grandes sphères

    // sol blanc (à -2000, avec un rayon de 2000 - le rayon : la boule est posée sur le sol)
    Sphere sol(Vector(0, -2000 - 20, 0), Vector(1, 1, 1), 2000);

    // plafond blanc (à +2000, avec un rayon de 2000 + un peu d'espace (100)
    Sphere plafond(Vector(0, 2000 + 100, 0), Vector(1, 1, 1), 2000);

    // murs vert et bleu
    Sphere mur_gauche(Vector(-2000 - 50, 0, 0), Vector(0, 0.5, 0.5), 2000);
    Sphere mur_droit(Vector(+2000 + 50, 0, 0), Vector(0, 0.5, 0.5), 2000);
    Sphere mur_fond(Vector(0, 0, -2000 - 100), Vector(0, 0, 1), 2000);

    // lumière sphérique
    Sphere slum(Vector(15, 70, -30), Vector(1, 1, 1), 5);

    // scène (avec lumière)
    Scene Sc;
    Sc.addSphere(slum); // tj en premiere position

    Sc.addSphere(s1);
    Sc.addSphere(s1b);
    Sc.addSphere(s1c);
    // Sc.addSphere(s2);
    Sc.addSphere(sol);
    Sc.addSphere(plafond);
    Sc.addSphere(mur_gauche);
    Sc.addSphere(mur_droit);
    Sc.addSphere(mur_fond);
    Sc.position_lumiere = Vector(15, 70, -40);
    Sc.intensite_lumiere = 60000000; // correction gamma implique une intensité nécessaire plus forte
    Vector position_camera(0., 0., 0.);
    double focus_distance = 55;

    // lumiere sphérique
    Sc.lumiere = &slum;

#pragma omp parallel for // plusieurs lignes en même temps
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {

            // 5 rebonds maxs (ex: deux miroirs face a face = bug)

            Vector color(0., 0., 0.);
            for (int k = 0; k < nrays; k++) // plusieurs rayons par pixel (image moins bruitée)
            {
                // methode Box Muller pour l'anti-aliasing

                double r1 = uniform(engine);
                double r2 = uniform(engine);

                double R = sqrt(-2 * log(r1));
                double dx = R * cos(2 * M_PI * r2);
                double dy = R * sin(2 * M_PI * r2);

                // gestion de la profondeur de champ (aperture carrée)
                double dx_aperture = (uniform(engine) - 0.5) * 5.;
                double dy_aperture = (uniform(engine) - 0.5) * 5.;

                Vector origin = Vector(0, 0, 0);
                Vector dir(j - W / 2 + 0.5 + dx, i - H / 2 + 0.5 + dy, -W / (2 * tan(fov / 2)));
                dir.normalize();

                Vector destination = position_camera + dir * focus_distance;
                Vector new_origin = position_camera + Vector(dx_aperture, dy_aperture, 0);

                // sans profondeur : Ray rayon(origin, dir);
                // avec profondeur :
                Ray rayon(new_origin, (destination - new_origin).getNormalized());
                color += getColor(rayon, Sc, 5);
            }

            // correction gamma = std.pow
            img[((H - i - 1) * W + j) * 3 + 0] = std::min(std::pow(color.x, 1 / 2.2), 255.); // has_inter ? 255 : 0; // coordo rouge
            img[((H - i - 1) * W + j) * 3 + 1] = std::min(std::pow(color.y, 1 / 2.2), 255.); // has_inter ? 255 : 0; // coordo verte
            img[((H - i - 1) * W + j) * 3 + 2] = std::min(std::pow(color.z, 1 / 2.2), 255.); // has_inter ? 255 : 0; // coordo bleue
        }
    }

    stbi_write_png("Sphere.png", W, H, 3, &img[0], 0);

    return 0;
}
