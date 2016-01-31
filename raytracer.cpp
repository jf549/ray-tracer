#include <iostream>
#include <vector>
#include <math.h>
#include <fstream>

#define CANVAS_WIDTH 500
#define CANVAS_HEIGHT 500
#define CANVAS_DEPTH 400
#define MAX_RAY_DEPTH 10
#define BIAS 1e-4

class Colour {
public:
    float red, green, blue;

    Colour(float r, float g, float b) : red(r), blue(b), green(g) {}

    Colour() : red(0), blue(0), green(0) {}

    Colour &operator+=(const Colour &rhs) {
        red += rhs.red, green += rhs.green, blue += rhs.blue;
        return *this;
    }

    Colour &operator-=(const Colour &rhs) {
        red -= rhs.red, green -= rhs.green, blue -= rhs.blue;
        return *this;
    }

    Colour &operator*=(const Colour &rhs) {
        red *= rhs.red, green *= rhs.green, blue *= rhs.blue;
        return *this;
    }

    Colour &operator*=(const float rhs) {
        red *= rhs, green *= rhs, blue *= rhs;
        return *this;
    }
};

inline Colour operator+(Colour lhs, const Colour &rhs) {
    lhs += rhs;
    return lhs;
}

inline Colour operator-(Colour lhs, const Colour &rhs) {
    lhs -= rhs;
    return lhs;
}

inline Colour operator*(Colour lhs, const Colour &rhs) {
    lhs *= rhs;
    return lhs;
}

inline Colour operator*(Colour lhs, const float rhs) {
    lhs *= rhs;
    return lhs;
}

class Vector3D {
public:
    float x, y, z;

    Vector3D(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}

    float length() {
        return sqrtf(x * x + y * y + z * z);
    }

    Vector3D &norm() {
        float len = length();

        if (len > 0) {
            x /= len, y /= len, z /= len;
        }

        return *this;
    }

    float dot(const Vector3D &rhs) const {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    Vector3D &operator+=(const Vector3D &rhs) {
        x += rhs.x, y += rhs.y, z += rhs.z;
        return *this;
    }

    Vector3D &operator-=(const Vector3D &rhs) {
        x -= rhs.x, y -= rhs.y, z -= rhs.z;
        return *this;
    }

    Vector3D &operator*=(const float rhs) {
        x *= rhs, y *= rhs, z *= rhs;
        return *this;
    }

    Vector3D operator-() const {
        return Vector3D(-x, -y, -z);
    }
};

inline Vector3D operator+(Vector3D lhs, const Vector3D &rhs) {
    lhs += rhs;
    return lhs;
}

inline Vector3D operator-(Vector3D lhs, const Vector3D &rhs) {
    lhs -= rhs;
    return lhs;
}

inline Vector3D operator*(Vector3D lhs, const float rhs) {
    lhs *= rhs;
    return lhs;
}

class Sphere {
public:
    Vector3D centre;
    float radius, radius2; //radius^2
    Colour surfaceColour, emissionColour;
    float kd, ks;

    //objects
    Sphere(const Vector3D &c,
           float r,
           const Colour &sc,
           float d, float s) :
            centre(c), radius(r), radius2(r * r), surfaceColour(sc), kd(d), ks(s) {}

    //light sources
    Sphere(const Vector3D &c,
           float r,
           const Colour &sc, const Colour &ec) :
            centre(c), radius(r), radius2(r * r), surfaceColour(sc), emissionColour(ec) {}

    bool intersect(const Vector3D &rayOrig, const Vector3D &rayDir, float &t0) const {
        Vector3D l = centre - rayOrig;

        float tca = l.dot(rayDir);

        if (tca < 0) {
            return false;
        }

        float d2 = l.dot(l) - tca * tca;

        if (d2 > radius2) {
            return false;
        }

        float thc = sqrtf(radius2 - d2);
        t0 = tca - thc;

        return true;
    }
};

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
        //ambient reflection
        c += intersectSphere->surfaceColour * 0.2;
        c += intersectSphere->emissionColour;

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
                 * intersectSphere->ks; //todo: phong
        }

    }

    return c;
}

void render(const std::vector<Sphere> &spheres) {
    Colour pixels[CANVAS_WIDTH * CANVAS_HEIGHT], *pixel = pixels;

    for (int i = 0; i < CANVAS_WIDTH; ++i) {
        for (int j = 0; j < CANVAS_HEIGHT; ++j, ++pixel) {
            float x = i - (CANVAS_WIDTH / 2 - 1);
            float y = j - (CANVAS_HEIGHT / 2 - 1);
            Vector3D primaryRay(x, y, CANVAS_DEPTH);
            *pixel = trace(Vector3D(0, 0, 0), primaryRay.norm(), spheres, 0);
        }
    }

    std::ofstream ofs("rayTrace.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << CANVAS_WIDTH << " " << CANVAS_HEIGHT << "\n255\n";
    for (unsigned i = 0; i < CANVAS_WIDTH * CANVAS_HEIGHT; ++i) {
        ofs << (unsigned char)(std::min(float(1), pixels[i].red) * 255) <<
        (unsigned char)(std::min(float(1), pixels[i].green) * 255) <<
        (unsigned char)(std::min(float(1), pixels[i].blue) * 255);
    }
    ofs.close();
}

int main(int argc, char const *argv[]) {
    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vector3D(30.0, 10.0, 600.0), 70.0, Colour(0.90, 0.20, 0.20), 0.6, 0.4));
    spheres.push_back(Sphere(Vector3D(-150, -150, 620.0), 90.0, Colour(0.00, 0.7, 0.7), 0.4, 0.6));
    //spheres.push_back(Sphere(Vector3D(5.0, -1, -15), 2, Colour(0.90, 0.76, 0.46)));
    //spheres.push_back(Sphere(Vector3D(5.0, 0, -25), 3, Colour(0.65, 0.77, 0.97)));
    //spheres.push_back(Sphere(Vector3D(-5.5, 0, -15), 3, Colour(0.90, 0.90, 0.90)));

    spheres.push_back(Sphere(Vector3D(-300, -300, 400.0), 40.0, Colour(0, 0, 0), Colour(1, 1, 1))); //light
    //spheres.push_back(Sphere(Vector3D(200, 200, 400.0), 40.0, Colour(0, 0, 0), Colour(1, 1, 1))); //light
    spheres.push_back(Sphere(Vector3D(10, 10, -100), 100.0, Colour(0, 0, 0), Colour(1, 1, 1)));

    render(spheres);

    return 0;
}