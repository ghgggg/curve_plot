
#include "highgui/highgui.hpp"
#include "highgui/plot.hpp"
#include <fstream>
#include <regex>


using namespace chaos;

std::vector<std::vector<float>> parse_logfile(std::string path);
void help()
{
	std::cout << "usage:\nexefile [-stem] FILEPATH [-dream] FILEPATH [-max_xrange] float \n";
	std::cout << "[-stem] : stem log file path, eg: D:\\1\\stem.log \n";
	std::cout << "[-dream] : dream log file path, eg: D:\\1\\dream.log \n";
	std::cout << "[-max_xrange] : x asis max x value defalut: 0.01\n";
}
std::map<std::string, std::string> parsecommandline(char** cmdline, int n)
{
	std::map<std::string, std::string> parseres;
	for (int i = 1; i < n; i++)
	{
		std::string stem = cmdline[i];
		int pos1 = stem.rfind("=");
		if (pos1 == std::string::npos) continue;
		std::string key = stem.substr(0, pos1);
		std::string value = stem.substr(pos1 + 1);
		if (key == "-help")
		{
			help();
			return parseres;
		}
		else if (key == "-stem")
		{
			parseres.insert(std::pair<std::string, std::string>(key, value));
			//continue;
		}
		else if (key == "-dream")
		{
			parseres.insert(std::pair<std::string, std::string>(key, value));
			//continue;
		}
		else if (key == "-max_xrange")
		{
			parseres.insert(std::pair<std::string, std::string>(key, value));
		}
	}
	return parseres;
}
int main(int argc, char** argv)
{
	std::map<std::string, std::string> parseres;
	if (argc < 3)
	{
		std::cout << "no special paths of dream and stem log files found.\n";
		help();
		return 0;
	}
	else
	{
		parseres = parsecommandline(argv, argc);
	}
	if (parseres.size() == 0)
	{
		help();
		return 0;
	}
	std::string strstem, strdream;
	float max_xrange = 0.01;

	for (auto t : parseres)
	{
		if (t.first == "-stem")
		{
			strstem = t.second;
			//continue;
		}
		else if (t.first == "-dream")
		{
			strdream = t.second;
			//continue;
		}
		else if (t.first == "-max_xrange")
		{
			max_xrange = atof(t.second.c_str());

		}
	}


	auto res = parse_logfile(strstem);
	auto res1 = parse_logfile(strdream);
	//auto res = parse_logfile("D:/profile-frontal-faceRecognition/20190812_STEM_100w.log");
	//auto res1 = parse_logfile("D:/profile-frontal-faceRecognition/20190812_DREAM_100w.log");;
	if (res.size()==0 || res1.size() == 0)
	{
		std::cout << "parse log file failed.\n";
		abort();

	}


	cv::Mat x1(res[2]); x1 = x1.reshape(0, 1);
	cv::Mat y1(res[1]); y1 = y1.reshape(0, 1);
	cv::Mat x2(res1[2]); x2 = x2.reshape(0, 1);
	cv::Mat y2(res1[1]); y2 = y2.reshape(0, 1);
	//std::cout << x1;
	auto figure = PlotFigure::Figure();
	
	figure->Plot(x1, y1, "LineWidth", 2, "Color", ColorPool::Get(LIME), "Legend", "STEM");
	
	figure->Hold(ON);
	figure->Plot(x2, y2, "LineWidth", 2, "Color", ColorPool::Get(RED), "Legend", "DREAM");
	figure->Set("YLabel", "Tpr", "XLabel", "Fpr", "Title", "ROC", "XRange", cv::Vec2f(0, max_xrange));
	//figure->Set("YLabel", "Tpr", "XLabel", "Fpr", "Title", "ROC", "XRange", cv::Vec2f(0, 1));
	figure->Show();
	cv::waitKey(3000);
	//getchar();
	

//	cv::Mat x = (cv::Mat_<float>(1, 9) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
//	cv::Mat y1 = (cv::Mat_<float>(1, 9) << 3, 2, 1, 6, 4, 3, 7, 8, 4);
//	cv::Mat y2 = (cv::Mat_<float>(1, 9) << 1, 2, 1, 5, 4, 2, 2, 6, 1);0

	
//	figure->Plot(x, y2, "LineWidth", 2, "Color", ColorPool::Get(BLUE), "Legend", "Y2");

	
	

	return 0;
}
std::vector<std::string> Split(const std::string& data, const std::string& delimiter)
{
	std::regex regex{ delimiter };
	return std::vector<std::string> {
		std::sregex_token_iterator(data.begin(), data.end(), regex, -1),
			std::sregex_token_iterator()
	};
}
std::vector<std::vector<float>> parse_logfile(std::string path)
{
	std::cout << path << std::endl;
	std::ifstream file(path, std::ios::in);
	if (!file.is_open()) { std::cout << "can not open file.\n"; return std::vector<std::vector<float>>(); }
	std::string s;
	std::vector<std::string> str;

	while (std::getline(file, s))
	{
		if (s == "")continue;
		if (s[0] == '*')continue;
		if (s.c_str()[0] == '=' ) break;
		//std::cout << s << std::endl;
		str.push_back(s);
	}
	file.close();
	std::vector<float> r[3]; // thr tpr fpr
	
	for (auto s : str)
	{
		auto res = Split(s, " ");
		for (int i = 0; i < res.size();i++)
		{
			auto rr = Split(res[i], ":");
			r[i].push_back(atof(rr[1].c_str()));
		}	
	}
	std::vector<std::vector<float>> res_f{r,r+3};

	return res_f;

}