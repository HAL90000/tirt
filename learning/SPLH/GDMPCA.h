#include "Def.h"

#include <Eigen/Eigenvalues> 
#include <vector>

#include <iostream>
#include <algorithm>
#include <cmath>

std::vector<Matrix> GDMPCA(Matrix A, int r)
{
	int N = A.rows();

	Matrix B = Matrix::Identity(N, N);

	std::vector<Matrix> x;

	for (int i = 0; i < r; ++i)
	{
		std::clog << "\t:: " << i << std::endl;

		Eigen::GeneralizedSelfAdjointEigenSolver<Matrix> solver(A, B);
		/*
		std::cout << "solver.eigenvalues()" << std::endl;
		std::cout << solver.eigenvalues() << std::endl;
		std::cout << "solver.eigenvectors()" << std::endl;
		std::cout << solver.eigenvectors() << std::endl;
		*/

		std::clog << solver.eigenvalues().row(N-1) << std::endl;

		x.push_back(solver.eigenvectors().col(N-1));

		std::clog << x.back().transpose() * x.back() << std::endl;

		Matrix q = B * x.back();
		Matrix qt = Matrix::Identity(N,N) - q * q.transpose();
		A = qt * A * qt;
		B = B * qt;
		x.back().normalize();
	}

	return x;
}


std::vector<Matrix> OrtogonalDeflation(Matrix A, int r)
{
	int N = A.rows();

	Matrix B = Matrix::Identity(N, N);

	std::vector<Matrix> x;

	Eigen::SelfAdjointEigenSolver<Matrix> solver(A);

	//std::vector<int> E;
	//E.resize(N);
	//for (int i = 0; i < N; ++i)
	//	E[i] = i;
	//std::sort(E.begin(), E.end(), [&](int a, int b){return abs(solver.eigenvalues()(a)) > abs(solver.eigenvalues()(b)); });

	for (int i = 0; i < r; ++i)
	{
		std::clog << "\t:: " << i << std::endl;

		//std::clog << solver.eigenvalues().row(E[i]) << std::endl;

		//x.push_back(solver.eigenvectors().col(E[i]));

		int j = N - i - 1;

		std::clog << solver.eigenvalues().row(j) << std::endl;
		x.push_back(solver.eigenvectors().col(j));
	}

	return x;
}