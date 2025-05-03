#include<iostream>
#include<ctime>
#include"my_memory_pool.h"


using namespace std;
using namespace my_memory_pool;
const size_t ELEMS1 = 500;
const size_t ELEMS2 = 100000;

int main()
{
	ios::sync_with_stdio(false), cout.tie(0);
	clock_t start = clock();
	memory_pool<size_t> pool;
	
	for(int i = 0;i < ELEMS2;i++)
	{
		for(int j = 0;j < ELEMS1;j++)
		{
			size_t* temp = pool.allocate();
			pool.deallocate(temp);
		}
	}
	cout << "my_memory_pool内存池耗时：";
	cout << (((double)clock() - start) / CLOCKS_PER_SEC) << endl << endl;

	start = clock();
	for(int i = 0;i < ELEMS2;i++)
	{
		for(int j = 0;j < ELEMS1;j++)
		{
			size_t* temp = new size_t;
			delete temp;
		}
	}
	cout << "new / delete动态内存耗时：";
	cout << (((double)clock() - start) / CLOCKS_PER_SEC) << endl;
	return 0;
}

