// assignment1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <limits>

// Vector structure for 3D operations
struct Vec3 {
    float x, y, z;

    Vec3 operator-(const Vec3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vec3 operator+(const Vec3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vec3 operator*(float scalar) const { return { x * scalar, y * scalar, z * scalar }; }

    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 normalize() const {
        float length = std::sqrt(dot(*this));
        return { x / length, y / length, z / length };
    }
};

// Ray class
struct Ray {
    Vec3 origin;
    Vec3 direction;
};

// Abstract surface class (base class for Plane and Sphere)
class Surface {
public:
    virtual bool intersect(const Ray& ray, float& t) const = 0;
};

// Plane class
class Plane : public Surface {
    float y; // y = -2 for the given plane

public:
    Plane(float y_) : y(y_) {}

    bool intersect(const Ray& ray, float& t) const override {
        if (ray.direction.y == 0) return false; // Parallel to the plane
        t = (y - ray.origin.y) / ray.direction.y;
        return t > 0;
    }
};

// Sphere class
class Sphere : public Surface {
    Vec3 center;
    float radius;

public:
    Sphere(const Vec3& c, float r) : center(c), radius(r) {}

    bool intersect(const Ray& ray, float& t) const override {
        Vec3 oc = ray.origin - center;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant > 0) {
            float t1 = (-b - std::sqrt(discriminant)) / (2.0f * a);
            float t2 = (-b + std::sqrt(discriminant)) / (2.0f * a);
            t = (t1 > 0) ? t1 : t2;
            return t > 0;
        }
        return false;
    }
};

// Camera class
class Camera {
    Vec3 eye;
    float l, r, b, t, d;
    int nx, ny;

public:
    Camera(const Vec3& eye_, float l_, float r_, float b_, float t_, float d_, int nx_, int ny_)
        : eye(eye_), l(l_), r(r_), b(b_), t(t_), d(d_), nx(nx_), ny(ny_) {
    }

    Ray getRay(int ix, int iy) const {
        float u = l + (r - l) * (ix + 0.5f) / nx;
        float v = b + (t - b) * (iy + 0.5f) / ny;
        Vec3 pixel = { u, v, -d };
        return { eye, pixel.normalize() };
    }
};

// Scene class
class Scene {
    std::vector<Surface*> surfaces;

public:
    void addSurface(Surface* surface) {
        surfaces.push_back(surface);
    }

    Vec3 trace(const Ray& ray, float tMin, float tMax) const {
        float closest_t = tMax;
        const Surface* hitSurface = nullptr;

        for (const Surface* surface : surfaces) {
            float t;
            if (surface->intersect(ray, t) && t < closest_t && t > tMin) {
                closest_t = t;
                hitSurface = surface;
            }
        }

        return hitSurface ? Vec3{ 1.0f, 1.0f, 1.0f } : Vec3{ 0.0f, 0.0f, 0.0f };
    }
};

// Image resolution and pixel data
const int nx = 512;
const int ny = 512;
std::vector<unsigned char> pixels(nx* ny * 3); // RGB pixel data

// Generate the ray-traced image
void generateImage(const Camera& camera, const Scene& scene) {
    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            Ray ray = camera.getRay(x, y);
            Vec3 color = scene.trace(ray, 0.001f, std::numeric_limits<float>::max());

            int index = (y * nx + x) * 3;
            pixels[index] = static_cast<unsigned char>(color.x * 255);     // Red
            pixels[index + 1] = static_cast<unsigned char>(color.y * 255); // Green
            pixels[index + 2] = static_cast<unsigned char>(color.z * 255); // Blue
        }
    }
}

// Display callback for OpenGL
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(nx, ny, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0]);
    glFlush();
}

// Main function
int main(int argc, char** argv) {
    // Set up camera
    Camera camera({ 0, 0, 0 }, -0.1f, 0.1f, -0.1f, 0.1f, 0.1f, nx, ny);

    // Set up scene
    Scene scene;
    scene.addSurface(new Plane(-2));
    scene.addSurface(new Sphere({ -4, 0, -7 }, 1));
    scene.addSurface(new Sphere({ 0, 0, -7 }, 2));
    scene.addSurface(new Sphere({ 4, 0, -7 }, 1));

    // Generate the image
    generateImage(camera, scene);

    // Initialize OpenGL
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(nx, ny);
    glutCreateWindow("Ray Tracer");
    glutDisplayFunc(display);
    glClearColor(0, 0, 0, 1);
    glutMainLoop();

    return 0;
}
