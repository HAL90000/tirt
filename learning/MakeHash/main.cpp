#include <iostream>
#include <vector>
#include <fstream>
#include <Eigen\Dense>
#include <Image.h>

double mult(const Image &img, const Hasher &hasher)
{
	size_t size = std::min(img.v.size(), hasher.v.size());
	double result = hasher.bias;
	for (int i = 0; i < size; ++i)
		result += img.v[i] / 255.0 * hasher.v[i];

	return result;
}

std::string hash(const Image &img, const std::vector<Hasher> &hashers)
{
	std::string h;

	for (int i = 0; i < hashers.size(); ++i)
	{
		double p = mult(img, hashers[i]);

		if (p > 0)
			h.push_back('1');
		else
			h.push_back('0');
	}

	return h;
}

std::string hashTab(const Image &img, const std::vector<Hasher> &hashers)
{
	std::string h;

	for (int i = 0; i < hashers.size(); ++i)
	{
		double p = mult(img, hashers[i]);

		if (p > 0)
			h.push_back('1');
		else
			h.push_back('0');
		h.push_back('\t');
	}

	return h;
}

int main(int argc, char **argv)
{
	Eigen::setNbThreads(4);
	std::clog << "Eigen::nbThreads() = " << Eigen::nbThreads() << std::endl;


	std::clog << "Eigen::nbThreads() = " << Eigen::nbThreads() << std::endl;

	std::vector<Image> images;

	std::clog << "Loading data... " << std::endl;

	for (int i = 2; i < argc; ++i)
	{
		std::clog << "\t" << argv[i] << std::endl;
		std::ifstream input(argv[i], std::ios::binary);
		auto imgs = LoadBatch(input);
		images.insert(images.end(), imgs.begin(), imgs.end());
	}

	//while (images.size() > 2)
	//	images.pop_back();

	std::clog << "DONE" << std::endl;

	int N = (int)images.size();
	int M = 32 * 32 * 3;
	//int M = 1024;

	std::clog << "Loading hashers... " << std::endl;

	std::vector<Hasher> hashers;
	{
		std::clog << "\t" << argv[1] << std::endl;
		std::ifstream input(argv[1]);
		hashers = LoadHashers(input, 64);
	}

	std::clog << "DONE" << std::endl;
	/*
	for (const auto &hash : hashers)
	{
		double bias = 0;
		for (const auto& img : images)
		{
			for (int i = 0; i < img.v.size(); ++i)
				bias += img.v[i] / 255.0 * hash.v[i];
		}
		bias /= images.size();
		bias = -bias;
		std::clog << hash.bias << " _ " << bias << "(" << (abs(bias - hash.bias) < 0.001) << ")" << std::endl;
	}
	*/
	for (const auto& img : images)
	{
		//if (img.label == 255)
		//	continue;
		std::string h = hash(img, hashers);
		std::cout << img.label << "\t" << h << std::endl;
	}

	return 0;
}