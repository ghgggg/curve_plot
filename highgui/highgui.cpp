#include "highgui.hpp"

namespace chaos
{
	std::vector<Color> CreateColorPool()
	{
		std::vector<Color> pool;
		// permutation produces 27 different colors
		std::vector<uchar> val = { 0, 0, 0, 128, 128, 128, 255, 255, 255 };
		do
		{
			auto same = std::find_if(pool.begin(), pool.end(), [=](Color& c) {
				if (c[0] == val[2] && c[1] == val[1] && c[2] == val[0])
				{
					return true;
				}
				return false;
			});

			if (same == pool.end())
			{
				pool.push_back(Color(val[2], val[1], val[0]));
			}
		} while (std::next_permutation(val.begin(), val.end()));

		return pool;
	}

	std::vector<Color> ColorPool::pool = CreateColorPool();
	int ColorPool::idx = 0;

	Color ColorPool::Get(int idx)
	{
		return pool[idx];
	}

	Color ColorPool::Next()
	{
		return pool[idx++ % 25 + 1];
	}

	int Figure::idx = 0;
	void Figure::Hold(State state)
	{
		this->state = state;
	}

	void Figure::ShowTitle()
	{
		if (!title.empty())
		{
			float font_scale = 0.6f;
			auto text_size = cv::getTextSize(title, cv::HersheyFonts::FONT_HERSHEY_SIMPLEX, font_scale, 1, 0);
			cv::putText(figure, title, cv::Point2f(figure.cols / 2.f - text_size.width / 2.f, 30),
				cv::HersheyFonts::FONT_HERSHEY_SIMPLEX, font_scale, ColorPool::Get(BLACK), 1, cv::LINE_AA);
		}
	}
}