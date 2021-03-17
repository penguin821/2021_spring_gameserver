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
		//this_thread::sleep_for(0ms); // �ƹ��ϵ� ���ϴ� �ý��� ���̶� 200�� �ð� ���̳�
		this_thread::yield(); // �� ������ �Ұ� �������ϱ� ���� �絵�ض�, �絵�� ������ ������ �갡 ����

	}
	auto end_time = system_clock::now();
	auto exec_time = end_time - start_time;

	cout << "Sum = " << sum << endl;

	cout << "exec time = " << duration_cast<milliseconds>(exec_time).count() << endl;
}