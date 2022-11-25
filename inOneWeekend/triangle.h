#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "common.h"
#include "hittable.h"
#include "vec3.h"

class triangle : public hittable {
public:
	triangle() {}
	triangle(point3 a, point3 b, point3 c, shared_ptr<material> m) : vertex0(a), vertex1(b), vertex2(c), mat_ptr(m) {};

	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;
	virtual bool bounding_box(float time0, float time1, aabb& output_box) const override;

public:
	point3 vertex0;
	point3 vertex1;
	point3 vertex2;
	vec3 edge0 = vertex1 - vertex0;
	vec3 edge1 = vertex2 - vertex0;
	shared_ptr<material> mat_ptr;
	vec3 outward_normal = cross(edge0, edge1);
};

//implementation of Möller-Trumbore intersection algorithm
bool triangle::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	auto h = cross(r.direction(), edge1);
	auto a = dot(edge0, h);

	if (a > -t_min && a < t_min)
		return false; //Ray is parallel to triangle

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

	auto at = r.at(t);

	rec.t = t;
	rec.p = at;
	rec.u = u;
	rec.v = v;
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat_ptr;

	return true;
}

bool triangle::bounding_box(float time0, float time1, aabb& output_box) const {
	float min_x = fmin(vertex0.x(), fmin(vertex1.x(), vertex2.x()));
	float min_y = fmin(vertex0.y(), fmin(vertex1.y(), vertex2.y()));
	float min_z = fmin(vertex0.z(), fmin(vertex1.z(), vertex2.z()));

	float max_x = fmax(vertex0.x(), fmax(vertex1.x(), vertex2.x()));
	float max_y = fmax(vertex0.y(), fmax(vertex1.y(), vertex2.y()));
	float max_z = fmax(vertex0.z(), fmax(vertex1.z(), vertex2.z()));

	output_box = aabb(point3(min_x, min_y, min_z), point3(max_x, max_y, max_z));

	return true;
}

#endif // !TRIANGLE_H
