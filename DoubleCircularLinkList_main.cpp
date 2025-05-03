#include"DoubleCircularLinkList.h"



int main()
{
	std::ios::sync_with_stdio(false), std::cout.tie(0);
	DoubleCircularLinkList<int> list;
	list.insert_from_head(10);
	list.insert_from_tail(30);
	list.insert(2, 20);
	list.print();
	list.update(3, 60);
	//list.remove(2);
	//list.remove_from_head();
	//list.remove_from_tail();
	list.print();
	std::cout << list.get_head_value() << std::endl;
	std::cout << list.get_tail_value() << std::endl;
	std::cout << list.get_node_value(2) << std::endl;
	return 0;
}
