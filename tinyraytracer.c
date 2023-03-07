#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define DEPTH 10

/**
 * @brief max of two floats
 *
 * @param a
 * @param b
 * @return float
 */
inline float MAX(const float a, const float b) { return a > b ? a : b; }

/**
 * @brief min of two floats
 *
 * @param a
 * @param b
 * @return float
 */
inline float MIN(const float a, const float b) { return a < b ? a : b; }

/**
 * @brief 3D vector struct
 *
 */
typedef struct __vec3 {
    float x, y, z;
} vec3;

/**
 * @brief Material struct
 *
 */
typedef struct __Material {
    float refractive_index;
    float albedo[4];
    vec3  diffuse_color;
    float specular_exponent;
} Material;

/**
 * @brief Sphere struct
 *
 */
typedef struct __Sphere {
    vec3     center;
    float    radius;
    Material material;
} Sphere;

const int   width  = 1024;
const int   height = 768;
const float fov    = 1.05f;

const vec3 BACKGROUND_COLOR = {.1955f, .9377f, .6533f};
const vec3 BOX_COLOR1       = {.9822f, .6044f, .1733f};
const vec3 BOX_COLOR2       = {.9822f, .2f, .1733f};

const Material ivory  = {1.f, {.9f, .5f, .1f, 0.f}, {.4f, .4f, .3f}, 50.f};
const Material glass  = {1.5f, {0.f, .9f, .1f, .8f}, {.6f, .7f, .8f}, 125.f};
const Material rubber = {1.f, {1.4f, .3f, 0.f, 0.f}, {.4f, .1f, .1f}, 10.f};
const Material mirror = {1.f, {0.f, 16.f, .8f, 0.f}, {1.f, 1.f, 1.f}, 1425.f};

/**
 * @brief Initialize a vec3
 *
 * @param v
 */
void init_vec3(vec3 *v) {
    v->x = 0.f;
    v->y = 0.f;
    v->z = 0.f;
}

/**
 * @brief Initialize a Material
 *
 * @param m
 */
void init_Material(Material *m) {
    m->refractive_index = 1.f;
    m->albedo[0]        = 2.f;
    m->albedo[1]        = 0.f;
    m->albedo[2]        = 0.f;
    m->albedo[3]        = 0.f;
    init_vec3(&m->diffuse_color);
    m->specular_exponent = 0.f;
}

/**
 * @brief add two vec3s
 *
 * @param u
 * @param v
 * @return vec3
 */
vec3 add(const vec3 u, const vec3 v) { return (vec3){u.x + v.x, u.y + v.y, u.z + v.z}; }

/**
 * @brief subtract two vec3s
 *
 * @param u
 * @param v
 * @return vec3
 */
vec3 subtract(const vec3 u, const vec3 v) { return (vec3){u.x - v.x, u.y - v.y, u.z - v.z}; }

/**
 * @brief additive inverse of a vec3
 *
 * @param v
 * @return vec3
 */
vec3 addinv(const vec3 v) { return (vec3){-v.x, -v.y, -v.z}; }

/**
 * @brief scale a vec3 by a scalar
 *
 * @param v
 * @param s
 * @return vec3
 */
vec3 scale(const vec3 v, const float s) { return (vec3){v.x * s, v.y * s, v.z * s}; }

/**
 * @brief dot product of two vec3s
 *
 * @param u
 * @param v
 * @return float
 */
float dot(const vec3 u, const vec3 v) { return u.x * v.x + u.y * v.y + u.z * v.z; }

/**
 * @brief magnitude of a vec3
 *
 * @param v
 * @return float
 */
