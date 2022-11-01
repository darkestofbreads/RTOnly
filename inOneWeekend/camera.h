#ifndef CAMERA_H
#define CAMERA_H

#include "common.h"

class camera {
public:
    camera(
        point3 lookfrom,
        point3 lookat,
        vec3 vup,
        float vfov,
        float aspect_ratio
    ) {
        auto theta = degreesToRadians(vfov);
        auto h = tan(theta / 2);
        viewport_height = 2.0 * h;
        viewport_width = aspect_ratio * viewport_height;
        auto focal_length = 1.0;

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        origin = lookfrom;
        horizontal = viewport_width * u;
        vertical = viewport_height * v;
        lower_left_corner = origin - horizontal / 2 - vertical / 2 - w;
    }

    void setDoF(float focusDist, float lensRadius) {
        horizontal = focusDist * viewport_width * u;
        vertical = focusDist * viewport_height * v;
        lower_left_corner = origin - horizontal / 2 - vertical / 2 - focusDist * w;
        lens_radius = lensRadius;
    };

    ray get_ray(double s, double t) const {
        vec3 rd = lens_radius * random_in_unit_disk();
        vec3 offset = u * rd.x() + v * rd.y();

        return ray(
            origin + offset,
            lower_left_corner + s * horizontal + t * vertical - origin - offset
        );
    }

private:
    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    float lens_radius = 0;
    float viewport_height;
    float viewport_width;
};

inline float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

//spooky function name with even spookier code, happy halloween
void write_color_to_int(color pixel_color, float scale, std::vector<int> &out) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Divide the color by the number of samples.
    r = sqrt(scale * r);
    g = sqrt(scale * g);
    b = sqrt(scale * b);

    // Write the translated [0,255] value of each color component.
    out.push_back(static_cast<int>(256 * clamp(r, 0.0, 0.999)));
    out.push_back(static_cast<int>(256 * clamp(g, 0.0, 0.999)));
    out.push_back(static_cast<int>(256 * clamp(b, 0.0, 0.999)));
}

#endif // !CAMERA_H
