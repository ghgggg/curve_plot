#pragma once

#include"highgui.hpp"

namespace chaos
{
	enum ColorMap
	{
		MGRAY = -1,
		AUTUMN = 0,
		BONE = 1,
		JET = 2,
		RAINBOW = 4,
		COOL = 8,
		HSV = 9,
		PINK = 10,
		HOT = 11,
	};

	class CHAOS_API ImagescFigure : public Figure
	{
	public:
		template<class ... Args>
		void Imagesc(const cv::Mat& data, Args ... args)
		{
			local = true;
			DummyWrap(Unpack(args)...);
			local = false;

			PushImage(data);
		}

		static std::shared_ptr<ImagescFigure> Figure(const std::string& name = std::string());
	protected:
		virtual void PushImage(const cv::Mat& data) = 0;
	};
}