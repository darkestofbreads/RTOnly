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

	virtual bool emitted(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, color& emit) const {
		return false;
	}
};

class diffuse_light : public material {
public:
	diffuse_light(shared_ptr<texture> a) : albedo(a), emit_map(make_shared<solid_color>(1)) {}
	diffuse_light(const color& c) : albedo(make_shared<solid_color>(c)), emit_map(make_shared<solid_color>(1)) {}
	diffuse_light(shared_ptr<texture> a, shared_ptr<texture> f) : albedo(a), emit_map(f) {}
	diffuse_light(const color& a, shared_ptr<texture> f) : albedo(make_shared<solid_color>(a)), emit_map(f) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const override {
		auto scatter_direction = random_in_hemisphere(rec.normal);

		if (scatter_direction.near_zero())
			scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo->value(rec.u, rec.v, rec.p);
		return true;
	}

	virtual bool emitted(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered, color& emit) const override {
		if (emit_map->value(rec.u, rec.v, rec.normal).x() == 1) {
			emit = albedo->value(rec.u, rec.v, rec.p);
			return true;
		}
		emit = color(0, 0, 0);
		return false;
	}
public:
	shared_ptr<texture> albedo;
	shared_ptr<texture> emit_map;
};

class lambertian : public material {
public:
	lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
	lambertian(shared_ptr<texture> a) : albedo(a) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const override {
		auto scatter_direction = random_in_hemisphere(rec.normal);

		if (scatter_direction.near_zero())
			scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo->value(rec.u, rec.v, rec.p);
		return true;
	}

public:
	shared_ptr<texture> albedo;
};

class metal : public material {
public:
	metal(const color& a, float f) : albedo(make_shared<solid_color>(a)), fuzz(make_shared<solid_color>(f < 1 ? f : 1)) {}
	metal(shared_ptr<texture> a, float f) : albedo(a), fuzz(make_shared<solid_color>(f < 1 ? f : 1)) {}
	metal(shared_ptr<texture> a, shared_ptr<texture> f) : albedo(a), fuzz(f) {}
	metal(const color& a, shared_ptr<texture> f) : albedo(make_shared<solid_color>(a)), fuzz(f) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const override {
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz->value(rec.u, rec.v, rec.normal).x() * random_in_unit_sphere());
		attenuation = albedo->value(rec.u, rec.v, rec.p);
		return (dot(scattered.direction(), rec.normal) > 0);
	}

public:
	shared_ptr<texture> fuzz;
	shared_ptr<texture> albedo;
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
