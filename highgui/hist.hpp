#pragma once

#include "highgui.hpp"

namespace chaos
{
	class HistFigure : public Figure
	{
	public:
		template<class ... Args>
		void Plot(const cv::Mat& data, Args ... args)
		{
			local = true;
			DummyWrap(Unpack(args)...);
			local = false;

			PushHist(data);
		}

		static std::shared_ptr<HistFigure> Figure(const std::string& name = std::string());
	protected:
		virtual void PushHist(const cv::Mat& data) = 0;
	};
}