float norm(const vec3 v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

/**
 * @brief normalized vec3
 *
 * @param v
 * @return vec3
 */
vec3 normalized(const vec3 v) { return scale(v, 1.f / norm(v)); }

/**
 * @brief reflect a vector I about a normal N
 *
 * @param I
 * @param N
 * @return vec3
 */
vec3 reflect(const vec3 I, const vec3 N) { return subtract(I, scale(N, 2.f * dot(I, N))); }

/**
 * @brief refract a vector I about a normal N with indices of refraction eta_t and eta_i
 *
 * @param I
 * @param N
 * @param eta_t
 * @param eta_i
 * @return vec3
 */
vec3 refract(const vec3 *I, const vec3 *N, const float eta_t, const float eta_i) {
    const float cosi = -MAX(-1.f, MIN(1.f, dot(*I, *N)));

    if (cosi < 0) {
        const vec3 Ninv = addinv(*N);
        return refract(I, &Ninv, eta_i, eta_t);
    }
    const float eta = eta_i / eta_t;
    const float k   = 1.f - eta * eta * (1.f - cosi * cosi);

    return k < 0.f ? (vec3){1.f, 0.f, 0.f} : add(scale(*I, eta), scale(*N, eta * cosi - sqrt(k)));
}

/**
 * @brief intersect a ray with a sphere
 *
 * @param orig
 * @param dir
 * @param s
 * @param intersection
 * @param d
 */
void ray_sphere_intersect(const vec3 orig, const vec3 dir, const Sphere s, bool *const intersection, float *const d) {
    const vec3  L   = subtract(s.center, orig);  // vector from ray origin to sphere center
    const float tca = dot(L, dir);               // distance from ray origin to sphere center along ray direction
    const float d2  = dot(L, L) - tca * tca;

    if (d2 > s.radius * s.radius) {  // ray misses sphere
        *intersection = false;
        *d            = 0.f;
        return;
    }

    const float thc = sqrt(s.radius * s.radius - d2);  // distance from sphere center to ray intersection
    const float t0  = tca - thc;                       // distance from ray origin to first intersection
    const float t1  = tca + thc;                       // distance from ray origin to second intersection

    if (t0 > .001f) {  // ray hits sphere at first intersection
        *intersection = true;
        *d            = t0;
    } else if (t1 > .001f) {  // ray hits sphere at second intersection
        *intersection = true;
        *d            = t1;
    } else {  // ray misses sphere
        *intersection = false;
        *d            = 0;
    }
}

/**
 * @brief intersect a ray with a scene
 *
 * @param orig
 * @param dir
 * @param spheres
 * @param num_spheres
 * @param hit
 * @param point
 * @param N
 * @param material
 */
void scene_intersect(const vec3 orig, const vec3 dir, const Sphere *spheres, const int num_spheres, bool *hit, vec3 *point, vec3 *N, Material *material) {
    float    nearest_dist = 1e10;  // initialize to a large value
    vec3     pt, Normal;           // point and normal of intersection
    Material mat;                  // material of intersection
    init_Material(&mat);           // initialize material to default values

    if (fabs(dir.y) > .001f) {                                                                                 // check if ray is parallel to the floor
        float d = -(orig.y + 4.f) / dir.y;                                                                     // distance from ray origin to floor
        vec3  p = add(orig, scale(dir, d));                                                                    // point of intersection
        if (d > 0.001f && d < nearest_dist && fabs(p.x) < 10.f && p.z < -10.f && p.z > -30.f) {                // check if intersection is within the scene
            nearest_dist      = d;                                                                             // update nearest distance
            pt                = p;                                                                             // update point of intersection
            Normal            = (vec3){0.f, 1.f, 0.f};                                                         // normal at intersection
            mat.diffuse_color = ((int)(.5f * p.x + 1000.f) + (int)(.5f * p.z)) & 1 ? BOX_COLOR1 : BOX_COLOR2;  // set color
        }
    }

    float d;  // distance from ray origin to intersection
    for (int i = 0; i < num_spheres; i++) {
        const Sphere s = spheres[i];
        ray_sphere_intersect(orig, dir, s, hit, &d);             // check if ray intersects sphere
        if (*hit && d <= nearest_dist) {                         // check if intersection is closer than previous
            nearest_dist = d;                                    // update nearest distance
            pt           = add(orig, scale(dir, nearest_dist));  // update point of intersection
            Normal       = normalized(subtract(pt, s.center));   // normal at intersection
            mat          = s.material;
        }
    }

    *hit      = nearest_dist < 1000.f;  // check if ray intersects any object
    *point    = pt;                     // return point of intersection
    *N        = Normal;                 // return normal at intersection
    *material = mat;                    // return material of intersection
}

/**
 * @brief cast a ray into a scene
 *
 * @param orig
 * @param dir
 * @param depth
 * @param spheres
 * @param num_spheres
 * @param lights
 * @param num_lights
 * @return vec3
 */
vec3 cast_ray(const vec3 orig, const vec3 dir, const int depth, const Sphere *spheres, const int num_spheres, const vec3 *lights, const int num_lights) {
    bool     hit;
    vec3     pt, N;
    Material material;
    init_Material(&material);

    scene_intersect(orig, dir, spheres, num_spheres, &hit, &pt, &N, &material);  // check if ray intersects any object

    if (depth > DEPTH || !hit) return BACKGROUND_COLOR;  // return background color if ray misses scene

    const vec3 reflect_dir   = normalized(reflect(dir, N));                                                     // compute reflection direction
    const vec3 refract_dir   = normalized(refract(&dir, &N, material.refractive_index, 1.f));                   // compute refraction direction
    const vec3 reflect_color = cast_ray(pt, reflect_dir, depth + 1, spheres, num_spheres, lights, num_lights);  // compute reflection color
    const vec3 refract_color = cast_ray(pt, refract_dir, depth + 1, spheres, num_spheres, lights, num_lights);  // compute refraction color

    float diffuse_light_intensity  = 0.f;
    float specular_light_intensity = 0.f;

    for (int i = 0; i < num_lights; i++) {
        const vec3 light     = lights[i];
        const vec3 light_dir = normalized(subtract(light, pt));  // compute light direction

        bool     hit;
        vec3     shadow_pt;
        vec3     trashnrm;
        Material trashmat;
        init_Material(&trashmat);

        scene_intersect(pt, light_dir, spheres, num_spheres, &hit, &shadow_pt, &trashnrm, &trashmat);  // check if point is in shadow

        if (hit && norm(subtract(shadow_pt, pt)) < norm(subtract(light, pt))) continue;  // if point is in shadow, skip to next light

        diffuse_light_intensity += MAX(0.f, dot(light_dir, N));                                                           // add diffuse light intensity
        specular_light_intensity += pow(MAX(0.f, -dot(reflect(addinv(light_dir), N), dir)), material.specular_exponent);  // add specular light intensity
    }

    const vec3 diffuse_color    = scale(material.diffuse_color, diffuse_light_intensity * material.albedo[0]);  // compute diffuse color
    const vec3 specular_color   = scale((vec3){1.f, 1.f, 1.f}, specular_light_intensity * material.albedo[1]);  // compute specular color
    const vec3 reflection_color = scale(reflect_color, material.albedo[2]);                                     // compute reflection color
    const vec3 refraction_color = scale(refract_color, material.albedo[3]);                                     // compute refraction color

    vec3 color;  // color of pixel
    color.x = diffuse_color.x + specular_color.x + reflection_color.x + refraction_color.x;
    color.y = diffuse_color.y + specular_color.y + reflection_color.y + refraction_color.y;
    color.z = diffuse_color.z + specular_color.z + reflection_color.z + refraction_color.z;

    return color;
}

int main(void) {
    const int num_spheres = 4;  // number of spheres in scene

    Sphere *const spheres = (Sphere *const)malloc(sizeof(Sphere) * num_spheres);  // allocate memory for spheres

    // initialize spheres
    spheres[0] = (Sphere){{-3.f, 0.f, -16.f}, 2.f, ivory};
    spheres[1] = (Sphere){{-1.f, -1.5f, -12.f}, 2.f, glass};
    spheres[2] = (Sphere){{1.5f, -0.5f, -18.f}, 3.f, rubber};
    spheres[3] = (Sphere){{7.f, 5.f, -18.f}, 4.f, mirror};

    const int num_lights = 3;  // number of lights in scene

    vec3 *const lights = (vec3 *const)malloc(sizeof(vec3) * num_lights);  // allocate memory for lights

    // initialize lights
    lights[0] = (vec3){-20.f, 20.f, 20.f};
    lights[1] = (vec3){30.f, 50.f, -25.f};
    lights[2] = (vec3){30.f, 20.f, 30.f};

    vec3 *framebuffer = malloc(sizeof(vec3) * width * height);  // allocate memory for framebuffer

#pragma omp parallel for                              // parallelize loop
    for (int pix = 0; pix < width * height; pix++) {  // loop through pixels
        // compute ray direction
        const float dir_x = (pix % width + 0.5f) - width / 2.f;
        const float dir_y = -(pix / width + 0.5f) + height / 2.f;
        const float dir_z = -height / (2.f * tan(fov / 2.f));

        const vec3 orig = (vec3){0.f, 0.f, 0.f};                    // ray origin
        const vec3 dir  = normalized((vec3){dir_x, dir_y, dir_z});  // ray direction

        framebuffer[pix] = cast_ray(orig, dir, 0, spheres, num_spheres, lights, num_lights);  // cast ray
    }

    FILE *f = fopen("out.ppm", "wb");               // open file for writing
    fprintf(f, "P6\n%d %d\n225\n", width, height);  // write header

    for (int pix = 0; pix < width * height; pix++) {                                                   // loop through pixels
        vec3        color = framebuffer[pix];                                                          // get pixel color
        const float max_  = MAX(1.f, MAX(color.x, MAX(color.y, color.z)));                             // get max color component
        color             = scale(color, 225.f / max_);                                                // scale color to 0-225 range
        fprintf(f, "%c%c%c", (unsigned char)color.x, (unsigned char)color.y, (unsigned char)color.z);  // write color to file
    }

    fclose(f);  // close file

    // free memory
    free(spheres);
    free(lights);
    free(framebuffer);

    return 0;
}