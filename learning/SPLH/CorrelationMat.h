#include <Eigen\Dense>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

template <typename Matrix = Eigen::MatrixXf>
class SubCorrMatrix
{

public:

	SubCorrMatrix(int D) :
		sumX(Matrix::Zero(D, 1)),
		XX(Matrix::Zero(D, D)),
		N(0),
		D(D)
	{
	}

	void AddX(const Matrix &X)
	{
		sumX += X;
		XX += X*X.transpose();
		N += 1;
	}

	Matrix CorrelationMatrix() const
	{
		return XX - sumX*sumX.transpose() / N;
	}

	Matrix Mean() const
	{
		return sumX / N;
	}

	SubCorrMatrix operator+(const SubCorrMatrix &m) const
	{
		SubCorrMatrix result;

		result.N = N + m.N;
		result.XX = XX + m.XX;
		result.sumX = sumX + m.XX;
		result.D = D;

		return result;
	}

	SubCorrMatrix operator+=(const SubCorrMatrix &m)
	{
		N += m.N;
		XX += m.XX;
		sumX += m.XX;

		return *this;
	}

private:
	SubCorrMatrix()
	{
	}

	Matrix sumX;
	Matrix XX;
	int N;
	int D;

};

template <typename Matrix>
class Camp
{
	struct Worker
	{
		SubCorrMatrix<Matrix> subMatrix;
		Camp &camp;
		bool end;
		std::queue<Matrix> x;

		void operator()()
		{
			while (!end)
			{
				{
					std::lock_guard<std::mutex> lock(camp.m);

					while (!end && camp.tasks.empty())
						camp.cv.wait(camp.m);

					if (!end)
					{
						for (int i = 0; i < 100 && !camp.tasks.empty();++i)
						{
							x.push(camp.tasks.front());
							camp.tasks.pop();
						}
					}
				}
				if (!end)
					while (!x.empty())
					{
						subMatrix.AddX(x.front());
						x.pop();
					}

			}
		}
	};


public:
	Camp(int N)
	{
		workers.resize(N, { SubCorrMatrix<Matrix>(32*32*3), *this, false});
		threads.resize(N);
		for (int i = 0; i < threads.size(); ++i)
		{
			threads[i] = std::thread(workers[i]);
		}
	}

	void finish(SubCorrMatrix<Matrix> &subMatrix)
	{
		for (int i = 0; i < workers.size(); ++i)
			workers[i].end = true;

		cv.notify_all();

		for (int i = 0; i < threads.size(); ++i)
			threads[i].join();

		for (int i = 0; i < workers.size(); ++i)
			subMatrix += workers[i].subMatrix;
	}

	void AddX(Matrix x)
	{
		std::lock_guard<std::mutex> lock(m);

		tasks.push(x);
		cv.notify_one();
	}

	int stat()
	{
		std::lock_guard<std::mutex> lock(m);
		int v = tasks.size();

		return v;
	}

private:
	std::mutex m;
	std::condition_variable_any cv;
	std::vector<Worker> workers;
	std::vector<std::thread> threads;
	std::queue<Matrix> tasks;
};

template <typename Matrix>
void LoadBatch(std::istream &input, SubCorrMatrix<Matrix> &subMatrix)
{
	std::vector<unsigned char> data;
	int D = 32 * 32 * 3;
	data.resize(1 + D);

	Camp<Matrix> camp(4);

	Matrix img = Matrix::Zero(D, 1);

	input.peek();
	while (!input.eof())
	{
		input.read((char*)data.data(), data.size());

		for (int i = 1; i < data.size(); ++i)
			img(i - 1, 0) = data[i] / 255.0;

		//subMatrix.AddX(img);
		camp.AddX(img);

		input.peek();
	}

	bool end = false;
	while (!end)
	{
		int v = camp.stat();
		if (v == 0)
		{
			end = true;
		}
		std::cout << v << std::endl;

		std::chrono::nanoseconds duration(1000000000);
		std::this_thread::sleep_for(duration);
	}

	camp.finish(subMatrix);
}