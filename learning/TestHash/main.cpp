#include <iostream>
#include <vector>
#include <fstream>
#include <Eigen\Dense>
#include <Image.h>

int main(int argc, char **argv)
{
	std::clog << "START" << std::endl;
	HashBase base;
	
	std::clog << "Loading hash base... " << std::endl;
	{
		std::clog << "\t" << argv[1] << std::endl;
		std::ifstream input(argv[1]);
		base.Load(input);
	}
	std::clog << "DONE" << std::endl;

	std::cout << "size = " << base.size() << std::endl;

	for (int i = 0; i < 100; ++i)
		base.Find(rand());

	return 0;
}