#include "imagemanip.h"
#include <algorithm>

namespace Pitri
{
	bool ImageAction::Valid(unsigned value_count, unsigned prec_count, unsigned string_count) const
	{
		return value_count <= values.size() 
			&& prec_count <= prec.size() 
			&& string_count <= strings.size();
	}

	void ImageAction::ReserveVal(unsigned amount)
	{
		values.reserve(amount);
	}
	void ImageAction::SetVal(unsigned index, int value, bool percent)
	{
		if (index + 1 >= values.size())
			values.resize(index + 1);
		values[index] = { value, percent };
	}
	int ImageAction::GetVal(unsigned index, int max) const
	{
		if (index >= values.size()) return 0;
		if(!values[index].second)
			return values[index].first;
		return values[index].first * max / 100;
	}

	void ImageAction::ReservePrec(unsigned amount)
	{
		prec.reserve(amount);
	}
	void ImageAction::SetPrec(unsigned index, float value, bool percent)
	{
		if (index + 1 >= prec.size())
			prec.resize(index + 1);
		prec[index] = { value, percent };
	}
	float ImageAction::GetPrec(unsigned index, float max) const
	{
		if (index >= prec.size()) return 0;
		if (!prec[index].second)
			return prec[index].first;
		return prec[index].first * max / 100;
	}

	void ImageAction::ReserveStr(unsigned amount)
	{
		strings.reserve(amount);
	}
	void ImageAction::SetStr(unsigned index, const std::string &value)
	{
		if (index + 1 >= strings.size())
			strings.resize(index + 1);
		strings[index] = value;
	}
	std::string ImageAction::GetStr(unsigned index) const
	{
		if (index >= strings.size()) return "";
		return strings[index];
	}

	bool ImageEditor::AdjustBorders(unsigned &begin, unsigned &end) const
	{
		if (begin > end)
		{
			begin ^= end;
			end ^= begin;
			begin ^= end;
		}
		if (begin >= layers.size())
			return false;
		if (end >= layers.size())
			end = layers.size() - 1;
		return true;
	}

	std::map<std::string, actionfunc> ImageEditor::GetActions()
	{
		std::map<std::string, actionfunc> actions;
		actions["Resize"] = ImageEditor::Action_SampleDown;
		actions["RoundCorners"] = ImageEditor::Action_RoundCorners;
		return actions;
	}
	bool ImageEditor::HasAction(const std::string &key)
	{
		auto map = GetActions();
		return map.find(key) != map.end();
	}

	std::vector<Image *>::iterator ImageEditor::begin()
	{
		return layers.begin();
	}
	std::vector<Image *>::iterator ImageEditor::end()
	{
		return layers.end();
	}

	bool ImageEditor::AddLayer(Image *img)
	{
		if (!img) return false;
		layers.push_back(img);
		return true;
	}
	bool ImageEditor::RemoveLayer(const unsigned index, bool delete_ptr)
	{
		if (index >= layers.size()) return false;
		if (delete_ptr && layers[index]) delete layers[index];

		layers.erase(layers.begin() + index);
		return true;
	}
	bool ImageEditor::RemoveLayers(const unsigned begin, const unsigned end, bool delete_ptr)
	{
		unsigned a = begin, b = end;
		if (!AdjustBorders(a, b))
			return false;
		if (a == b)
			return RemoveLayer(a, delete_ptr);

		if (delete_ptr)
		{
			for (unsigned i = a; i < b; ++i)
			{
				if (layers[i])
					delete layers[i];
			}
		}
		layers.erase(layers.begin() + a, layers.begin() + b + 1);
		return true;
	}
	Image *ImageEditor::GetLayer(const unsigned index) const
	{
		if (index >= layers.size()) return 0;
		return layers[index];
	}
	std::vector<Image *> ImageEditor::GetLayers() const
	{
		return layers;
	}

