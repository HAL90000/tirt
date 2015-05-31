#include <iostream>
#include <vector>
#include <fstream>
#include <Windows.h>


#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <Eigen\Dense>
#include <Image.h>


#include "CorrelationMat.h"

#include "GDMPCA.h"

int main(int argc, char **argv)
{
	Eigen::setNbThreads(4);
	std::clog << "Eigen::nbThreads() = " << Eigen::nbThreads() << std::endl;


	std::clog << "Eigen::nbThreads() = " << Eigen::nbThreads() << std::endl;

	std::vector<Image> images;

	std::clog << "Loading data... " << std::endl;

	int M = 32 * 32 * 3;
	SubCorrMatrix<Eigen::MatrixXf> subCorrMat(M);

	//M = 1024;

	for (int i = 1; i < argc; ++i)
	{
		std::clog << "\t" << argv[i] << std::endl;
		std::ifstream input(argv[i], std::ios::binary);
		auto imgs = LoadBatch(input);
		images.insert(images.end(), imgs.begin(), imgs.end());

		//LoadBatch(input, subCorrMat);
	}

	//while (images.size() > 100)
	//	images.pop_back();

	std::clog << "DONE" << std::endl;

	int N = images.size();

	std::clog << "Creating matrix X... ";
	
	Eigen::MatrixXf X(M, N);
	//Eigen::MatrixXf mi = subCorrMat.Mean();
	Eigen::MatrixXf mi = Eigen::MatrixXf::Zero(M, 1);
	
	for (int i = 0; i < M; ++i)
		for (int j = 0; j < N; ++j)
		{
			double v = images[j].v[i] / 255.0f;
			X(i, j) = v;
			mi(i, 0) += v;
		}

	for (int i = 0; i < M; ++i)
		mi(i, 0) /= N;

	for (int i = 0; i < M; ++i)
		for (int j = 0; j < N; ++j)
		{
			X(i, j) -= mi(i,0);
		}
	

	std::clog << "DONE" << std::endl;
	
	std::clog << "Creating matrix S... ";

	//Eigen::MatrixXf S(N, N);
	//for (int i = 0; i < N; ++i)
	//	for (int j = 0; j < N; ++j)
	//		S(i, j) = images[i].label == images[j].label ? 1 : -1;

	std::clog << "DONE" << std::endl;
	
	std::clog << "Creating matrix M... ";

	Eigen::MatrixXf Me(M, M);
	//Eigen::MatrixXf Me = subCorrMat.CorrelationMatrix();
	//Me = X * S * X.transpose();
	//Me += 0.1 * (X * X.transpose());

	Me = X * X.transpose();

	std::clog << "DONE" << std::endl;

	std::clog << "Matrix deflation... " << std::endl;

	int H = 64;

	auto x = OrtogonalDeflation(Me, H);
	//auto x = GDMPCA(Me, 10);

	std::clog << "DONE" << std::endl;

	std::cout << H << " " << M << std::endl;
	for (auto &v : x)
		std::cout << -mi.transpose() * v << "\t" << v.transpose() << std::endl;

	return 0;
}