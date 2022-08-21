#include <iostream>
#include <fstream>
#include "vec3.h"
#include "color.h"

//This is mostly NOT my code, this is from the book "Raytracing in one weekend" by Peter Shirley

//TODO: - create github repository and push code
//		- error stream to logfile       > Low priority

int WinMain() {

	//Image dimensions
	const int image_width = 256;
	const int image_height = 256;

	//Create imagefile object and open file
	std::ofstream imageFile;
	imageFile.open("image.ppm");

	//Output stream to object file
	std::cout.rdbuf(imageFile.rdbuf());


	//Render:

	//Head for .ppm file
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	for (int j = image_height - 1; j >= 0; --j) {

		//Broadcast image progress
		std::cerr << "\rScanlines Remaining: " << j << ' ' << std::flush;
		
		//Render image
		for (int i = 0; i < image_width; ++i) {
			color pixelColor(
				float(i) / (image_width - 1),
				float(j) / (image_height - 1),
				0.25f	);

			//stream pixels to output stream
			write_color(pixelColor);

		}
	}

	//Broadcast that image is finished
	std::cerr << "\nImage is done Rendering.\n";

	//Close image file object
	imageFile.close();

	return 0;
}