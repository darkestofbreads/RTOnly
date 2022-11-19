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
#include "quad.h"

//TODO: 
//		- bounding boxes would be neat as im looking to add actual 3D-models and not just primitives relatively soon (scratchapixel has the code for a functional raytracer with a mesh)
//			- this will probably come in late
// 
//		- light sources:
//			- point light sources which should be pretty straight forward
//			- directional light sources (here comes the sun) with radius
//				- circles shouldnt be too hard to trace, just gotta make them.. global?
//					- an idea would be to have a hittable list in an unit sphere and just register objects there
// 
//		- normal maps, roughness and metallic maps and so on (a good opportunity to assure blender "compatibility")
//			- probably just make some procedual material and export its maps for testing
// 
//		- as stated in the book: volumetrics (fog), which is apparently covered in "raytracing the next week"

color rayColor(const ray& r, const color& background, const hittable& world, int depth);

void startRender(const int imageWidth, float aspectRatio, const color& background, const int samples, const int bounces, hittable_list& world, const int processorCount, camera& cam);
std::vector<int> render(int imageWidth, int imageHeight_steps, int imageHeight, int j, const color& background, int samples, int bounces, const hittable_list& world, const camera& cam);

int WinMain() {
	
	// Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 1920;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	
	// Image quality
	const int samples_per_pixel = 50;
	const int bounces = 20;
	const bool doDepthOfField = true;
	const color background(0.01, 0.01, 0.01);

	// Get CPU specs
	const auto processor_count = std::thread::hardware_concurrency();

	// Camera
	//point3 lookfrom(0, 2, 4);
	//point3 lookat(3, 0, -1);
	const float fov = 50;
	vec3 vup(0, 1, 0);
	auto lookfrom = point3(278, 278, -800);
	auto lookat = point3(278, 278, 0);

	auto dist_to_focus = (lookfrom - lookat).length();
	auto lensRadius = 0.1;

	camera cam(lookfrom, lookat, vup, fov, aspect_ratio);

	// world
	hittable_list world;

	// materials
	auto material_ground = make_shared<metal>(color(0.4, 0.9, 0.0), 0);
	auto material_center = make_shared<lambertian>(color(0.7, 0.3, 0.3));
	auto material_right0 = make_shared<metal>(color(1, 0, 0.5), 0.1);
	auto material_right1 = make_shared<metal>(color(1, 0, 0.5), 0);
	auto material_left = make_shared<metal>(color(0.8, 0.6, 0.2), 0.05);
	auto dielectric0 = make_shared<dielectric>(1.5);
	auto dielectric1 = make_shared<dielectric>(3);
	auto whitelight = make_shared<diffuse_light>(color(0, 4, 0.7));
	auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto blue = make_shared<lambertian>(color(.12, .15, .55));
	auto light = make_shared<diffuse_light>(color(15, 15, 15));
	auto red_light = make_shared<diffuse_light>(color(.65, .05, .05));

	auto some_texture = make_shared<image_texture>("textures/earthmap.jpg");
	auto crappy_bricks = make_shared<image_texture>("textures/crappy_bricks.png");
	auto cyan = make_shared<image_texture>(" ");
	auto sand_texture = make_shared<image_texture>("textures/sand.png");

	auto crappy_roughness = make_shared<greyscale>("textures/crappy_roughness.png");
	auto redstone_emission = make_shared<greyscale>("textures/redstone_emission.png");

	auto cyan_light = make_shared<diffuse_light>(cyan);
	auto crappy_light = make_shared<diffuse_light>(crappy_bricks);
	auto some_surface = make_shared<lambertian>(some_texture);
	auto some_surface_light = make_shared<diffuse_light>(some_texture);
	auto sand = make_shared<lambertian>(sand_texture);
	auto redstone_lamp = make_shared<diffuse_light>(make_shared<image_texture>("textures/redstone_lamp.png"), redstone_emission);

		//CAUTION: Adding hittables currently increases render time by a lot! (remove when bounding boxes are implemented)
	//		boxes:				point3 start, point3 end, material
	//							point3 origin, vec3 a, vec3 b, vec3 height, material
	//		triangles:			point3 a, point3 b, point3 c, material
	//		quads:				point3 pos, vec3 v, vec3 u, material
	//		spheres:			point3 pos, radius, material

	world.add(make_shared<box>(point3(70, 165, 230), point3(230, 0, 65), redstone_lamp));
	world.add(make_shared<box>(point3(265, 0, 295), vec3(165, 0, -100), vec3(50, 40, 165), vec3(10, 330, 0), sand));

	//world.add(make_shared<triangle>(point3(2, -2, -4), point3(3, -1, 0), point3(2.5, 3, -3), some_surface));
	//world.add(make_shared<triangle>(point3(500, 1000, 500), point3(1000, 500, 500), point3(500, 500, 1000), crappy_light));
	//world.add(make_shared<triangle>(point3(-3, -1, 0), point3(2, -2, -4), point3(2.5, 3, -3), some_surface_light));
	//world.add(make_shared<triangle>(point3(1, 0, -4), point3(2, 0, -6), point3(3, 3, -7), material_center));

	world.add(make_shared<quad>(point3(0, 0, 555), vec3(0, 555, 0), vec3(555,0,0), cyan_light));
	world.add(make_shared<quad>(point3(0, 0, 555), vec3(0, 555, 0), vec3(0, 0, -555), make_shared<metal>(crappy_bricks, redstone_emission)));
	world.add(make_shared<quad>(point3(0, 0, 555), vec3(555, 0, 0), vec3(0, 0, -555), red));
	world.add(make_shared<quad>(point3(0, 555, 555), vec3(555, 0, 0), vec3(0, 0, -555), green));
	world.add(make_shared<quad>(point3(555, 0, 555), vec3(0, 555, 0), vec3(0, 0, -555), blue));

	//world.add(make_shared<sphere>(point3(275, 400, 250), 75, material_right0));
	
	// Depth of field: get camera ray at the center of the image, if it hits anything compute the distance and set DoF
	//world.hit returns false for some reason, something is off with the math below
	hit_record rec;
	auto s = image_width / 2;
	auto t = image_height / 2;
	ray r = cam.get_ray(s / (image_width - 1), t / (image_height - 1));
	if (world.hit(r, 0.001, infinity, rec) && doDepthOfField) {
		float defocusDist = vec3(lookfrom - rec.p).length();
		cam.setDoF(defocusDist, 3);
	}

	startRender(image_width, aspect_ratio, background, samples_per_pixel, bounces, world, processor_count, cam);

	return 0;
}

