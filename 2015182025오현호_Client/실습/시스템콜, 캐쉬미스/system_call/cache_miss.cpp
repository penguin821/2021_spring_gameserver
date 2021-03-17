#include <iostream>
#include <chrono>
#include <thread>
#include <stdlib.h>

using namespace std;
using namespace chrono;

constexpr int CACHE_LINE_SIZE = 64;

int main()
{
	for (int i = 0; i < 21; ++i)
	{
		size_t size = (1024 * 1024) << i;
		char* a = (char*)malloc(size);
		if (nullptr == a) break;
		int tmp = 0;
		int index = 0;

		auto start_time = system_clock::now();

		for (int j = 0; j < 100'000'000; ++j)
		{
			tmp += a[index % size];
			index += CACHE_LINE_SIZE * 11;
		}

		auto end_time = system_clock::now();
		auto exec_time = end_time - start_time;

		cout << "size = " << (1 << i) << "MBytes : " << tmp << "  ";

		cout << "exec time = " << duration_cast<milliseconds>(exec_time).count() << endl;

		free(a);
	}
}