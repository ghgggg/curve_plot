#pragma once

#include "highgui.hpp"
//#include "utils/utils.hpp"

CHAOS_API cv::Mat LineSpace(float a, float b, int n);
namespace chaos
{
	class CHAOS_API PlotFigure : public Figure
	{
	public:
		/// <summary>Plot the curve</summary>
		/// <param name="x">X data, a row data</param>
		/// <param name="y">Y data, a row data</param>
		/// <param name="args">
		/// <para>Curve args</para>
		/// <para>Just like Matlab, for example, figure->Plot(x, y, "LineWidth", 2, ...)</para>
		/// <para>For more details, please see Wikis and the args belows</para>
		/// <para>"LineWidth", int</para>
		/// <para>"Color", Color</para>
		/// <para>"Marker", int, </para>
		/// <para>"Legend", const char*</para>
		/// </param>
		template<class ... Args>
		void Plot(const cv::Mat& x, const cv::Mat& y, Args ... args)
		{
			local = true;
			DummyWrap(Unpack(args)...);
			local = false;

			PushCurve(x, y);
		}
		// @overload
		template<class ... Args>
		void Plot(const cv::Mat& y, Args ... args)
		{
			local = true;
			DummyWrap(Unpack(args)...);
			local = false;

			cv::Mat x = LineSpace(0.f, y.cols - 1.f, y.cols);
			PushCurve(x, y);
		}

		static std::shared_ptr<PlotFigure> Figure(const std::string& name = std::string());
	protected:
		virtual void PushCurve(const cv::Mat& x, const cv::Mat& y) = 0;
	};
}