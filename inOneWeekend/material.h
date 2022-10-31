#ifndef MATERIAL_H
#define MATERIAL_H

#include "common.h"
#include "ray.h"
#include "hittable.h"
#include "texture.h"

struct hit_record;

class material {
public:
	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const = 0;
};

class lambertian : public material {
public:
	lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
	lambertian(shared_ptr<texture> a) : albedo(a) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const override {
		auto scatter_direction = rec.normal + random_unit_vector();

		if (scatter_direction.near_zero())
			scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo->value(rec.u, rec.v, rec.p);
		return true;
	}

public:
	shared_ptr<texture> albedo;
};

//this should only work on triangles
class surface : public material {
public:
	surface(const texture_ppm& texture) : txtr(texture) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const override {
		auto scatter_direction = rec.normal + random_unit_vector();

		if (scatter_direction.near_zero())
			scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction);
		attenuation = get_value_at(txtr, rec.p.x(), rec.p.y());
		return true;
	}

public:
	texture_ppm txtr;
};

/*class barycentric_test : public material {
public:
	barycentric_test(point3 a, point3 b, point3 c) : vertex0(a), vertex1(b), vertex2(c) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const override {
		color colors[3] = { color(1,0,0), color(0,1,0), color(0,0,1) };
		auto scatter_direction = rec.normal + random_unit_vector();
		auto bary_coords = barycentric_coordinates(rec.p, vertex0, vertex1, vertex2);

		if (scatter_direction.near_zero())
			scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction);
		attenuation = bary_coords.x() * colors[0] + bary_coords.y() * colors[1] + (1 - bary_coords.x() - bary_coords.y()) * colors[2];
		return true;
	}
public:
	point3 vertex0;
	point3 vertex1;
	point3 vertex2;
};*/

class metal : public material {
public:
	metal(const color& a, float f) : albedo(a), fuzz(f < 1 ? f : 1) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const override {
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere() );
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}

public:
	color albedo;
	float fuzz;
};

//Tip for hollow glass spheres: in a sphere add another sphere with a negative radius
class dielectric : public material {
public:
	dielectric(float indexOfRefraction) : ir(indexOfRefraction) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const override {
		attenuation = color(1.0, 1.0, 1.0);
		double refractionRatio = rec.front_face ? (1.0 / ir) : ir;

		vec3 unitDirection = unit_vector(r_in.direction());
		float cosTheta = fmin(dot(-unitDirection, rec.normal), 1.0);
		float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

		bool cannotRefract = refractionRatio * sinTheta > 1.0;
		vec3 direction;

		if (cannotRefract || reflectance(cosTheta, refractionRatio) > random_float())
			direction = reflect(unitDirection, rec.normal);
		else
			direction = refract(unitDirection, rec.normal, refractionRatio);

		scattered = ray(rec.p, direction);
		return true;
	}

public:
	float ir;
private:
	static double reflectance(float cosine, float ref_idx) {
		//Schlicks approximation for reflectance
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};

#endif // !MATERIAL_H
