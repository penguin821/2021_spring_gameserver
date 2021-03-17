#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace chrono;

int main()
{
	auto start_time = system_clock::now();

	long long sum = 0;
	for (int i = 0; i <= 10'000'000; i++)
	{
		sum += i;
		//this_thread::sleep_for(0ms); // 아무일도 안하는 시스템 콜이라도 200배 시간 차이남
		this_thread::yield(); // 이 스레드 할거 끝났으니까 순서 양도해라, 양도할 스레드 없으면 얘가 실행

	}
	auto end_time = system_clock::now();
	auto exec_time = end_time - start_time;

	cout << "Sum = " << sum << endl;

	cout << "exec time = " << duration_cast<milliseconds>(exec_time).count() << endl;
}