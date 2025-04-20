#include<vector>
#include<iostream>

// SGI vector storage allocation mechanism
int main()
{
  size_t sz;
  std::vector<int> v;
  sz = v.capacity();
  std::cout << "making v grow:\n";
  for (int i = 0; i < 100; ++i)
  {
	  v.push_back(i);
	  if (sz != v.capacity())
	  {
		  sz = v.capacity();
		  std::cout << "capacity changed: " << sz << '\n';
	  }
  }
  return 0;
}
