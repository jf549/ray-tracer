//
// Created by Jamie Fox on 06/02/2016.
//

#ifndef RAY_TRACER_SPHERE_H
#define RAY_TRACER_SPHERE_H


#include "Vector3D.h"
#include "Colour.h"

class Sphere {
public:
    Vector3D centre;
    float radius, radius2; //radius^2
    Colour surfaceColour, emissionColour;
    float kd, ks;

    //objects
    Sphere(const Vector3D &c, float r, const Colour &sc, float d, float s);

    //light sources
    Sphere(const Vector3D &c, float r, const Colour &sc, const Colour &ec);

    bool intersect(const Vector3D &rayOrig, const Vector3D &rayDir, float &t0) const;
};


#endif //RAY_TRACER_SPHERE_H
