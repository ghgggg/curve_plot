#include "plot.hpp"

#include <set>

namespace chaos
{
	cv::Mat LineSpace(float a, float b, int n)
	{
		//CHECK_LE(a, b);

		float step = (b - a) / (n - 1);
		cv::Mat space(1, n, CV_32FC1);

		auto ptr = space.ptr<float>();
		for (int i = 0; i < n; i++)
		{
			ptr[i] = a + i * step;
		}

		return space;
	}
	
	class PlotImpl : public PlotFigure
	{
	public:
		PlotImpl(const std::string& name)
		{
			Figure::name = name.empty() ? "Figure" + std::to_string(idx++) : name;
		}

		virtual void Show() final
		{
			// Refresh
			figure = cv::Scalar(255, 255, 255);
			//cv::rectangle(figure, cv::Rect(roi.tl() - cv::Point(1,1), roi.br() + cv::Point(1,1)), ColorPool::Get(BLACK));
			cv::rectangle(figure, roi, ColorPool::Get(BLACK));
			//cv::Mat curve = figure(roi);

			ShowTitle();
			ShowXLabel();
			ShowYLabel();
			ShowAxis();
			ShowLegend();

			ShowCurves();

			cv::namedWindow(name);
			cv::setMouseCallback(name, OnMouse, this);
			cv::imshow(name, figure);
			cv::imwrite("plot_res.jpg", figure);
		}

		virtual void Save(const std::string& _file) final
		{
			return;
		}

		virtual void Load(const std::string& _file) final
		{
			return;
		}

	private:
		virtual void PushCurve(const cv::Mat& x, const cv::Mat& y) final
		{
			//CHECK_EQ(1, x.rows);
			//CHECK_EQ(1, y.rows);
			//CHECK_EQ(x.cols, y.cols);
			//CHECK_EQ(x.type(), CV_32FC1);
			//CHECK_EQ(y.type(), CV_32FC1);

			if (state == OFF) Clear();

			new_color = new_color == Color(-1) ? ColorPool::Next() : new_color;
			//new_legend = new_legend == "" ? cv::format("L%02d", curves_legend.size()+1).c_str() : new_legend;

			curves_color.push_back(new_color);
			curves_width.push_back(new_width);
			curves_marker.push_back(new_marker);
			curves_legend.push_back(new_legend);

			// Update the range of axis
			UpdateRange(x, y);

			x_data.push_back(x);
			y_data.push_back(y);

			// Reset new args
			new_color = Color(-1);
			new_type = -1;
			new_width = 1;
			new_marker = -1;
			new_legend = "";
		}

		virtual void Parse(Any& any) final
		{
			if (any.Is<const char*>() && args_list.find((const char*)(any)) != args_list.end())
			{
				const char* arg = any;
				local ? ParseLoacl(arg) : ParseGlobal(arg);
			}
			else
			{
				arg_value = any;
			}
		}

