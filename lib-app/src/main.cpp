#pragma optimize("gsy", on)

#include "pitri.h"

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

	std::cout << par.PrintHelp(par.GetParameter("?!").used) << std::endl;
	if (par.GetParameter("?").used || par.GetParameter("?!").used)
		return 0xF00;

	par.ReturnValue("s", size);
	const int limit = UCHAR_MAX / 3;

	std::cout << "=== Stuff ===" << std::endl;
	std::cout << "Creating circle.png... ";
	Pitri::Image circle(size, size);
	for (unsigned y = 0; y < circle.Height(); ++y)
	{
		const int dy = (static_cast<int>(y) - size / 2) * UCHAR_MAX / size;
		for (unsigned x = 0; x < circle.Width(); ++x)
		{
			const int dx = (static_cast<int>(x) - size / 2) * UCHAR_MAX / size;

			int dir_a = limit + dy / 2 + dx;
			int dir_b = limit + dy / 2 - dx;
			int dir_c = limit - dy;

			auto &px = circle.Pixel(x, y);
			px.r = dir_a > 0 ? dir_a : 0;
			px.g = dir_b > 0 ? dir_b : 0;
			px.b = dir_c > 0 ? dir_c : 0;
			px.a = 255;
		}
	}

	Pitri::ImageEditor::RoundCorners(circle, 50, true);
	if (circle.Save("circle.png"))
		std::cout << "Success!";
	else
		std::cout << "Fail!";
	std::cout << std::endl << std::endl;

	{
		Pitri::Image copy = circle;
		Pitri::ImageEditor::ResizeCanvas(copy, 150, 150, 120, -80, true);
		copy.Save("shifted-circle.png");
		Pitri::ImageEditor::DrawLine(copy, Pitri::Color(0x63, 0xa5, 0xff), 2, 10, 10, 90, 90, true);

		Pitri::ImageEditor::AutoCrop(copy);
		copy.Save("cropped-circle.png");
	}
	std::cin.get();
	return 0;
}