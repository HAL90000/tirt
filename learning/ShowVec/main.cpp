#include <iostream>
#include <vector>
#include "WinBitmap.h"

#include <algorithm>
#include <fstream>

void Save(std::vector<float> &v, tstring file)
{
	if (v.size() < 32 * 32 * 3)
		return;


	float max = -1e55;
	float min = 1e55;
	for (float &x : v)
	{
		min = std::min(min, x);
		max = std::max(max, x);
	}

	for (float &x : v)
	{
		x -= min;
		x /= max - min;
		x *= 255;
	}

	WinBitmap bmp;

	HDC hDC = GetDC(GetDesktopWindow());
	bmp.Alloc(32, 32, NULL);

	for (int y = 0; y < 32; ++y)
		for (int x = 0; x < 32; ++x)
		{
			bmp.Pixel(x, y) = TColorBGR((BYTE)v[y * 32 + x], (BYTE)v[y * 32 + x + 1024], (BYTE)v[y * 32 + x + 2048]);
		}

	bmp.Save(file, TEXT("png"));
}

int main(int argc, char **argv)
{
	std::ifstream file(argv[1]);

	int i = 0;

	file.peek();
	while (!file.eof())
	{
		std::vector<float> v;
		std::string line;
		std::getline(file, line);
		std::istringstream ss(line);

		float offset;
		ss >> offset;
		for (int i = 0; i < 32 * 32 * 3; ++i)
		{
			float x;
			ss >> x;
			v.push_back(x);
		}

		std::wstring output = FROM_UTF8(argv[2]);

		tstringstream filename;
		filename << output << i << L".png";
		Save(v, filename.str());

		++i;
		file.peek();
	}
}