		void ShowAxis()
		{
			cv::Mat curve = figure(roi);

			// If the range is very short, reset the range to 1
			if (x_range[1] - x_range[0] < 1e-8) x_range = cv::Vec2f(x_range[0] - 0.5f, x_range[1] + 0.5f);
			if (y_range[1] - y_range[0] < 1e-8) y_range = cv::Vec2f(y_range[0] - 0.5f, y_range[1] + 0.5f);

			// Scale
			double x_scale = pow(10, floor(log10(std::max(abs(x_range[1]), abs(x_range[0])))));
			double y_scale = pow(10, floor(log10(std::max(abs(y_range[1]), abs(y_range[0])))));

			auto font_face = cv::HersheyFonts::FONT_HERSHEY_SIMPLEX;
			float font_scale = 0.3f;
			if (x_scale != 1)
			{
				auto text_size = cv::getTextSize(cv::format("x10^%d", (int)log10(x_scale)), font_face, font_scale, 1, 0);
				cv::putText(figure, cv::format("x10^%d", (int)log10(x_scale)), cv::Point(roi.br().x - text_size.width, roi.br().y + text_size.height + 20),
					font_face, font_scale, ColorPool::Get(BLACK), 1, cv::LINE_AA);
			}
			if (y_scale != 1)
			{
				cv::Mat rotated;
				cv::rotate(figure, rotated, cv::ROTATE_90_CLOCKWISE);
				auto text_size = cv::getTextSize(cv::format("x10^%d", (int)log10(y_scale)), font_face, font_scale, 1, 0);
				cv::putText(rotated, cv::format("x10^%d", (int)log10(y_scale)), cv::Point(figure.rows - roi.y - text_size.width, roi.x - text_size.height - 35),
					font_face, font_scale, ColorPool::Get(BLACK), 1, cv::LINE_AA);
				cv::rotate(rotated, figure, cv::ROTATE_90_COUNTERCLOCKWISE);
			}

			float zero_x = ((0.f - x_range[0]) / (x_range[1] - x_range[0]) * (roi.width - 1.f));
			float zero_y = ((roi.height - 1.f) - (roi.height - 1.f) / (y_range[1] - y_range[0]) * (0.f - y_range[0]));
			cv::line(curve, cv::Point2f(0.f, zero_y), cv::Point2f((float)roi.width, zero_y), ColorPool::Get(GRAY)); // X Axis
			cv::line(curve, cv::Point2f(zero_x, 0.f), cv::Point2f(zero_x, (float)roi.height), ColorPool::Get(GRAY)); // Y Axis

			cv::Mat x_axis_mat = LineSpace(x_range[0], x_range[1], index_cnt);
			cv::Mat y_axis_mat = LineSpace(y_range[0], y_range[1], index_cnt);
			auto x_axis = x_axis_mat.ptr<float>();
			auto y_axis = y_axis_mat.ptr<float>();

			float index_x;
			float index_y;

			for (int i = 0; i < index_cnt; i++)
			{
				// X scale
				index_x = (x_axis[i] - x_range[0]) / (x_range[1] - x_range[0]) * (roi.width - 1.f) + roi.tl().x;
				index_y = roi.br().y - 1.f;

				if (i % 5 == 0)
				{
					cv::line(figure, cv::Point2f(index_x, index_y - index_len), cv::Point2f(index_x, index_y + index_len), ColorPool::Get(BLACK));
					// scale value
					auto text_size = cv::getTextSize(cv::format("%.2f", x_axis[i] / x_scale), font_face, font_scale, 1, 0);
					cv::putText(figure, cv::format("%.2f", x_axis[i] / x_scale),
						cv::Point2f(index_x - text_size.width / 2.f, index_y + text_size.height + index_len),
						font_face, font_scale, ColorPool::Get(BLACK), 1, cv::LINE_AA);
				}
				else
				{
					cv::line(figure, cv::Point2f(index_x, index_y - 5), cv::Point2f(index_x, index_y), ColorPool::Get(BLACK));
				}

				// Y scale
				index_x = (float)roi.tl().x;
				index_y = (roi.height - 1.f) - (roi.height - 1.f) / (y_range[1] - y_range[0]) * (y_axis[i] - y_range[0]) + roi.tl().y;

				if (i % 5 == 0)
				{
					cv::line(figure, cv::Point2f(index_x - index_len, index_y), cv::Point2f(index_x + index_len, index_y), ColorPool::Get(BLACK));
					// scale value
					auto text_size = cv::getTextSize(cv::format("%.2f", y_axis[i] / y_scale), font_face, font_scale, 1, 0);
					cv::putText(figure, cv::format("%.2f", y_axis[i] / y_scale),
						cv::Point2f(index_x - text_size.width - index_len, index_y + text_size.height / 2.f),
						font_face, font_scale, ColorPool::Get(BLACK), 1, cv::LINE_AA);
				}
				else
				{
					cv::line(figure, cv::Point2f(index_x, index_y), cv::Point2f(index_x + 5, index_y), ColorPool::Get(BLACK));
				}
			}
		}
		void ShowXLabel()
		{
			if (!x_label.empty())
			{
				auto font_face = cv::HersheyFonts::FONT_HERSHEY_SIMPLEX;
				float font_scale = 0.4f;
				auto text_size = cv::getTextSize(x_label, font_face, font_scale, 1, 0);
				cv::putText(figure, x_label, cv::Point2f(roi.x + roi.width / 2.f - text_size.width / 2.f, roi.br().y + 35.f),
					font_face, font_scale, ColorPool::Get(BLACK), 1, cv::LINE_AA);
			}
		}
		void ShowYLabel()
		{
			if (!y_label.empty())
			{
				auto font_face = cv::HersheyFonts::FONT_HERSHEY_SIMPLEX;
				float font_scale = 0.4f;
				auto text_size = cv::getTextSize(y_label, font_face, font_scale, 1, 0);
				cv::Mat rotated;
				cv::rotate(figure, rotated, cv::ROTATE_90_CLOCKWISE);
				cv::putText(rotated, y_label, cv::Point2f(rotated.cols - roi.y - roi.height / 2.f - text_size.width / 2.f, roi.x - 50.f),
					font_face, font_scale, ColorPool::Get(BLACK), 1, cv::LINE_AA);
				cv::rotate(rotated, figure, cv::ROTATE_90_COUNTERCLOCKWISE);
			}
		}
		void ShowLegend()
		{
			cv::Rect roi(20, roi.y, 70, roi.height);
			cv::Mat legend = figure(roi);
			float font_scale = 0.5f;
			auto font_face = cv::HersheyFonts::FONT_HERSHEY_SIMPLEX;
			for (int i = 0, k = 0; i < curves_legend.size(); i++)
			{
				if (curves_legend[i].empty())
					continue;

				auto text_size = cv::getTextSize(curves_legend[i], font_face, font_scale, 1, 0);
				float pt_y = (k + 1)*(text_size.height + 15.f) + text_size.height / 2.f;
				float fn_y = (k + 1)*(text_size.height + 15.f) + text_size.height;
				cv::line(legend, cv::Point2f(0.f, pt_y), cv::Point2f(15.f, pt_y), curves_color[i], curves_width[i], cv::LINE_AA);
				if (curves_marker[i] != -1)
					cv::drawMarker(legend, cv::Point2f(7.5f, pt_y), curves_color[i], curves_marker[i], curves_width[i] * 5, curves_width[i], cv::LINE_AA);

				cv::putText(legend, curves_legend[i], cv::Point2f(20.f, fn_y), font_face, font_scale, curves_color[i], 1, cv::LINE_AA);
				k++;
			}
		}
		void ShowCurves()
		{
			cv::Mat curve = figure(roi);

			// Show curve in roi area
			for (int n = 0; n < y_data.size(); n++)
			{
				auto x_ptr = x_data[n].ptr<float>();
				auto y_ptr = y_data[n].ptr<float>();

				std::vector<cv::Point> pts;
				for (int i = 0; i < y_data[n].size().width; i++)
				{
					float x_value = (x_ptr[i] - x_range[0]) / (x_range[1] - x_range[0]) * (roi.width - 1.f);
					float y_value = (roi.height - 1.f) - (roi.height - 1.f) / (y_range[1] - y_range[0]) * (y_ptr[i] - y_range[0]);
					pts.push_back(cv::Point2f(x_value, y_value));

					if (curves_marker[n] != -1)
					{
						cv::drawMarker(curve, pts.back(), curves_color[n], curves_marker[n], curves_width[n] * 5, curves_width[n], cv::LINE_AA);
					}
				}
				// Here just support Point2i in newer version // ??
				cv::polylines(curve, pts, false, curves_color[n], curves_width[n], cv::LINE_AA);
			}
		}

