#include<iostream>

using namespace std;

int main(int argc, char* argv[], char* env[])
{
  cout<<"start execution"<<endl;
  int i = 0;

  cout<<endl<<"argv:"<<endl;
  for(i = 0; argv[i]; i++)
  {
    cout<<i<<"->"<<argv[i]<<endl;
  }

  cout<<endl<<"env:"<<endl;
  for(i = 0; env[i]; i++)
  {
    cout<<i<<"->"<<env[i]<<endl;
  }

  cout<<endl<<"execution finshed"<<endl;
  return 0;
}
