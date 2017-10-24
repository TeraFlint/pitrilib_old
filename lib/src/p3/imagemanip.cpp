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
		actions["FillRect"] = ImageEditor::Action_FillRect;
		actions["Resize"] = ImageEditor::Action_Resize;
		actions["ResizeCanvas"] = ImageEditor::Action_ResizeCanvas;
		actions["RoundCorners"] = ImageEditor::Action_RoundCorners;
		actions["ShiftImage"] = ImageEditor::Action_ShiftImage;
		actions["TileImage"] = ImageEditor::Action_TileImage;
		return actions;
	}
	bool ImageEditor::HasAction(const std::string &key)
	{
		auto map = GetActions();
		return map.find(key) != map.end();
	}

	ImageEditor::~ImageEditor()
	{
		unsigned size = layers.size();
		for (unsigned i = 0; i < size; ++i)
		{
			if (layers[i] && ownership[i])
			{
				delete layers[i];
				layers[i] = 0;
			}
		}
	}

	std::vector<Image *>::iterator ImageEditor::begin()
	{
		return layers.begin();
	}
	std::vector<Image *>::iterator ImageEditor::end()
	{
		return layers.end();
	}

	bool ImageEditor::_CreateLayer(const std::string &path)
	{
		Image *img = new Image;
		if (img->Load(path))
			return _AddLayer(img, true);
		return false;
	}
	bool ImageEditor::_CreateLayer(unsigned width, unsigned height, Color clr)
	{
		Image *img = new Image(width, height);
		for (auto &px : *img)
		{
			px = clr;
		}
		return _AddLayer(img, true);
	}
	bool ImageEditor::_AddLayer(Image *img, bool new_operator)
	{
		//if (!img) return false;
		unsigned index = layers.size();
		layers.resize(index + 1);
		layers[index] = img;
		ownership.resize(index + 1);
		ownership[index] = new_operator;
		return true;
	}
	bool ImageEditor::_RemoveLayer(const unsigned index)
	{
		if (index >= layers.size()) return false;
		if (layers[index] && ownership[index])
		{
			delete layers[index];
			layers[index] = 0;
		}

		layers.erase(layers.begin() + index);
		ownership.erase(ownership.begin() + index);
		return true;
	}
	bool ImageEditor::_RemoveLayers(const unsigned begin, const unsigned end)
	{
		unsigned a = begin, b = end;
		if (!AdjustBorders(a, b))
			return false;
		if (a == b)
			return _RemoveLayer(a);

		for (unsigned i = a; i < b; ++i)
		{
			if (layers[i] && ownership[i])
				delete layers[i];
		}
		layers.erase(layers.begin() + a, layers.begin() + b + 1);
		ownership.erase(ownership.begin() + a, ownership.begin() + b + 1);
		return true;
	}
	bool ImageEditor::_ClearInvalidLayers()
	{
		bool success = false;
		for (int i = layers.size() - 1; i >= 0; --i)
		{
			if (!layers[i])
				success |= _RemoveLayer(i);
		}
		return success;
	}

	Image *ImageEditor::_GetLayer(const unsigned index) const
	{
		if (index == -1) return layers[layers.size() - 1];
		if (index >= layers.size()) return 0;
		return layers[index];
	}
	std::vector<Image *> ImageEditor::_GetLayers() const
	{
		return layers;
	}
	unsigned ImageEditor::_GetLayerCount() const
	{
		return layers.size();
	}

	bool ImageEditor::_SwapLayers(unsigned from, unsigned to)
	{
		if (from >= layers.size() || to >= layers.size())
			return false;
		Image *buffer = layers[to];
		layers[to] = layers[from];
		layers[to] = buffer;
	}
	bool ImageEditor::_MergeLayers(const bool remove, unsigned from, unsigned to)
	{
		if (!layers.size() || !AdjustBorders(from, to) || from == to)
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
		for (int i = to; i >= static_cast<int>(from); --i)
		{
			Image *layer = layers[i];
			if (!layer) continue;

			Color *to = &img.Pixel(0, 0), *from = &layer->Pixel(0, 0);

			for (unsigned y = 0; y < layer->Height(); ++y)
			{
				for (unsigned x = 0; x < layer->Width(); ++x)
				{
					*to++ <<= *from++;
				}
				from += width - layer->Width();
			}
		}
		if (!layers[from])
		{
			layers[from] = new Image(width, height);
			ownership[from] = true;
		}
		*layers[from] = img;

		if (remove)
		{
			for (unsigned i = from + 1; i < to; ++i)
			{
				if (layers[i] && ownership[i])
				{
					delete layers[i];
					layers[i] = 0;
				}
			}
			layers.erase(layers.begin() + from + 1, layers.begin() + to + 1);
			ownership.erase(ownership.begin() + from + 1, ownership.begin() + to + 1);
		}
		return true;
	}
	bool ImageEditor::_CollapseLayers(const bool remove)
	{
		return _MergeLayers(remove, 0, layers.size() - 1);
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
	bool ImageEditor::Action_RoundCorners(const ImageAction &data, Image &img)
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

	/* Resizing and repositioning */

	bool ImageEditor::SubAction_SquashX(const ImageAction &data, Image &img)
	{
		if (!data.Valid(1)) return false;
		unsigned width = data.GetVal(0, img.Width());
		unsigned height = img.Height();

		Image result(width, height);
		Color *dst = &result.Pixel(0, 0), *src;
		for (unsigned y = 0; y < height; ++y)
		{
			for (unsigned x = 0; x < width; ++x)
			{
				int r = 0, g = 0, b = 0, a = 0, pxs = 0;
				float clrpxs = 0;

				for (unsigned x2 = x*img.Width() / width; x2 < (x + 1)*img.Width() / width; ++x2)
				{
					src = &img.Pixel(x2, y);
					pxs++;
					a += src->a;
					if (src->a)
					{
						r += src->r * src->a / 255;
						g += src->g * src->a / 255;
						b += src->b * src->a / 255;
						clrpxs += (float)src->a / 255;
					}
				}
				if (clrpxs)
				{
					dst->r = r / clrpxs;
					dst->g = g / clrpxs;
					dst->b = b / clrpxs;
					dst->a = a / pxs;
				}
				++dst;
			}
		}
		img = result;
		return true;
	}
	bool ImageEditor::SubAction_SquashY(const ImageAction &data, Image &img)
	{
		if (!data.Valid(2)) return false;
		unsigned width = img.Width();
		unsigned height = data.GetVal(1, img.Height());

		Image result(width, height);
		Color *dst = &result.Pixel(0, 0), *src;
		for (unsigned y = 0; y < height; ++y)
		{
			for (unsigned x = 0; x < width; ++x)
			{
				int r = 0, g = 0, b = 0, a = 0, pxs = 0;
				float clrpxs = 0;

				for (unsigned y2 = y*img.Height() / height; y2 < (y + 1)*img.Height() / height; ++y2)
				{
					src = &img.Pixel(x, y2);
					pxs++;
					a += src->a;
					if (src->a)
					{
						r += src->r * src->a / 255;
						g += src->g * src->a / 255;
						b += src->b * src->a / 255;
						clrpxs += (float)src->a / 255;
					}
				}
				if (clrpxs)
				{
					dst->r = r / clrpxs;
					dst->g = g / clrpxs;
					dst->b = b / clrpxs;
					dst->a = a / pxs;
				}
				++dst;
			}
		}
		img = result;
		return true;
	}
	bool ImageEditor::SubAction_SquashXY(const ImageAction &data, Image &img)
	{
		if (!data.Valid(2)) return false;
		unsigned width = data.GetVal(0, img.Width());
		unsigned height = data.GetVal(1, img.Height());

		Image result(width, height);
		Color *dst = &result.Pixel(0, 0), *src;
		for (unsigned y = 0; y < height; ++y)
		{
			for (unsigned x = 0; x < width; ++x)
			{
				int r = 0, g = 0, b = 0, a = 0, pxs = 0;
				float clrpxs = 0;

				for (unsigned y2 = y*img.Height() / height; y2 < (y + 1)*img.Height() / height; ++y2)
				{
					for (unsigned x2 = x*img.Width() / width; x2 < (x + 1)*img.Width() / width; ++x2)
					{
						src = &img.Pixel(x2, y2);
						pxs++;
						a += src->a;
						if (src->a)
						{
							r += src->r * src->a / 255;
							g += src->g * src->a / 255;
							b += src->b * src->a / 255;
							clrpxs += (float)src->a / 255;
						}
					}
				}
				if (clrpxs)
				{
					dst->r = r / clrpxs;
					dst->g = g / clrpxs;
					dst->b = b / clrpxs;
					dst->a = a / pxs;
				}
				++dst;
			}
		}
		img = result;
		return true;

		/*
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
				}
			}
		}
		img = result;
		return true;
		*/
	}
	bool ImageEditor::SubAction_StretchX(const ImageAction &data, Image &img)
	{
		if (!data.Valid(1)) return false;
		unsigned width = data.GetVal(0, img.Width());
		unsigned height = img.Height();

		/*Image result(width, height);
		Color *dst = &result.Pixel(0, 0), *src;
		for (unsigned y = 0; y < height; ++y)
		{
			for (unsigned x = 0; x < width; ++x)
			{
				//...
				++dst;
			}
			//...
		}
		img = result;*/
		return true;
	}
	bool ImageEditor::SubAction_StretchY(const ImageAction &data, Image &img)
	{
		if (!data.Valid(2)) return false;
		unsigned width = img.Width();
		unsigned height = data.GetVal(1, img.Height());

		/*Image result(width, height);
		Color *dst = &result.Pixel(0, 0), *src;
		for (unsigned y = 0; y < height; ++y)
		{
			for (unsigned x = 0; x < width; ++x)
			{
				//...
				++dst;
			}
			//...
		}
		img = result;*/
		return true;
	}
	bool ImageEditor::SubAction_StretchXY(const ImageAction &data, Image &img)
	{
		if (!data.Valid(2)) return false;
		unsigned width = img.Width();
		unsigned height = data.GetVal(1, img.Height());

		/*Image result(width, height);
		Color *dst = &result.Pixel(0, 0), *src;
		for (unsigned y = 0; y < height; ++y)
		{
			for (unsigned x = 0; x < width; ++x)
			{
				//...
				++dst;
			}
			//...
		}
		img = result;*/
		return true;
	}

	bool ImageEditor::Resize(Image &img, const unsigned width, const unsigned height, const bool percent)
	{
		ImageAction action;
		action.SetVal(0, width, percent);
		action.SetVal(1, height, percent);
		return Action_Resize(action, img);
	}
	bool ImageEditor::Action_Resize(const ImageAction &data, Image &img)
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

		int xdiff = width - img.Width(), ydiff = height - img.Height();
		if (!xdiff && !ydiff) return false;

		ImageAction newdata;
		newdata.SetVal(0, width);
		newdata.SetVal(1, height);

		actionfunc func = 0;
		if (xdiff < 0 && ydiff < 0)
			func = SubAction_SquashXY;
		else if (xdiff > 0 && ydiff > 0)
			func = SubAction_StretchXY;
		if (func) return func(newdata, img);

		if (xdiff)
		{
			func = xdiff < 0 ? SubAction_SquashX : SubAction_StretchX;
			func(newdata, img);
		}

		if (ydiff)
		{
			func = ydiff < 0 ? SubAction_SquashY : SubAction_StretchY;
			func(newdata, img);
		}
		return true;
	}

	bool ImageEditor::ResizeCanvas(Image &img, const unsigned width, const unsigned height, const int x, const int y, bool percent)
	{
		ImageAction action;
		action.SetVal(0, width, percent);
		action.SetVal(1, height, percent);
		action.SetVal(2, x, percent);
		action.SetVal(3, y, percent);
		return Action_ResizeCanvas(action, img);
	}
	bool ImageEditor::Action_ResizeCanvas(const ImageAction &data, Image &img)
	{
		if (!data.Valid(2))
			return false;

		//...
		return true;
	}

	bool ImageEditor::ShiftImage(Image &img, const int x, const int y, const bool percent)
	{
		ImageAction action;
		action.SetVal(0, x, percent);
		action.SetVal(1, y, percent);
		return Action_ShiftImage(action, img);
	}
	bool ImageEditor::Action_ShiftImage(const ImageAction &data, Image &img)
	{
		if (!data.Valid(2)) return false;
		int dx = data.GetVal(0, img.Width());
		int dy = data.GetVal(1, img.Height());
		if (!dx && !dy) return false;

		Image result(img.Width(), img.Height());
		if (abs(dx) < img.Width() && abs(dy) < img.Height())
		{
			unsigned wdt = img.Width() - abs(dx), hgt = img.Height() - abs(dy);
			unsigned x1 = 0, x2 = img.Width(), y1 = 0, y2 = img.Height();
			if (dx < 0)
				x2 = wdt;
			else if (dx > 0)
				x1 = dx;
			if (dy < 0)
				y2 = hgt;
			else if (dy > 0)
				y1 = dy;

			Color *src = &img.Pixel(0, 0);
			if (dx < 0)
				src += -dx;
			if (dy < 0)
				src += -dy * img.Width();

			Color *dst = &result.Pixel(x1, y1);
			for (unsigned y = y1; y < y2; ++y)
			{
				for (unsigned x = x1; x < x2; ++x)
				{
					*dst++ = *src++;
				}
				src += abs(dx);
				dst += abs(dx);
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
	bool ImageEditor::Action_FillRect(const ImageAction &data, Image &img)
	{
		Color clr;
		clr.r = data.GetVal(0);
		clr.g = data.GetVal(1);
		clr.b = data.GetVal(2);
		clr.a = data.GetVal(3);

		int xpos = data.GetVal(4, img.Width());
		int ypos = data.GetVal(5, img.Height());
		int wdt = data.GetVal(6, img.Width());
		int hgt = data.GetVal(7, img.Height());

		if (wdt < 0)
		{
			xpos += wdt;
			wdt *= -1;
		}
		if (hgt < 0)
		{
			ypos += hgt;
			hgt *= -1;
		}

		if (xpos < 0) xpos = 0;
		if (ypos < 0) ypos = 0;
		if (xpos + wdt >= img.Width()) wdt = img.Width() - xpos - 1;
		if (ypos + hgt >= img.Height()) hgt = img.Height() - ypos - 1;

		Color *px = &img.Pixel(xpos, ypos);
		for (unsigned y = ypos; y < ypos + hgt; ++y)
		{
			for (unsigned x = xpos; x < xpos + wdt; ++x)
			{
				*px++ <<= clr;
			}
			px += img.Width() - wdt;
		}
		return true;
	}

	/* Tiling the image */

	bool ImageEditor::TileImage(Image &img, const Image &source, const bool overlay)
	{
		ImageAction action;
		action.SetVal(0, reinterpret_cast<int>(&source));
		action.SetVal(1, overlay);
		return Action_TileImage(action, img);
	}
	bool ImageEditor::Action_TileImage(const ImageAction &data, Image &img)
	{
		Image *other = reinterpret_cast<Image*>(data.GetVal(0));
		if (!other) return false;
		bool overlay = data.GetVal(1);

		Color *dst = &img.Pixel(0, 0);
		for (unsigned y = 0; y < img.Height(); ++y)
		{
			Color *src = &other->Pixel(0, y % other->Height());
			for (unsigned x = 0; x < img.Width(); ++x)
			{
				if (!overlay)
					*dst++ = *src++;
				else
					*dst++ <<= *src++;
				if ((x + 1) % other->Width() == 0)
					src -= other->Width();
			}
		}

		return true;
	}




	/* Color functions */

	bool ChangeColorLighting(Color &clr, const unsigned char light, const bool brighten)
	{
		if (brighten && light == 0 || !brighten && light == 255)
			return false;

		unsigned char *c = &clr.r;
		for (unsigned i = 0; i < 3; ++i)
		{
			if (!brighten)
				*c++ = *c * light / 255;
			else
			{
				*c++ = 255 - (255 - light) * (255 - *c) / 255;
			}
		}
		return true;
	}
	bool ChangeColorLighting(Color &clr, float light)
	{
		if (!light) return false;
		if (light < -1) light = -1;
		else if (light > 1) light = 1;

		unsigned char *c = &clr.r;
		for (unsigned i = 0; i < 3; ++i)
		{
			if (light < 0) *c++ *= light + 1;
			else *c++ = 255 - (255 * (1 - light)) * (255 - *c) / 255;
		}
		return true;
	}
}