		void Clear()
		{
			x_range[0] = y_range[0] = FLT_MAX;
			x_range[1] = y_range[1] = -FLT_MAX; //FLT_MIN;

			std::vector<cv::Mat>().swap(x_data);
			std::vector<cv::Mat>().swap(y_data);

			std::vector<Color>().swap(curves_color);
			std::vector<int>().swap(curves_width);
			std::vector<int>().swap(curves_marker);
			std::vector<std::string>().swap(curves_legend);
		}

		virtual void ParseLoacl(const char* arg) final
		{
			switch (Hash(arg))
			{
			case "LineWidth"_hash:
				new_width = arg_value;
				//new_width = std::any_cast<int>(arg_value);
				break;
			case "Color"_hash:
				//new_color = std::any_cast<Color>(arg_value);
				new_color = arg_value;
				break;
			case "Marker"_hash:
				new_marker = (cv::MarkerTypes)(arg_value);
				break;
			case "Legend"_hash:
				new_legend = (const char*)(arg_value);
				break;
			default:
				//LOG(WARNING) << "Unknown arg " << arg;
				break;
			}
		}
		virtual void ParseGlobal(const char* arg) final
		{
			switch (Hash(arg))
			{
			case "Title"_hash:
				title = (const char*)arg_value;
				break;
			case "XLabel"_hash:
				x_label = (const char*)arg_value;
				break;
			case "YLabel"_hash:
				y_label = (const char*)arg_value;
				break;
			case "XRange"_hash:
				x_range = arg_value;
				break;
			case "YRange"_hash:
				y_range = arg_value;
				break;
			default:
				//LOG(WARNING) << "Unknown arg " << arg;
				break;
			}
		}

