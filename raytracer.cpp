//
// Created by Jamie Fox on 06/02/2016.
//

#include <iostream>
#include <vector>
#include <math.h>
#include <fstream>

#include "Vector3D.h"
#include "Colour.h"
#include "Sphere.h"

#define CANVAS_WIDTH 1000
#define CANVAS_HEIGHT 1000
#define CANVAS_DEPTH 700
#define MAX_RAY_DEPTH 1000
#define BIAS 1e-4

Colour trace(const Vector3D &rayOrig, const Vector3D &rayDir, const std::vector<Sphere> &spheres, int recursionDepth) {
    Colour c(0, 0, 0);
    const Sphere *intersectSphere = nullptr;
    float closestIntersect = INFINITY;

    for (int i = 0; i < spheres.size(); ++i) {
        float t0 = INFINITY;
        if (spheres[i].intersect(rayOrig, rayDir, t0)) {
            if (t0 < closestIntersect) {
                intersectSphere = &spheres[i];
                closestIntersect = t0;
            }
        }
    }

    if (intersectSphere) {
        //emmission
        c += intersectSphere->emissionColour;

        //ambient reflection
        c += intersectSphere->surfaceColour * 0.2;

        Vector3D pIntersection = rayOrig + rayDir * closestIntersect;
        Vector3D nIntersection = (pIntersection - intersectSphere->centre).norm();

        //diffuse reflection
        for (int i = 0; i < spheres.size(); ++i) {
            if (spheres[i].emissionColour.red > 0 ||
                spheres[i].emissionColour.green > 0 ||
                spheres[i].emissionColour.blue > 0) {//it's a light!

                Vector3D lightRay = (spheres[i].centre - pIntersection).norm();
                bool blocked = false;

                for (int j = 0; j < spheres.size(); ++j) {
                    if (i != j) {
                        float t0 = 0;
                        if (spheres[j].intersect(pIntersection + nIntersection * BIAS, lightRay, t0)) {
                            blocked = true;
                            break;
                        }
                    }
                }

                if (!blocked) {
                    c += spheres[i].emissionColour * intersectSphere->surfaceColour * intersectSphere->kd *
                            std::max(float(0), nIntersection.dot(lightRay));
                }
            }
        }

        //specular reflection
        if (intersectSphere->ks > 0 && recursionDepth < MAX_RAY_DEPTH) {
            Vector3D reflRay = (rayDir - nIntersection * 2 * rayDir.dot(nIntersection)).norm();
            c += trace(pIntersection + nIntersection * BIAS, reflRay, spheres, recursionDepth + 1)
                 * intersectSphere->ks; //todo: Phong
        }

    }

    return c;
}

void render(const std::vector<Sphere> &spheres) {
    Colour *pixels = new Colour[CANVAS_WIDTH * CANVAS_HEIGHT], *pixel = pixels;

    for (int i = 0; i < CANVAS_WIDTH; ++i) {
        for (int j = 0; j < CANVAS_HEIGHT; ++j, ++pixel) {
            float x = i - (CANVAS_WIDTH / 2 - 1);
            float y = j - (CANVAS_HEIGHT / 2 - 1);
            Vector3D primaryRay(x, y, CANVAS_DEPTH);
            *pixel = trace(Vector3D(0, 0, 0), primaryRay.norm(), spheres, 0);
        }
    }

    std::ofstream ofs("test.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << CANVAS_WIDTH << " " << CANVAS_HEIGHT << "\n255\n";
    for (unsigned i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; ++i) {
        ofs << (unsigned char)(std::min(float(1), pixels[i].red) * 255) <<
        (unsigned char)(std::min(float(1), pixels[i].green) * 255) <<
        (unsigned char)(std::min(float(1), pixels[i].blue) * 255);
    }
    ofs.close();

    delete[] pixels;
}

int main(int argc, char const *argv[]) {
    std::vector<Sphere> spheres;

    spheres.push_back(Sphere(Vector3D(12280, 0, 800), 12000, Colour(0.9, 0.9, 0.9), 0.8, 0.2));

    for (int i = -100; i <= 200; i += 150) {
        for (int j = -100; j <= 200; j += 150) {
            spheres.push_back(Sphere(Vector3D(i, j, 600), 70.0, Colour(1.0, ((float)i + 100)/350, ((float)j + 100)/350), 0.6, 0.4));
            spheres.push_back(Sphere(Vector3D(i, j, 750), 70.0, Colour(((float)i + 100)/350, 1.0, ((float)j + 100)/350), 0.6, 0.4));
            spheres.push_back(Sphere(Vector3D(i, j, 900), 70.0, Colour(((float)i + 100)/350, ((float)j + 100)/400, 1.0), 0.6, 0.4));
        }
    }

    //spheres.push_back(Sphere(Vector3D(30.0, 10.0, 600.0), 70.0, Colour(0.90, 0.20, 0.20), 0.6, 0.4));
    //spheres.push_back(Sphere(Vector3D(-150, -150, 620.0), 90.0, Colour(0.00, 0.7, 0.7), 0.4, 0.6));
    //spheres.push_back(Sphere(Vector3D(5.0, -1, -15), 2, Colour(0.90, 0.76, 0.46)));
    //spheres.push_back(Sphere(Vector3D(5.0, 0, -25), 3, Colour(0.65, 0.77, 0.97)));
    //spheres.push_back(Sphere(Vector3D(-5.5, 0, -15), 3, Colour(0.90, 0.90, 0.90)));

    //lights
    spheres.push_back(Sphere(Vector3D(-1000, -1000, 700.0), 150.0, Colour(0, 0, 0), Colour(1, 1, 1)));
    //spheres.push_back(Sphere(Vector3D(200, 200, 400.0), 40.0, Colour(0, 0, 0), Colour(1, 1, 1)));
    //spheres.push_back(Sphere(Vector3D(10, 10, -100), 100.0, Colour(0, 0, 0), Colour(1, 1, 1)));

    render(spheres);

    return 0;
}
