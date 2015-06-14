#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <sstream>

class Image
{
public:
	std::vector<unsigned char> v;
	int label;
};

class Hasher
{
public:
	std::vector<double> v;
	double bias;
};

static std::vector<Image> LoadBatch(std::istream &input)
{
	std::vector<unsigned char> data;
	data.resize(1 + 32 * 32 * 3);

	std::vector<Image> result;

	input.peek();
	while (!input.eof())
	{
		input.read((char*)data.data(), data.size());

		Image img;
		img.label = data[0];
		img.v.resize(3 * 32 * 32);
		for (unsigned int i = 1; i < data.size(); ++i)
			img.v[i - 1] = data[i];

		result.push_back(img);
		input.peek();
	}

	return result;
}

static Hasher LoadHasher(std::istream &input, int size)
{
	Hasher hash;
	hash.v.resize(size);
	input >> hash.bias;
	for (int i = 0; i < size; ++i)
		input >> hash.v[i];

	return hash;
}

static std::vector<Hasher> LoadHashers(std::istream &input)
{
	std::vector<Hasher> hashes;
	int N, size;
	input >> N >> size;

	for (int i = 0; i < N; ++i)
		hashes.push_back(LoadHasher(input, size));

	return hashes;
}


class HashBase
{
public:
	typedef uint64_t T;

	HashBase(int m = 4, int mb = 16, int bitsPerChunk = 1, int maxErr = 1) :
		m(m), mb(mb),
		mBits(m*mb),
		mbMask((T(1) << mb) - T(1)),
		bitsPerChunk(bitsPerChunk),
		maxErr(maxErr)
	{
		chunks.resize(T(1) << mb);
		for (int i=0;i<chunks.size();++i)
			chunks[i].resize(m);

		initMove(0, 0, bitsPerChunk);
	}

	struct Cls
	{
		int cls;
		int hamming;
	};

	void Load(std::istream &input)
	{
		imgs.clear();
		while (!input.eof())
		{
			std::string line;
			std::getline(input, line);
			std::istringstream l(line);
			int cls;
			l >> cls;
			std::string h;
			l >> h;
			T hash = str2hash(h);

			//if (imgs.find(hash) != imgs.end() && cls != imgs[hash])
			//	std::clog << ":: " << cls << " x " << imgs[hash] << " {" << h << "}" << std::endl;
			imgs[hash] = cls;
			insertHash(hash);
		}

		//printf("size: %d\n", imgs.size());
	}

	size_t size()
	{
		return imgs.size();
	}

	Cls Find(T hash)
	{
		Cls result = { -1, 666 };
		T resultHash = 0;

		// /*
		int count = 0;
		for (int j = 0; j<m; ++j)
		{
			T subHash = getSubHash(hash, j);
			for (unsigned int i = 0; i<move.size(); ++i)
			{
				T chunkIndex = subHash ^ move[i];
				for (unsigned int k = 0; k<chunks[chunkIndex][j].size(); ++k)
				{
					T cHash = chunks[chunkIndex][j][k];
					int hmm = hamming(hash, cHash);
					if (hmm < result.hamming)
					{
						result.hamming = hmm;
						resultHash = cHash;
					}
					++count;
				}
			}
		}
		//printf("count:%d\n", count);
		result.cls = imgs[resultHash];

		if (result.hamming > maxErr)
			result.cls = -1;

		return result;
		// */

		/*
		for (std::map<unsigned int,int>::iterator it = imgs.begin(); it != imgs.end(); ++it)
		{
		int hmm = hamming(hash, it->first);
		if (hmm < result.hamming)
		{
		result.hamming = hmm;
		result.cls = it->second;
		}
		}
		return result;
		*/
	}

private:
	T str2hash(const std::string &s)
	{
		T hash = 0;
		for (int i = 0; i<s.length() && i < mBits; ++i)
			if (s[i] == '1')
				hash |= T(1) << i;
		return hash;
	}

	static int hamming(T a, T b)
	{
		return (int)__builtin_popcount(a^b);
	}

	void initMove(T base, int start, int level)
	{
		move.push_back(base);
		if (level <= 0)
			return;

		for (int i = start; i<mb; ++i)
			initMove(base | (T(1) << i), i + 1, level - 1);
	}

	T getSubHash(T hash, int n)
	{
		return (hash >> (n*mb)) & mbMask;
	}

	void insertHash(T hash)
	{
		for (int i = 0; i<m; ++i)
			chunks[getSubHash(hash, i)][i].push_back(hash);
	}


	int m;
	int mb;
	int mBits;
	T mbMask;
	int bitsPerChunk;
	int maxErr;

	std::vector<T> move;
	std::map<T, int> imgs;
	std::vector< std::vector< std::vector<T> > > chunks;
};
