#ifndef RAY_H
#define RAY_H

//RAAAAAAYS FOR RAAAAAAYTRACING WHO WOULD HAVE THOUGHT OF THAT?!
//cones for raytracing tho, thatd be... something

#include "vec3.h"

class ray {
public:
	ray() {}
	ray(const point3& origin, const vec3& direction)
		: orig(origin), dir(direction)
	{}

	point3 origin() const { return orig; }
	vec3 direction() const { return dir; }

	point3 at(float slope) const {
		return dir * slope + orig;
	}


public: //not private for some reason, might find out why later | UPDATE: still havent found out
	point3 orig;
	vec3 dir;
};

#endif // !RAY_H
