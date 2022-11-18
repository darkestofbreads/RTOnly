#ifndef QUAD_H
#define QUAD_H

//not in the original book but on the github page on another branch
//https://github.com/RayTracing/raytracing.github.io/blob/ed119afc11dab9bd912d0c2872fbd780c0909f8c/src/TheNextWeek/quad.h

#include "common.h"
#include "hittable.h"
#include "hittable_list.h"

class quad : public hittable {
public:
	quad(const point3& _q, const vec3& _u, const vec3& _v, shared_ptr<material> m) : q(_q), u(_u), v(_v), mat(m) {
		auto n = cross(u, v);
		normal = unit_vector(n);
		d = -dot(normal, q);
		w = n / dot(n, n);
	}

	virtual bool hit(
		const ray& r, float t_min, float t_max, hit_record& rec) const override;

public:
	point3 q;
	vec3 u, v;
	shared_ptr<material> mat;
	vec3 normal;
	double d;
	vec3 w;
};

bool quad::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	auto denom = dot(normal, r.direction());

	if (abs(denom) < 1e-8) return false; //ray is parallel to quad

	auto t = (-d - dot(normal, r.origin())) / denom;
	if (t < t_min || t > t_max) return false;

	auto at = r.at(t);
	vec3 planar_hitpt = at - q;
	auto a = dot(w, cross(planar_hitpt, v));
	auto b = dot(w, cross(u, planar_hitpt));

	if (a < 0 || 1 < a || b < 0 || 1 < b) return false;

	rec.u = a;
	rec.v = b;
	rec.t = t;
	rec.p = at;
	rec.mat_ptr = mat;
	rec.set_face_normal(r, normal);
	return true;
}

class box : public hittable {
public:
	box() {}
	box(const point3& a, const point3& b, shared_ptr<material> mat);
	box(const point3& origin, const vec3& a, const vec3& b, const vec3& height, shared_ptr<material> mat);

	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const override;

public:
	point3 box_min;
	point3 box_max;
	hittable_list sides;
};

box::box(const point3& a, const point3& b, shared_ptr<material> mat)
{
	// Returns the 3D box (six sides) that contains the two opposite vertices a & b.

	// Construct the two opposite vertices with the minimum and maximum coordinates.
	auto min = point3(fmin(a.x(), b.x()), fmin(a.y(), b.y()), fmin(a.z(), b.z()));
	auto max = point3(fmax(a.x(), b.x()), fmax(a.y(), b.y()), fmax(a.z(), b.z()));

	auto dx = vec3(max.x() - min.x(), 0, 0);
	auto dy = vec3(0, max.y() - min.y(), 0);
	auto dz = vec3(0, 0, max.z() - min.z());
																							  
	sides.add(make_shared<quad>(point3(min.x(), min.y(), max.z()), dx, dy, mat));  // front	  
	sides.add(make_shared<quad>(point3(max.x(), min.y(), max.z()), -dz, dy, mat)); // right	  
	sides.add(make_shared<quad>(point3(max.x(), min.y(), min.z()), -dx, dy, mat)); // back	  
	sides.add(make_shared<quad>(point3(min.x(), min.y(), min.z()), dz, dy, mat));  // left	  
	sides.add(make_shared<quad>(point3(min.x(), max.y(), max.z()), dx, -dz, mat)); // top	  
	sides.add(make_shared<quad>(point3(min.x(), min.y(), min.z()), dx, dz, mat));  // bottom  
}

box::box(const point3& origin, const vec3& a, const vec3& b, const vec3& height, shared_ptr<material> mat)
{
	sides.add(make_shared<quad>(origin, a, height, mat));  // front	  
	sides.add(make_shared<quad>(origin+b, a, height, mat)); // back	 

	sides.add(make_shared<quad>(origin, b, height, mat)); // left  
	sides.add(make_shared<quad>(origin+a, b, height, mat));  // right

	sides.add(make_shared<quad>(origin+height, a, b, mat)); // top	  
	sides.add(make_shared<quad>(origin, a, b, mat));  // bottom  
}

bool box::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
	return sides.hit(r, t_min, t_max, rec);
}


#endif // !QUAD_H
