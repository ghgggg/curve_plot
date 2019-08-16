#include"imagesc.hpp"

namespace chaos
{
	class ImagescImpl : public ImagescFigure
	{
	public:
		ImagescImpl(const std::string name)
		{
			Figure::name = name.empty() ? "Figure" + std::to_string(idx++) : name;
		}

		virtual void Show() final
		{
			figure = cv::Scalar(255, 255, 255);
			cv::rectangle(figure, cv::Rect(roi.tl() - cv::Point(1, 1), roi.br() + cv::Point(1, 1)), ColorPool::Get(BLACK));

			ShowTitle();

			ShowImage();
			ShowMapping();

			cv::namedWindow(name);
			cv::imshow(name, figure);
		}

		virtual void Save(const std::string& file) final
		{

		}

		virtual void Load(const std::string& file) final
		{

		}

	private:
		virtual void PushImage(const cv::Mat& data) final
		{
			//CHECK_EQ(2, data.dims);
			//CHECK_EQ(1, data.channels());
			//CHECK_EQ(CV_32F, data.depth()); // DO not limit the depth of the input data
			
			data.convertTo(gray, CV_32F); // Convert to float

			double min, max;
			cv::minMaxIdx(gray, &min, &max);
			range[0] = (float)min;
			range[1] = (float)max;

			cv::normalize(gray, gray, 0, 255, cv::NORM_MINMAX);
			gray.convertTo(gray, CV_8U);
		}

		void ShowImage()
		{
			cv::Mat image = figure(roi);

			// Resize to roi
			auto scale = (float)gray.rows / gray.cols > (float)roi.height / roi.width ? 
				roi.height / (float)gray.rows : roi.width / (float)gray.cols;
			
			cv::Mat mapped_image;
			cv::resize(gray, mapped_image, cv::Size(), scale, scale);

			color_map == MGRAY ? cv::cvtColor(mapped_image, mapped_image, cv::COLOR_GRAY2BGR) : cv::applyColorMap(mapped_image, mapped_image, (int)color_map);

			float x = (roi.width - mapped_image.cols) / 2.f;
			float y = (roi.height - mapped_image.rows) / 2.f;

			mapped_image.copyTo(image(cv::Rect((int)x, (int)y, mapped_image.cols, mapped_image.rows)));
		}
		void ShowMapping()
		{
			cv::rectangle(figure, cv::Rect(44, 89, 52, 258), cv::Scalar());

			cv::Mat legend = cv::Mat::zeros(256, 50, CV_8U);
			for (int i = 0; i < 256; i++)
			{
				legend.row(i) += (255 - i);
			}

			color_map == MGRAY ? cv::cvtColor(legend, legend, cv::COLOR_GRAY2BGR) : cv::applyColorMap(legend, legend, (int)color_map);
			legend.copyTo(figure(cv::Rect(45, 90, 50, 256)));
			
			// Show Range

			std::string text;
			cv::Size text_size;
			float font_scale = 0.5f;
			auto font_face = cv::HersheyFonts::FONT_HERSHEY_SIMPLEX;

			text = cv::format("%g", range[1]);
			text_size = cv::getTextSize(text, font_face, font_scale, 1, 0);
			cv::putText(figure, text, cv::Point2f(70.f - text_size.width/2.f, 90.f - text_size.height), font_face, font_scale, cv::Scalar(), 1, cv::LINE_AA);

			text = cv::format("%g", range[0]);
			text_size = cv::getTextSize(text, font_face, font_scale, 1, 0);
			cv::putText(figure, text, cv::Point2f(70.f - text_size.width / 2.f, 346.f + 2*text_size.height), font_face, font_scale, cv::Scalar(), 1, cv::LINE_AA);
		}

		virtual void Parse(const std::any& any) final
		{
			if (any.type() == typeid(const char*) && args_list.find(std::any_cast<const char*>(any)) != args_list.end())
			{
				const char* arg = std::any_cast<const char*>(any);
				local ? ParseLoacl(arg) : ParseGlobal(arg);
			}
			else
			{
				arg_value = any;
			}
		}

		virtual void ParseLoacl(const char* arg) final
		{
			switch (Hash(arg))
			{
			case "ColorMap"_hash:
				color_map = (ColorMap)(arg_value);
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
				title = (const char*)(arg_value);
				break;
			default:
				//LOG(WARNING) << "Unknown arg " << arg << " for Set";
				break;
			}
		}

		std::set<std::string> args_list = {"ColorMap", "Title"};

		ColorMap color_map = MGRAY;
		cv::Mat gray;

		cv::Vec2f range;
	};

	std::shared_ptr<ImagescFigure> ImagescFigure::Figure(const std::string& name)
	{
		return std::shared_ptr<ImagescFigure>(new ImagescImpl(name));
	}

} // namespace chaos