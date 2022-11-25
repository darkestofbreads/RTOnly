#ifndef COMMON_H
#define COMMON_H

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>

// https://github.com/nothings/stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// constants
const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385f;

// utility functions
inline float degreesToRadians(float degrees) {
	return degrees * (pi / 180);
}

inline float random_float() {
	//returns a random float from 0 to <1
	return rand() / (RAND_MAX + 1.0f);
}
inline float random_float(float min, float max) {
	return min + (max - min) * random_float();
}

inline int random_int(int min, int max) {
	return static_cast<int>(random_float(min, max + 1));
}

#endif // !COMMON_H
