#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable {
public:
	sphere() {}
	sphere(point3 cen, float r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {};

	virtual bool hit(
		const ray& r, float t_min, float t_max, hit_record& rec) const override;

public:
	point3 center;
	float radius;
    shared_ptr<material> mat_ptr;

private:
    static void get_sphere_uv(const point3& p, float& u, float& v) {
        auto theta = acos(-p.y());
        auto phi = atan2(-p.z(), p.x()) + pi;
        u = phi / (2 * pi);
        v = theta / pi;
    }
};


bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;

    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return false;
    auto sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = (rec.p - center) / radius;
    rec.normal = outward_normal;
    rec.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, rec.u, rec.v);
    rec.mat_ptr = mat_ptr;

    return true;
}

#endif // !SPHERE_H