	bool ImageEditor::MergeLayers(const bool remove, unsigned from, unsigned to)
	{
		if (!AdjustBorders(from, to))
			return false;
		if (from == to)
			return false;

		unsigned width = 0, height = 0;
		for (auto img : layers)
		{
			if (!img) continue;
			if (width < img->Width())
				width = img->Width();
			if (height < img->Height())
				height = img->Height();
		}
		if (!width || !height)
			return false;

		//overlay
		Image img(width, height);
		for (int i = to; i >= from; --i)
		{
			Image *layer = layers[i];
			Color *from = &img.Pixel(0, 0), *to = &layer->Pixel(0, 0);
			for (unsigned y = 0; y < layer->Height(); ++y)
			{
				for (unsigned x = 0; x < layer->Width(); ++x)
				{
					*to++ <<= *from++;
				}
				from += width - layer->Width();
			}
		}
		*layers[from] = img;

		if (remove)
			layers.erase(layers.begin() + from + 1, layers.begin() + to + 1);
		return true;
	}
	bool ImageEditor::CollapseLayers(const bool remove)
	{
		return MergeLayers(remove, 0, layers.size() - 1);
	}

	bool ImageEditor::PerformLayerAction(const std::string &name, ImageAction &data, unsigned begin, unsigned end)
	{
		if (!AdjustBorders(begin, end))
			return false;

		if(!HasAction(name))
			return false;

		bool result = false;
		for (unsigned i = begin; i <= end; ++i)
		{
			Image *img = layers[i];
			if (img)
				result |= PerformLayerAction(name, data, *img);
		}
		return result;
	}
	bool ImageEditor::PerformLayerAction(const std::string &name, ImageAction &data, Image &img)
	{
		auto actions = GetActions();
		auto entry = actions.find(name);
		if (entry == actions.end())
			return false;
		return entry->second(data, img);
	}

	/* Rounding corners of the images */

	bool ImageEditor::RoundCorners(Image &img, const unsigned radius, bool percent)
	{
		ImageAction action;
		action.SetVal(0, radius, percent);
		return Action_RoundCorners(action, img);
	}
	bool ImageEditor::Action_RoundCorners(ImageAction &data, Image &img)
	{
		if (!data.Valid(1))
			return false;

		const unsigned smaller = std::min(img.Width(), img.Height());
		int radius = data.GetVal(0, smaller);
		if (radius > smaller / 2)
			radius = smaller / 2;

		for (unsigned y = 0; y < radius; ++y)
		{
			for (unsigned x = 0; x < radius; ++x)
			{
				float dist = sqrt(pow((int)x - radius, 2) + pow((int)y - radius, 2));
				if (dist <= radius)	continue;

				unsigned char opacity = 0;
				if (dist <= radius + 1)
					opacity = 255 - 255 * (dist - radius);

				std::vector<unsigned> xpos = { x, img.Width() - 1 - x };
				std::vector<unsigned> ypos = { y, img.Height() - 1 - y };

				for (auto xp : xpos)
				{
					for (auto yp : ypos)
					{
						auto &p = img.Pixel(xp, yp);
						p.a = opacity * p.a / 255;
					}
				}
			}
		}
		return true;
	}

	/* Resizing an image */

