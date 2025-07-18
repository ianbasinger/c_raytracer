#include "raytracer.h"

// this relies on lodepng to encode to .PNG, however this can be re-worked to not use any external libraries, output would instead be .PPM


// vector 
typedef struct { float x, y, z; } Vec3;

Vec3 vec_add(Vec3 a, Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 vec_sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 vec_mul(Vec3 a, float s) { return (Vec3){a.x * s, a.y * s, a.z * s}; }
float vec_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 vec_norm(Vec3 v) {
    float len = sqrtf(vec_dot(v, v));
    return vec_mul(v, 1.0f / len);
}
Vec3 vec_reflect(Vec3 d, Vec3 n) {
    return vec_sub(d, vec_mul(n, 2 * vec_dot(d, n)));
}

typedef struct {
    Vec3 center;
    float radius;
    unsigned char color[3];
    float reflectivity;
} Sphere;

Vec3 light_dir = {-1, -1, -1};

int intersect_sphere(Vec3 ray_orig, Vec3 ray_dir, Sphere s, float *t_out) {
    Vec3 oc = vec_sub(ray_orig, s.center);
    float b = 2 * vec_dot(ray_dir, oc);
    float c = vec_dot(oc, oc) - s.radius * s.radius;
    float disc = b * b - 4 * c;
    if (disc < 0) return 0;
    float t = (-b - sqrtf(disc)) / 2;
    if (t < 0.001f) return 0;
    *t_out = t;
    return 1;
}

int trace(Vec3 orig, Vec3 dir, Sphere *spheres, int sphere_count, int depth, unsigned char *out_color) {
    float closest = 1e9;
    Sphere *hit_obj = NULL;
    float t_hit = 0;

    for (int i = 0; i < sphere_count; i++) {
        float t = 0;
        if (intersect_sphere(orig, dir, spheres[i], &t)) {
            if (t < closest) {
                closest = t;
                t_hit = t;
                hit_obj = &spheres[i];
            }
        }
    }

    if (!hit_obj) return 0;

    Vec3 hit_point = vec_add(orig, vec_mul(dir, t_hit));
    Vec3 normal = vec_norm(vec_sub(hit_point, hit_obj->center));
    Vec3 to_light = vec_norm(vec_mul(light_dir, -1));

    // for shadows
    int in_shadow = 0;
    Vec3 shadow_orig = vec_add(hit_point, vec_mul(normal, 0.001f));
    for (int i = 0; i < sphere_count; i++) {
        float t = 0;
        if (&spheres[i] != hit_obj && intersect_sphere(shadow_orig, to_light, spheres[i], &t)) {
            in_shadow = 1;
            break;
        }
    }

    float diffuse = in_shadow ? 0.1f : fmaxf(0.0f, vec_dot(normal, to_light));

    // the reflection
    unsigned char refl_color[3] = {0, 0, 0};
    if (depth < 2 && hit_obj->reflectivity > 0.0f) {
        Vec3 reflect_dir = vec_reflect(dir, normal);
        Vec3 reflect_orig = vec_add(hit_point, vec_mul(normal, 0.001f));
        trace(reflect_orig, vec_norm(reflect_dir), spheres, sphere_count, depth + 1, refl_color);
    }

    for (int i = 0; i < 3; i++) {
        float base = hit_obj->color[i] * diffuse;
        float refl = refl_color[i];
        out_color[i] = (unsigned char)(base * (1 - hit_obj->reflectivity) + refl * hit_obj->reflectivity);
    }

    return 1;
}

float randf(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

int main() {
    int width = 800, height = 600;
    unsigned char *img = malloc(width * height * 4);
    srand((unsigned int)time(NULL));
    light_dir = vec_norm(light_dir);

    int max_spheres = 6;
    Sphere spheres[max_spheres + 1]; // floor here

    for (int i = 0; i < max_spheres; i++) {
        spheres[i].center = (Vec3){
            randf(-4, 4),
            randf(-1, 1),
            randf(-8, -4)
        };
        spheres[i].radius = randf(0.5, 1.2);
        spheres[i].color[0] = rand() % 256;
        spheres[i].color[1] = rand() % 256;
        spheres[i].color[2] = rand() % 256;
        spheres[i].reflectivity = randf(0.0f, 0.8f);
    }

    // floor
    spheres[max_spheres].center = (Vec3){0, -5001, 0};
    spheres[max_spheres].radius = 5000.0f;
    spheres[max_spheres].color[0] = 255;
    spheres[max_spheres].color[1] = 255;
    spheres[max_spheres].color[2] = 255;
    spheres[max_spheres].reflectivity = 0.0f;

    int sphere_count = max_spheres + 1;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float u = (x - width / 2.0f) / width;
            float v = (y - height / 2.0f) / height;
            Vec3 ray_dir = vec_norm((Vec3){u, -v, -1});
            Vec3 ray_orig = {0, 0, 0};

            unsigned char color[3] = {20, 20, 30};
            trace(ray_orig, ray_dir, spheres, sphere_count, 0, color);

            int idx = 4 * (y * width + x);
            img[idx + 0] = color[0];
            img[idx + 1] = color[1];
            img[idx + 2] = color[2];
            img[idx + 3] = 255;
        }
    }

    // output (replace each run), take raw img assuming 32 bit RGBA and compress to .png
    lodepng_encode32_file("raytrace_output.png", img, width, height);
    free(img);
    return 0;
}