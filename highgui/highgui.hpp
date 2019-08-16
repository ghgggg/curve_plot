#pragma once

#include "core.h"

namespace chaos
{
	
	class CHAOS_API Base
	{
	public:
		/// <summary>To set args</summary>
		template<class ... Args>
		void Set(Args ... args)
		{
			DummyWrap(Unpack(args)...);
		}

		virtual void Save(const std::string& file) {}
		virtual void Load(const std::string& file) {}

	protected:
		virtual void Parse(Any& any) = 0;

		template <class ... Args>
		void DummyWrap(Args... args) {}

		template <class Arg>
		Arg& Unpack(Arg& arg)
		{
			Any any = arg;
			Parse(any);
			return arg;
		}

		Any arg_value;
	};
	using Color = cv::Scalar;
	enum ColorName
	{
		BLACK = 0,
		BLUE = 2,
		CYAN = 8,
		GRAY = 13,
		GREEN = 3,
		LIME = 6,
		MAGENTA = 20,
		MAROON = 9,
		NAVY = 1,
		OLIVE = 12,
		PURPLE = 10,
		RED = 18,
		TEAL = 4,
		WHITE = 26,
		YELLOW = 24,
	};

	class CHAOS_API ColorPool
	{
	public:
		static Color Get(int idx);
		static Color Next();

	private:
		static std::vector<Color> pool;
		static int idx;
	};

	enum State
	{
		OFF,
		ON,
	};

	class CHAOS_API Figure : public Base
	{
	public:
		void Hold(State state);
		virtual void Show() = 0;

	protected:
		virtual void ShowTitle();
			
		virtual void ParseLoacl(const char* arg) = 0;
		virtual void ParseGlobal(const char* arg) = 0;

		static int idx;
		std::string name;
		std::string title;

		State state;

		cv::Mat figure = cv::Mat(600, 800, CV_8UC3, cv::Scalar(255, 255, 255));
		const cv::Rect roi = cv::Rect(140, 60, 640, 480);

		bool local = false; // loacl flag for args
	};

}