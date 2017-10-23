#pragma once
#include "image.h"
#include <map>

namespace Pitri
{
	typedef Image(*imgfunc)(Image);
	typedef void(*imgmanip)(Image &);

	typedef Color(*clrfunc)(Color);
	typedef void(*clrmanip)(Color &);

	class ImageAction
	{
		private:
			//Values, bool percent
			std::vector<std::pair<int, bool>> values;
			//Precision values, bool percent
			std::vector<std::pair<float, bool>> prec;

			//Some strings, if needed.
			std::vector<std::string> strings;

		public:
			/*Valid() checks, if the data is enough for the requirements of the function.
			- value_count is the minimum amount for normal values.
			- prec_count is the minimum amount for precision values.
			- string_count is the minimum amount for strings.*/
			bool Valid(unsigned value_count, unsigned prec_count = 0, unsigned string_count = 0) const;

			void ReserveVal(unsigned amount);
			void SetVal(unsigned index, int value, bool percent = false);
			int GetVal(unsigned index, int max = 100) const;

			void ReservePrec(unsigned amount);
			void SetPrec(unsigned index, float value, bool percent = false);
			float GetPrec(unsigned index, float max = 100) const;

			void ReserveStr(unsigned amount);
			void SetStr(unsigned index, const std::string &value);
			std::string GetStr(unsigned index) const;
	};

	typedef bool(*actionfunc)(ImageAction &, Image &);

	class ImageEditor
	{
		private: 
			std::vector<Image *> layers;
			std::vector<bool> ownership;

		private:
			bool AdjustBorders(unsigned &begin, unsigned &end) const;
			static std::map<std::string, actionfunc> GetActions();
			static bool HasAction(const std::string &key);

			static bool Action_RoundCorners(ImageAction &data, Image &img);
			//static bool Action_SampleUp(ImageAction &data, Image &img);

			static bool SubAction_SquashX(ImageAction &data, Image &img);
			static bool SubAction_SquashY(ImageAction &data, Image &img);
			static bool SubAction_SquashXY(ImageAction &data, Image &img);
			static bool SubAction_StretchX(ImageAction &data, Image &img);
			static bool SubAction_StretchY(ImageAction &data, Image &img);
			static bool SubAction_StretchXY(ImageAction &data, Image &img);
			static bool Action_Resize(ImageAction &data, Image &img);

			static bool Action_FillRect(ImageAction &data, Image &img);



		public:
			~ImageEditor();

			std::vector<Image *>::iterator begin();
			std::vector<Image *>::iterator end();

			bool _CreateLayer(const std::string &path);
			bool _CreateLayer(unsigned width, unsigned height, Color clr = Color());
			bool _AddLayer(Image *img, bool new_operator = false);
			bool _RemoveLayer(const unsigned index);
			bool _RemoveLayers(const unsigned begin, const unsigned end);
			bool _ClearInvalidLayers();

			Image *_GetLayer(const unsigned index) const;
			std::vector<Image *> _GetLayers() const;
			unsigned _GetLayerCount() const;

			bool _SwapLayers(unsigned from, unsigned to);
			bool _MergeLayers(const bool remove = true, unsigned from = 0, unsigned to = -1);
			bool _CollapseLayers(const bool remove = true);

			bool PerformLayerAction(const std::string &name, ImageAction &data, unsigned begin = 0, unsigned end = -1);
			static bool PerformLayerAction(const std::string &name, ImageAction &data, Image &img);

			/*RoundCorners() searches for the corner pixels and makes them transparent.
			- img: Reference to the image.
			- radius: Radius of the corner circles.
			- percent: Percentage values, if true. Values over 50% will be treated as 50%. */
			static bool RoundCorners(Image &img, const unsigned radius, bool percent = false);

			/*Resize() grows, shrinks, stretches or squashes a image to fit the new measurements.
			- img: Reference to the image.
			- width: New width. If not set, it will be calculated from the height in order to sustain the original aspect ratio.
			- height: New height. If not set, it will be calculated from the width.*/
			static bool Resize(Image &img, const unsigned width, const unsigned height = 0, const bool percent = false);

			/*FillRect() fills a rectangle of the image with the chosen color. Default is the whole image.
			- img: Referenace to the image.
			- clr: The chosen color.
			- x, y: Top left position of the rectangle. 
			- w, h: Width and height of the rectangle. (-1) for maximum size.*/
			static bool FillRect(Image &img, const Color &clr, const unsigned x = 0, const unsigned y = 0, const unsigned w = -1, const unsigned h = -1, bool percent = false);
	};

	/*FillImage() overwrites the whole image with the same color.
	- img: Reference to the image.
	- source: Source color.*/
	void FillImage(Image &img, Color source);

	/*FillImage() overwrites the whole image with the same texture. If the image is bigger than the source, the source will be looped.
	- img: Reference to the image.
	- source: Source texture.*/
	void FillImage(Image &img, Image source);

	/*InvertColor() */
	void InvertColor(Color &clr, unsigned components = 7);

	void MixColor(Color &base, Color other, unsigned char opacity = 127);

	void OverlayColor(Color &base, Color other);
}