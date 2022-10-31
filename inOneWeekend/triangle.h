#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "vec3.h"

class triangle : public hittable {
public:
	triangle() {}
	triangle(point3 a, point3 b, point3 c, shared_ptr<material> m) : vertex0(a), vertex1(b), vertex2(c), mat_ptr(m) {};

	virtual bool hit(
			const ray& r, float t_min, float t_max, hit_record& rec) const override;

public:
	point3 vertex0;
	point3 vertex1;
	point3 vertex2;
	vec3 edge0 = vertex1 - vertex0;
	vec3 edge1 = vertex2 - vertex0;
	shared_ptr<material> mat_ptr;

private:
	static void get_triangle_uv(point3& p, point3 a, vec3 edge0, vec3 edge1, float& u, float& v) {
		vec3 v2(p - a);
		auto d = 1 / (edge0.x() * edge1.y() - edge1.x() * edge0.y());
		auto w = (edge0.x() * v2.y() - v2.x() * edge0.y()) * d;
		v = (v2.x() * edge1.y() - edge1.x() * v2.y()) * d;
		u = 1.0f - v - w;
	}
};

//implementation of Möller-Trumbore intersection algorithm
bool triangle::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	auto h = cross(r.direction(), edge1);
	auto a = dot(edge0, h);
	
	if (a > -t_min && a < t_min)
		return false; //Ray is parallel to triangle

	vec3 outward_normal = cross(edge0, edge1);
	if (dot(r.dir, outward_normal) > 0) 
		return false; //Ray intersects with backside of triangle

	auto f = 1.0 / a;
	auto s = r.origin() - vertex0;
	auto u = f * dot(s, h);
	if (u < 0.0 || u > 1.0)
		return false;

	auto q = cross(s, edge0);
	auto v = f * dot(r.direction(), q);
	if (v < 0.0 || u + v > 1.0)
		return false;

	auto t = f * dot(edge1, q);
	if (t < t_min || t_max < t)
		return false; //Another primitive is in front of the triangle

	rec.t = t;
	rec.p = r.at(t);
	rec.set_face_normal(r, outward_normal);
	get_triangle_uv(outward_normal, vertex0, edge0, edge1, rec.u, rec.v);
	rec.mat_ptr = mat_ptr;

	return true;
}

#endif // !TRIANGLE_H
