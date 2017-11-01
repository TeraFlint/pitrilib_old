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

	typedef bool(*actionfunc)(const ImageAction &, Image &);

	class ImageEditorBase
	{
		protected:
			bool AdjustBorders(unsigned &begin, unsigned &end) const;

			std::vector<Image *> layers;
			std::vector<bool> ownership;

		public:
			~ImageEditorBase();

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
	};

	class ImageEditor : public ImageEditorBase
	{
		protected:
			static std::map<std::string, actionfunc> GetActions();
			static bool HasAction(const std::string &key);

			static bool Action_RoundCorners(const ImageAction &data, Image &img);

			static bool SubAction_SquashX(const ImageAction &data, Image &img);
			static bool SubAction_SquashY(const ImageAction &data, Image &img);
			static bool SubAction_SquashXY(const ImageAction &data, Image &img);
			static bool SubAction_StretchX(const ImageAction &data, Image &img);
			static bool SubAction_StretchY(const ImageAction &data, Image &img);
			static bool SubAction_StretchXY(const ImageAction &data, Image &img);
			static bool Action_Resize(const ImageAction &data, Image &img);

			static bool Action_ResizeCanvas(const ImageAction &data, Image &img);
			static bool Action_AutoCrop(const ImageAction &data, Image &img);

			static bool Action_ShiftImage(const ImageAction &data, Image &img);

			static bool SubAction_Bezier(const ImageAction &data, Image &img);
			static bool Action_DrawLine(const ImageAction &data, Image &img);
			static bool Action_DrawBezier(const ImageAction &data, Image &img);

			static bool Action_FillRect(const ImageAction &data, Image &img);

			static bool Action_TileImage(const ImageAction &data, Image &img);

		public:
			bool PerformLayerAction(const std::string &name, ImageAction &data, unsigned begin = 0, unsigned end = -1);
			static bool PerformLayerAction(const std::string &name, ImageAction &data, Image &img);

			/*RoundCorners() searches for the corner pixels and makes them transparent.
			- img: Reference to the image.
			- radius: Radius of the corner circles.
			- percent: Percentage values, if true. Values over 50% will be treated as 50%.*/
			static bool RoundCorners(Image &img, const unsigned radius, bool percent = false);

			/*Resize() scales an image and its contents to fit the new measurements.
			- img: Reference to the image.
			- width: New width. If not set, it will be calculated from the height in order to sustain the original aspect ratio.
			- height: New height. If not set, it will be calculated from the width.
			- percent: Percentage values, if true. */
			static bool Resize(Image &img, const unsigned width, const unsigned height = 0, const bool percent = false);

			/*ResizeCanvas() gives the Canvas a new size without resizing its contents.
			- img: Reference to the image.
			- width, height: New image size.
			- x, y: Relative new position. 0% = centered, +-100% = aligned on the border, if percentage is used.
			- percent: Percentage values, if true.*/
			static bool ResizeCanvas(Image &img, const unsigned width, const unsigned height, const int x, const int y, bool percent = false);

			/*AutoCrop() automatically gets rid of all the empty space around the image.
			- img: Reference to the image.*/
			static bool AutoCrop(Image &img);
			bool CropLayers(unsigned from = 0, unsigned to = -1);

			/*ShiftImage() Shifts the contents of the image.
			- x: Relative horizontal difference.
			- y: Relative vertical difference.
			- percent: Percentage values, if true.*/
			static bool ShiftImage(Image &img, const int x, const int y, const bool percent = false);

			/*DrawLine() draws a line from (x1, y1) to (x2, y2).
			- x1, x2: Start coordinates.
			- x2, y2: End coordinates.
			- strength: Line thickness in pixels.*/
			static bool DrawLine(Image &img, const Color &clr, float strength, const unsigned x1, const unsigned y1, const unsigned x2, const unsigned y2, bool percent = false);

			/*DrawBezier() draws a bezier curve.*/
			static bool DrawBezier(Image &img, const Color &clr, float strength, const std::vector<int> &coordinates, bool percent = false);

			/*FillRect() fills a rectangle of the image with the chosen color. Default is the whole image.
			- img: Reference to the image.
			- clr: The chosen color.
			- x, y: Top left position of the rectangle. 
			- w, h: Width and height of the rectangle. (-1) for maximum size.
			- percent: Percentage values, if true.*/
			static bool FillRect(Image &img, const Color &clr, const unsigned x = 0, const unsigned y = 0, const unsigned w = -1, const unsigned h = -1, bool percent = false);

			/*TileImage() overwrites the whole image with the same texture. If the image is bigger than the source, the source will be looped.
			- img: Reference to the image.
			- source: Reference to the source image.
			- overlay: If true, the source will be an overlay over the original file instead overwriting it.*/
			static bool TileImage(Image &img, const Image &source, const bool overlay = false);
	};
}