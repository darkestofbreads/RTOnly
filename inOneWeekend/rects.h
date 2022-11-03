#ifndef XY_RECT_H
#define XY_RECT_H

#include "hittable.h"

class xy_rect : public hittable{
public:
	xy_rect() {}
	xy_rect(float _x0, float _x1, float _y0, float _y1, float _k, shared_ptr<material> _mat)
		: x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mat(_mat) {};

	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;

public:
	shared_ptr<material> mat;
	float x0, x1, y0, y1, k;
	vec3 outward_normal = vec3(0, 0, 1);
};

class xz_rect : public hittable {
public:
    xz_rect() {}

    xz_rect(float _x0, float _x1, float _z0, float _z1, float _k,
        shared_ptr<material> mat)
        : x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

    virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;

public:
    shared_ptr<material> mp;
    float x0, x1, z0, z1, k;
    vec3 outward_normal = vec3(0, 1, 0);
};

class yz_rect : public hittable {
public:
    yz_rect() {}

    yz_rect(float _y0, float _y1, float _z0, float _z1, double _k,
        shared_ptr<material> mat)
        : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

    virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;

public:
    shared_ptr<material> mp;
    float y0, y1, z0, z1, k;
    vec3 outward_normal = vec3(1, 0, 0);
};

bool xy_rect::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	auto t = (k - r.origin().z()) / r.direction().z();
	if (t < t_min || t > t_max) return false;

	auto x = r.origin().x() + t * r.direction().x();
	auto y = r.origin().y() + t * r.direction().y();
	if (x < x0 || x > x1 || y < y0 || y > y1) return false;

	rec.u = (x - x0) / (x1 - x0);
	rec.v = (y - y0) / (y1 - y0);
	rec.t = t;
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat;
	rec.p = r.at(t);
	return true;
}

bool xz_rect::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    auto t = (k - r.origin().y()) / r.direction().y();
    if (t < t_min || t > t_max)
        return false;
    auto x = r.origin().x() + t * r.direction().x();
    auto z = r.origin().z() + t * r.direction().z();
    if (x < x0 || x > x1 || z < z0 || z > z1)
        return false;
    rec.u = (x - x0) / (x1 - x0);
    rec.v = (z - z0) / (z1 - z0);
    rec.t = t;
    auto outward_normal = vec3(0, 1, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

bool yz_rect::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    auto t = (k - r.origin().x()) / r.direction().x();
    if (t < t_min || t > t_max)
        return false;
    auto y = r.origin().y() + t * r.direction().y();
    auto z = r.origin().z() + t * r.direction().z();
    if (y < y0 || y > y1 || z < z0 || z > z1)
        return false;
    rec.u = (y - y0) / (y1 - y0);
    rec.v = (z - z0) / (z1 - z0);
    rec.t = t;
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

#endif // !XY_RECT_H