	bool ImageEditor::SampleDown(Image &img, const unsigned width, const unsigned height, const bool percent)
	{
		ImageAction action;
		action.SetVal(0, width, percent);
		action.SetVal(1, height, percent);
		return Action_SampleDown(action, img);
	}
	bool ImageEditor::Action_SampleDown(ImageAction &data, Image &img)
	{
		if (!data.Valid(1))
			return false;

		unsigned width = data.GetVal(0, img.Width());
		unsigned height = data.GetVal(1, img.Height());

		if (!width && !height)
			return false;

		if (!height)
			height = img.Height() * width / img.Width();
		if (!width)
			width = img.Width() * height / img.Height();

		Image result(width, height);
		for (unsigned y = 0; y < result.Height(); ++y)
		{
			for (unsigned x = 0; x < result.Width(); ++x)
			{
				auto &p = result.Pixel(x, y);
				int r = 0, g = 0, b = 0, a = 0, pxs = 0;
				float clrpxs = 0;

				for (unsigned y2 = y*img.Height() / result.Height(); y2 < (y + 1)*img.Height() / result.Height(); ++y2)
				{
					for (unsigned x2 = x*img.Width() / result.Width(); x2 < (x + 1)*img.Width() / result.Width(); ++x2)
					{
						auto &p2 = img.Pixel(x2, y2);
						pxs++;
						a += p2.a;
						if (p2.a)
						{
							r += p2.r * p2.a / 255;
							g += p2.g * p2.a / 255;
							b += p2.b * p2.a / 255;
							clrpxs += (float)p2.a / 255;
						}
					}
				}

				if (clrpxs)
				{
					p.r = r / clrpxs;
					p.g = g / clrpxs;
					p.b = b / clrpxs;
					p.a = a / pxs;

					/*r /= clrpxs;
					g /= clrpxs;
					b /= clrpxs;
					a /= pxs;
					p = Color(r, g, b, a);*/
				}
			}
		}
		img = result;
		return true;
	}

	/* Filling forms */

	bool ImageEditor::FillRect(Image &img, const Color &clr, const unsigned x, const unsigned y, const unsigned w, const unsigned h, const bool percent)
	{
		unsigned wdt = w == -1 ? img.Width() : w;
		unsigned hgt = h == -1 ? img.Height() : h;

		ImageAction action;
		action.ReserveVal(8);
		action.SetVal(0, clr.r);
		action.SetVal(1, clr.g);
		action.SetVal(2, clr.b);
		action.SetVal(3, clr.a);
		action.SetVal(4, x, percent);
		action.SetVal(5, y, percent);
		action.SetVal(6, wdt, percent);
		action.SetVal(7, hgt, percent);
		return Action_FillRect(action, img);
	}
	bool ImageEditor::Action_FillRect(ImageAction &data, Image &img)
	{
		Color clr;
		clr.r = data.GetVal(0);
		clr.g = data.GetVal(1);
		clr.b = data.GetVal(2);
		clr.a = data.GetVal(3);

		unsigned x = data.GetVal(4, img.Width());
		unsigned y = data.GetVal(5, img.Height());
		unsigned wdt = data.GetVal(6, img.Width());
		unsigned hgt = data.GetVal(7, img.Height());

		return true;
	}




	void FillImage(Image &img, Color source)
	{
		for (auto &c : img.bitmap)
			c = source;
	}
	void FillImage(Image &img, Image source)
	{
		unsigned width = img.Width(), height = img.Height();
		for (unsigned y = 0; y < height; ++y)
		{
			for (unsigned x = 0; x < width; ++x)
			{
				img.bitmap[y*width + x] = source.Pixel(x % source.Width(), y % source.Height());
			}
		}
	}

	void InvertColor(Color &clr, unsigned components)
	{
		if (components & 1)
			clr.r = 255 - clr.r;
		if (components & 2)
			clr.g = 255 - clr.g;
		if (components & 4)
			clr.b = 255 - clr.b;
		if (components & 8)
			clr.a = 255 - clr.a;
	}

	void MixColor(Color &base, Color other, unsigned char opacity)
	{
		if (!base.a && !other.a)
			return;

		base.r += (other.r - base.r) * opacity / 255;
		base.g += (other.g - base.g) * opacity / 255;
		base.b += (other.b - base.b) * opacity / 255;
		base.a += (other.a - base.a) * opacity / 255;
	}

	void OverlayColor(Color &base, Color other)
	{
		if (!base.a && !other.a)
			return;

		base.r += (other.r - base.r) * other.a / 255;
		base.g += (other.g - base.g) * other.a / 255;
		base.b += (other.b - base.b) * other.a / 255;
		base.a = 255 - ((255 - base.a)*(255 - other.a) / 255);
	}
}