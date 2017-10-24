#pragma optimize("gsy", on)

#include "pitri.h"

namespace Pitri
{

}

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

	std::vector<std::string> users = Pitri::GetUserList();
	for (auto dirname : users)
	{
		Pitri::UserProfile user(dirname);
		if (user.Load())
		{
			std::string name = dirname;
			auto userdata = user.GetKeyValue("name");
			if (!userdata.contents.empty())
				name = userdata.contents[0];

			if (user.HasFlag())
			{
				std::cout << "Transforming flag of " << name << "... ";
				Pitri::Image origin = user.GetFlag(), transform(size, size);
				float factor = 1.2;
				float frequency = 6.24, amplitude = 0.05;

				unsigned frames = 20;

				for (unsigned i = 0; i < frames; ++i)
				{
					Pitri::Color *px = &transform.Pixel(0, 0);
					float phase = static_cast<float>(i) / frames;
					for (int y = 0; y < transform.Height(); ++y)
					{
						for (int x = 0; x < transform.Width(); ++x)
						{
							float dx = (static_cast<float>(x) / size - 0.5) * factor, dy = (static_cast<float>(y) / size - 0.5) * factor;
							float xpos = -amplitude * cos(frequency * dy);
							float ypos = -amplitude * sin(frequency * (dx - phase)) * (dx / factor + 0.5);

							//derivation of xpos
							float light = 0.5 * (cos(frequency * (dx - phase)) + 1) * (dx / factor + 0.5) - 0.25;

							Pitri::Color result = origin.InterpolatePixelColor(dx + xpos + 0.5, dy + ypos + 0.5);
							Pitri::ChangeColorLighting(result, light);
							*px++ = result;
						}
					}
					transform.Save("frame-" + dirname + "-" +  std::to_string(i) + ".png");
				}
				std::cout << "Success!" << std::endl << std::endl;
			}
		}
	}
	std::cin.get();
	return 0;
}