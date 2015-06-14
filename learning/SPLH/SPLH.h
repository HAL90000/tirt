#pragma once

#include "Def.h"

#include <Eigen/Eigenvalues> 
#include <vector>

#include <iostream>
#include <algorithm>
#include <cmath>

Matrix T(Matrix Sk, Matrix S)
{
	for (int i = 0; i < S.rows(); ++i)
		for (int j = 0; j < S.cols(); ++j)
			if (S(i, j) * Sk(i, j) >= 0)
				Sk(i, j) = 0;
	return Sk;
}

std::vector<Matrix> SPLH(Matrix X, Matrix Xl, Matrix S, int K, double alpha, double eta)
{
	std::vector<Matrix> x;

	//std::vector<int> E;
	//E.resize(N);
	//for (int i = 0; i < N; ++i)
	//	E[i] = i;
	//std::sort(E.begin(), E.end(), [&](int a, int b){return abs(solver.eigenvalues()(a)) > abs(solver.eigenvalues()(b)); });

	for (int i = 0; i < K; ++i)
	{
		std::clog << "\t:: " << i << std::endl;

		//std::clog << solver.eigenvalues().row(E[i]) << std::endl;

		//x.push_back(solver.eigenvectors().col(E[i]));

		Matrix M = Xl * S * Xl.transpose() + eta * X * X.transpose();

		Eigen::SelfAdjointEigenSolver<Matrix> solver(M);

		int j = solver.eigenvectors().rows() - 1;

		Matrix w = solver.eigenvectors().col(j);

		std::clog << solver.eigenvalues().row(j) << std::endl;
		x.push_back(w);

		S -= alpha * T(Xl.transpose()*w*w.transpose()*Xl, S);
		X -= w * w.transpose() * X;
		Xl -= w * w.transpose() * Xl;
	}

	return x;
}