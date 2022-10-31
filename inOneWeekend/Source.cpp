#include <iostream>
#include <fstream>
#include <future>
#include <string>

#include "color.h"
#include "common.h"
#include "hittable_list.h"
#include "sphere.h"
#include "triangle.h"
#include "camera.h"
#include "material.h"

//TODO: 
//		- bounding boxes would be neat as im looking to add actual 3D-models and not just primitives relatively soon (iirc thats done in the next book "raytracing the next week")
// 
//		- light sources:
//			- point light sources which should be pretty straight forward -> send ray to light source and check if it hits anything else in the way
//			- directional light sources (here comes the sun): 
//				1. cast ray directly into the center of the sun (which is always facing the same direction regardless of origin)
//				2. if the ray doesnt hit the sun yet (because it hits something else), cast another ray in some kind of cone facing the sun (meaning i'm going to need a random_in_unit_cone vec3 function)
//			- emissive objects shouldnt be too hard to do too, right?
// 
//		- textures, normal maps, roughness and metallic maps and so on
// 
//		- as stated in the book: volumetrics (fog), how do i do that?? (this should also automatically include stuff like light shafts? i think?)
//			- make the ray hit a random point in the volumetric and then spread secondary rays in random directions? this will fuck over performance by a good chunk
//
//		- would be good to have this open source too

//there currently is a problem with triangles where they reflect when they shouldnt

color rayColor(const ray& r, const hittable& world, int depth);

//yes i'm aware of how awful this looks atm
void startRender(int imageWidth, float aspectRatio, int samples, int bounces, hittable_list& world, int processorCount, camera& cam);
std::string render(int imageWidth, int imageHeight_steps, int imageHeight, int j, int samples, int bounces, const hittable_list& world, const camera& cam);

int WinMain() {
	
	// Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 1920;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	
	// Image quality
	const int samples_per_pixel = 50;
	const int bounces = 20;
	const bool doDepthOfField = false;

	// Get CPU specs
	const auto processor_count = std::thread::hardware_concurrency();

	// Camera
	point3 lookfrom(0, 0, 3);
	point3 lookat(0, 0, -1);
	vec3 vup(0, 1, 0);

	auto dist_to_focus = (lookfrom - lookat).length();
	auto lensRadius = 0.1;

	camera cam(lookfrom, lookat, vup, 90, aspect_ratio);

	// world
	hittable_list world;

	// Depth of field: get camera ray at the center of the image, if it hits anything compute the distance+1 and set DoF
	hit_record rec;
	ray r = cam.get_ray(image_width / 2, image_height / 2);
	if (!world.hit(r, 0.001, infinity, rec) && doDepthOfField) {
		float defocusDist = vec3(lookfrom - rec.p).length() + 1;
		cam.setDoF(defocusDist, lensRadius);
	}

	auto material_ground = make_shared<metal>(color(0.4, 0.9, 0.0), 0);
	auto material_center = make_shared<lambertian>(color(0.7, 0.3, 0.3));
	auto material_right0 = make_shared<metal>(color(1, 0, 0.5), 0.1);
	auto material_right1 = make_shared<metal>(color(1, 0, 0.5), 0);
	auto material_left = make_shared<metal>(color(0.8, 0.6, 0.2), 0.05);
	auto dielectric0 = make_shared<dielectric>(1.5);
	auto dielectric1 = make_shared<dielectric>(3);
	auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

	//CAUTION: Adding hittables currently increases render time by a lot! (remove when bounding boxes are implemented)

	//		spheres:					(x, y, z), radius, material
	//		triangles:			 point3 a, point3 b, point3 c, material
	world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.7, material_center));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.4, material_left));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.4, material_left));
	world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right1));
	world.add(make_shared<sphere>(point3(6, 3, -4), 1, material_left));
	world.add(make_shared<sphere>(point3(-2, 0, -2), 0.5, dielectric0));
	world.add(make_shared<sphere>(point3(-1, 0, -1.5), -0.35, dielectric1));
	world.add(make_shared<sphere>(point3(-1, 0, -1.5), 0.3, dielectric1));
	world.add(make_shared<sphere>(point3(0, -100.5, -1), 100, make_shared<lambertian>(checker)));
	world.add(make_shared<triangle>(point3(1, 0, -4), point3(2, 0, -6), point3(3, 3, -7), material_center));
	world.add(make_shared<triangle>(point3(2, -2, -4), point3(3, -1, 0), point3(2.5, 3, -3), make_shared<lambertian>(checker)));
	//world.add(make_shared<triangle>(point3(-3, -1, 0), point3(2, -2, -4), point3(2.5, 3, -3), material_center));

	//Create imagefile object and open file (.ppm files can be opened using Gimp)
	std::ofstream imageFile;
	imageFile.open("image.ppm");
	//imageFile.open("image.txt");

	//Output stream to object file
	std::cout.rdbuf(imageFile.rdbuf());


	//Render:

	//Head for .ppm file
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	startRender(image_width, aspect_ratio, samples_per_pixel, bounces, world, processor_count, cam);

	//Broadcast that image is finished
	std::cerr << "\nImage is done Rendering.\n";

	//Close image file object
	imageFile.close();

	return 0;
}

//i should probably start understanding this function before continuing with adding rt related features
color rayColor(const ray& r, const hittable& world, int bounces) {
	hit_record rec;

	if (bounces <= 0)
		return color(0, 0, 0);

	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * rayColor(scattered, world, bounces - 1);
		return color(0, 0, 0);
	}

	//i think this it the background? if it is then change 
	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5f * (unit_direction.y() + 1.0f);
	return (1.0f - t) * color(1.0f, 1.0f, 1.0f) + t * color(0.5f, 0.7f, 1.0f);
}

//this part below is actually fully written by me and not entirely copy-pasted
void startRender(int imageWidth, float aspectRatio, int samples, int bounces, hittable_list& world, int processorCount, camera& cam) {
	std::vector<std::future<std::string>> ftr;

	auto imageHeight = static_cast<int>(imageWidth / aspectRatio);
	auto heightPixelSteps = static_cast<int>(imageHeight / processorCount);

	//to pass something by reference here, std::ref or std::cref (for constant stuff, hence the c) needs to be used
	for (int pos = processorCount-1; pos >= 0; pos--)
		//ridiculous amount of parameters but oh well
		ftr.push_back(std::async(render, imageWidth, heightPixelSteps, imageHeight, pos + 1, samples, bounces, std::cref(world), std::cref(cam)));

	//send pixeldata to output stream in order
	for (auto& oc : ftr)
		std::cout << oc.get();
}

std::string render(int imageWidth, int imageHeight_steps, int imageHeight, int j, int samples, int bounces, const hittable_list& world, const camera& cam) {
	std::string image;
	int new_height = imageHeight_steps * j;
	int prev_height = imageHeight_steps * (j - 1);
	//do the render magic
	for (int a = new_height - 1; a >= prev_height; --a) {
		for (int b = 0; b < imageWidth; ++b) {
			color pixelColor(0, 0, 0);
			for (int s = 0; s < samples; ++s) {
				auto u = (b + random_float()) / (imageWidth - 1);
				auto v = (a + random_float()) / (imageHeight - 1);
				ray r = cam.get_ray(u, v);
				pixelColor += rayColor(r, world, bounces);
			}

			//allocate pixels to output string
			image += write_color_to_string(pixelColor, samples);
		}
	}
	return image;
}