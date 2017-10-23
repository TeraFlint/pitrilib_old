#pragma optimize("gsy", on)

#include "pitri.h"
//using namespace pitri;

std::string Description()
{
	return "This is the test application for Pitri's C++ library.";
}

int main(int argc, char **argv)
{
	int size = UCHAR_MAX;

	Pitri::ParList par;
	par.SetExeTime(__DATE__, __TIME__);
	par.SetExeInfo("Pitri", "Library test", "1.0", 1);
	par.SetDescription(Description);
	par.AddParameter("s", false, 1, "Size of the image. Default is " + std::to_string(size) + ".");
	par.HandleParameters(argc, argv);

	std::cout << par.PrintHelp(par.GetParameter("?!").used);
	if (par.GetParameter("?").used || par.GetParameter("?!").used) 
		return 0xF00; 

	par.ReturnValue("s", size);
	const int limit = UCHAR_MAX / 3;

	Pitri::Image img(size, size);
	for (unsigned y = 0; y < img.Height(); ++y)
	{
		const int dy = (static_cast<int>(y) - size / 2) * UCHAR_MAX / size;
		for (unsigned x = 0; x < img.Width(); ++x)
		{
			const int dx = (static_cast<int>(x) - size / 2) * UCHAR_MAX / size;

			int dir_a = limit + dy / 2 + dx;
			int dir_b = limit + dy / 2 - dx;
			int dir_c = limit - dy;

			auto &px = img.Pixel(x, y);
			px.r = dir_a > 0 ? dir_a : 0;
			px.g = dir_b > 0 ? dir_b : 0;
			px.b = dir_c > 0 ? dir_c : 0;
			px.a = 255;
		}
	}

	Pitri::ImageEditor::RoundCorners(img, 50, true);

	if(img.Save("result.png"))
		std::cout << "Success!" << std::endl;
	std::cin.get();
	return 0;
}