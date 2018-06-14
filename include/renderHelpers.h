#include <string>

struct renderWindow
{
	int x;
	int y;
	int w;
	int h;
	bool visible;
	bool resize;
	std::string name;

	void set(int x_, int y_, int w_, int h_, bool visible_, bool resize_, std::string name_)
	{
		x = x_; y = y_; w = w_; h = h_; visible = visible_; resize = resize_, name = name_;
	}


};

struct anchorPoint
{
	int x;
	int y;
};