		void UpdateRange(const cv::Mat& x, const cv::Mat& y)
		{
			// Update the range of axis
			double min_x, max_x;
			cv::minMaxIdx(x, &min_x, &max_x);
			x_range[0] = min_x < x_range[0] ? (float)min_x : x_range[0];
			x_range[1] = max_x > x_range[1] ? (float)max_x : x_range[1];

			double min_y, max_y;
			cv::minMaxIdx(y, &min_y, &max_y);

			y_range[0] = min_y < y_range[0] ? (float)min_y : y_range[0];
			y_range[1] = max_y > y_range[1] ? (float)max_y : y_range[1];
		}

		static void OnMouse(int event, int x, int y, int flags, void* user_data)
		{
			auto figure = static_cast<PlotImpl*>(user_data);

			if (figure->figure.empty()) return;

			cv::Point pt(x, y);

			cv::Mat on_mouse = figure->figure.clone();

			if (pt.inside(figure->roi))
			{
				auto font_face = cv::HersheyFonts::FONT_HERSHEY_SIMPLEX;
				float font_scale = 0.35f;

				auto x_value = (x - figure->roi.x) * (figure->x_range[1] - figure->x_range[0]) / (figure->roi.width - 1) + figure->x_range[0];

				std::vector<int> idx;
				// Search nearest point idx
				for (auto x : figure->x_data)
				{
					int min_idx[2]; // 1
					cv::minMaxIdx(cv::abs(x - x_value), nullptr, nullptr, min_idx);
					idx.push_back(min_idx[1]);
				}

				cv::Rect value_rect = cv::Rect(10, figure->roi.y + figure->roi.height / 2 + 50, 100, figure->roi.height / 2);

				for (size_t i = 0; i < idx.size(); i++)
				{
					float x_value = (figure->x_data[i].at<float>(0, idx[i]) - figure->x_range[0]) / (figure->x_range[1] - figure->x_range[0]) * (figure->roi.width - 1);
					float y_value = (figure->roi.height - 1) - (figure->roi.height - 1) / (figure->y_range[1] - figure->y_range[0]) * (figure->y_data[i].at<float>(0, idx[i]) - figure->y_range[0]);
					cv::circle(on_mouse(figure->roi), cv::Point2f(x_value, y_value), 5, figure->curves_color[i], -1);

					// Show Text
					std::string value = cv::format("(%.3g,%.3g)", figure->x_data[i].at<float>(0, idx[i]), figure->y_data[i].at<float>(0, idx[i]));
					auto text_size = cv::getTextSize(value, font_face, font_scale, 1, 0);

					cv::putText(on_mouse(value_rect), value, cv::Point2f(5, (i + 1)*(text_size.height + 15.f) + text_size.height),
						font_face, font_scale, figure->curves_color[i], 1, cv::LINE_AA);
				}
			}

			cv::imshow(figure->name, on_mouse);
		}

		std::set<std::string> args_list = { "LineWidth", "Color", "Marker", "Legend", "Title", "XLabel", "YLabel", "XRange", "YRange" };

		std::vector<Color> curves_color;
		std::vector<int> curves_width;
		std::vector<int> curves_marker;
		std::vector<std::string> curves_legend;

		Color new_color = Color(-1);
		int new_type = -1;
		int new_width = 1;
		int new_marker = -1;
		std::string new_legend = "";

		cv::Vec2f x_range = cv::Vec2f(FLT_MAX, -FLT_MAX);
		cv::Vec2f y_range = cv::Vec2f(FLT_MAX, -FLT_MAX);

		std::vector<cv::Mat> y_data;
		std::vector<cv::Mat> x_data;

		std::string x_label;
		std::string y_label;

		int index_cnt = 101;
		int index_len = 5;
	};

	std::shared_ptr<PlotFigure> PlotFigure::Figure(const std::string& name)
	{
		return std::shared_ptr<PlotFigure>(new PlotImpl(name));
	}
}