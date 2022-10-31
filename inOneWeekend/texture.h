#ifndef TEXTURE_H
#define TEXTURE_H

#include <vector>

#include "color.h"
#include "common.h"

class texture {
public:
	virtual color value(float u, float v, const point3& p) const = 0;
};

class texture_ppm {
public:
	texture_ppm() {}
	texture_ppm(std::vector<color> color_values, int width) : values(color_values), wdth(width) {}

public:
	std::vector<color> values;
	int wdth;
};

class solid_color :public texture {
public:
	solid_color(){}
	solid_color(color c) : color_value(c){}

	solid_color(float red, float green, float blue) : solid_color(color(red, green, blue)) {}

	virtual color value(float u, float v, const vec3& p) const override {
		return color_value;
	}
private:
	color color_value;
};

class checker_texture : public texture {
public:
	checker_texture(){}

	checker_texture(shared_ptr<texture> _even, shared_ptr<texture> _odd) : even(_even), odd(_odd) {}
	checker_texture(color c1, color c2) : even(make_shared<solid_color>(c1)), odd(make_shared<solid_color>(c2)) {}

	virtual color value(float u, float v, const point3& p) const override {
		auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
		if (sines < 0)
			return odd->value(u, v, p);
		else
			return even->value(u, v, p);
	}

public:
	shared_ptr<texture> odd;
	shared_ptr<texture> even;
};

inline color get_value_at(const texture_ppm& texture, int x, int y) {
	return texture.values[(y - 1) * texture.wdth + x - 1];
}

#endif // !TEXTURE_H
