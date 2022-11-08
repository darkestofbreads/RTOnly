#include <iostream>
#include <future>
#include <string>

#include "color.h"
#include "common.h"
#include "hittable_list.h"
#include "sphere.h"
#include "triangle.h"
#include "camera.h"
#include "material.h"
#include "rects.h"
#include "box.h"

//TODO: 
//		- bounding boxes would be neat as im looking to add actual 3D-models and not just primitives relatively soon (scratchapixel has the code for a functional raytracer with a mesh)
//			- this will probably come in late
// 
//		- light sources:
//			- point light sources which should be pretty straight forward
//			- directional light sources (here comes the sun)
// 
//		- normal maps, roughness and metallic maps and so on (a good opportunity to assure blender "compatibility")
// 
//		- as stated in the book: volumetrics (fog), which is apparently covered in "raytracing the next week"
//			-it seems like thats also a material for already existing objects, possibly compare with how other game engines do it
//
//		- clean up headers if possible

// textures will only fully work on spheres, they wont work at all on boxes and on triangles only when
// boxes arent present (tldr: boxes make kaput pictures)

color rayColor(const ray& r, const color& background, const hittable& world, int depth);

//yes i'm aware of how awful this looks atm
void startRender(const int imageWidth, float aspectRatio, const color& background, const int samples, const int bounces, hittable_list& world, const int processorCount, camera& cam);
std::vector<int> render(int imageWidth, int imageHeight_steps, int imageHeight, int j, const color& background, int samples, int bounces, const hittable_list& world, const camera& cam);

int WinMain() {
	
	// Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 1920;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	
	// Image quality
	const int samples_per_pixel = 500;
	const int bounces = 20;
	const bool doDepthOfField = true;
	const color background(0.01, 0.01, 0.01);

	// Get CPU specs
	const auto processor_count = std::thread::hardware_concurrency();

	// Camera
	point3 lookfrom(0, 2, 4);
	point3 lookat(3, 0, -1);
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
		cam.setDoF(defocusDist, lensRadius/defocusDist);
	}

	auto material_ground = make_shared<metal>(color(0.4, 0.9, 0.0), 0);
	auto material_center = make_shared<lambertian>(color(0.7, 0.3, 0.3));
	auto material_right0 = make_shared<metal>(color(1, 0, 0.5), 0.1);
	auto material_right1 = make_shared<metal>(color(1, 0, 0.5), 0);
	auto material_left = make_shared<metal>(color(0.8, 0.6, 0.2), 0.05);
	auto dielectric0 = make_shared<dielectric>(1.5);
	auto dielectric1 = make_shared<dielectric>(3);
	auto whitelight = make_shared<diffuse_light>(color(0,4,0.7));
	auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

	auto some_texture = make_shared<image_texture>("earthmap.jpg");
	auto crappy_bricks = make_shared<image_texture>("crappy_bricks.png");

	auto crappy_light = make_shared<diffuse_light>(crappy_bricks);

	auto some_surface = make_shared<lambertian>(some_texture);
	auto some_surface_light = make_shared<diffuse_light>(some_texture);

	//CAUTION: Adding hittables currently increases render time by a lot! (remove when bounding boxes are implemented)

	//		spheres:					(x, y, z), radius, material
	//		triangles:			 point3 a, point3 b, point3 c, material
	world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.6, material_center));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.4, material_left));
	world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_center));
	world.add(make_shared<sphere>(point3(-2, 0, -2), 0.5, dielectric0));
	world.add(make_shared<sphere>(point3(-1, 0, -1.5), -0.35, dielectric1));
	world.add(make_shared<sphere>(point3(-1, 0, -1.5), 0.3, dielectric1));
	world.add(make_shared<sphere>(point3(-2, 3, -5), 0.4, whitelight));
	world.add(make_shared<sphere>(point3(5, 2, -2), 0.6, crappy_light));
	world.add(make_shared<sphere>(point3(0, -100.5, -1), 100, make_shared<lambertian>(checker)));
	world.add(make_shared<triangle>(point3(1, 0, -4), point3(2, 0, -6), point3(3, 3, -7), material_center));
	world.add(make_shared<triangle>(point3(2, -2, -4), point3(3, -1, 0), point3(2.5, 3, -3), some_surface));
	world.add(make_shared<triangle>(point3(-3, -1, 0), point3(2, -2, -4), point3(2.5, 3, -3), some_surface_light));

	startRender(image_width, aspect_ratio, background, samples_per_pixel, bounces, world, processor_count, cam);

	return 0;
}

//i should probably start understanding this function before continuing with adding rt related features
color rayColor(const ray& r, const color& background, const hittable& world, int bounces) {
	hit_record rec;

	if (bounces <= 0)
		return color(0, 0, 0);

	if (!world.hit(r, 0.001, infinity, rec))
		return background;

	ray scattered;
	color attenuation;
	color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

	if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		return emitted;

	//this is our background
	return emitted + attenuation * rayColor(scattered, background, world, bounces - 1);
}

//this part below is actually fully written by me and not entirely copy-pasted
void startRender(const int imageWidth, float aspectRatio, const color& background, const int samples, const int bounces, hittable_list& world, const int processorCount, camera& cam) {
	std::vector<std::future<std::vector<int>>> ftr;
	std::vector<std::vector<int>> imageData;

	auto imageHeight = static_cast<int>(imageWidth / aspectRatio);
	auto heightPixelSteps = static_cast<int>(imageHeight / processorCount);

	//i should probably change this or the vectors to be more consistent with each other
	//actually i should probably bother with it when i get to memory management
	uint8_t* image = new uint8_t[imageWidth * imageHeight * 3];

	//to pass something by reference here, std::ref or std::cref (for constant stuff, hence the c) needs to be used
	for (int pos = processorCount-1; pos >= 0; pos--)
		//ridiculous amount of parameters but oh well
		ftr.push_back(std::async(render, imageWidth, heightPixelSteps, imageHeight, pos + 1, std::cref(background), samples, bounces, std::cref(world), std::cref(cam)));

	//future of vector of int to vector of int
	for (auto& oc : ftr)
		imageData.push_back(oc.get());

	//send pixeldata to output stream in order
	int index = 0;
	for (int j = 0; j < imageData.size(); j++)
		for(int i = 0; i < imageData[j].size(); i++)
			//this warning is a bit annoying
			image[index++] = imageData[j][i];

	//create .png file									 3 Channels: R, G and B, a fourth one would add the Alpha channel which is useless here
	stbi_write_png("image.png", imageWidth, imageHeight, 3, image, imageWidth*3);
}

std::vector<int> render(int imageWidth, int imageHeight_steps, int imageHeight, int j, const color& background, int samples, int bounces, const hittable_list& world, const camera& cam) {
	std::vector<int> image;
	auto scale = 1.0 / samples;

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
				pixelColor += rayColor(r, background, world, bounces);
			}

			//allocate pixels to output string
			write_color_to_int(pixelColor, scale, image);
		}
	}
	return image;
}