color rayColor(const ray& r, const color& background, const hittable& world, int bounces) {
	hit_record rec;

	if (bounces <= 0)
		//bounce limit has been exceeded
		return color(0, 0, 0);

	if (!world.hit(r, 0.001, infinity, rec))
		return background;

	ray scattered;
	color attenuation;
	color emitted;

	if(rec.mat_ptr->emitted(r, rec, attenuation, scattered, emitted))
		//hit a light source
		return emitted;

	rec.mat_ptr->scatter(r, rec, attenuation, scattered);

	//hit a non light source object
	return emitted + attenuation * rayColor(scattered, background, world, bounces - 1);
}

void startRender(const int imageWidth, float aspectRatio, const color& background, const int samples, const int bounces, hittable_list& world, const int processorCount, camera& cam) {
	std::vector<std::future<std::vector<int>>> ftr;
	std::vector<std::vector<int>> imageData;

	auto imageHeight = static_cast<int>(imageWidth / aspectRatio);
	auto heightPixelSteps = static_cast<int>(imageHeight / processorCount);

	//i should probably change this or the vectors to be more consistent with each other
	//actually i should probably bother with it when i get to memory management
	unsigned char* image = new unsigned char[imageWidth * imageHeight * 3];

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

			//allocate pixels to output int vector
			write_color_to_int(pixelColor, scale, image);
		}
	}
	return image;
}