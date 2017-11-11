#include <iostream>
#include <string>
#include <vector>

#ifdef DEBUG
#define DEBUGINFO(x, y, str) std::cout<<"stage"<<(x)<< (x==y ? " remains" : " jumps to")<< \
    " stage"<<(y)<<'\t'<<"current sub string:"<<str<<std::endl
#endif
#ifndef DEBUG
#define DEBUGINFO(x,y,z)
#endif

using std::string;
using std::vector;
vector<string> Split(string str);

vector<string> Split(string str)
{
  vector<string> str_list;
  string sub_str = string();
  string::size_type i = 0;
  int state = 0;
  while(i < str.size())
  {
    switch(state)
    {
      case 0:
        if (str[i] == 'a')
        {
          sub_str += "a";
          state = 1;
          DEBUGINFO(0, 1, sub_str);
        }
        else if (str[i] == 'b')
        {
          sub_str += "b";
          state = 2;
          DEBUGINFO(0, 2, sub_str);
        }
        else
          std::cerr << "Error: undefined character:" << str[i] << std::endl;
        break;
      case 1:
        if (str[i] == 'a')
        {
          sub_str += "a";
          DEBUGINFO(1, 1, sub_str);
        }
        else
        {
          str_list.push_back(sub_str);
          sub_str.clear();
          state = 0;
          i--;
          DEBUGINFO(1, 0, sub_str);
        }
        break;
      case 2:
        if (str[i] == 'b')
        {
          sub_str += "b";
          DEBUGINFO(2, 2, sub_str);
        }
        else
        {
          str_list.push_back(sub_str);
          sub_str.clear();
          state = 0;
          i--;
          DEBUGINFO(2, 0, sub_str);
        }
        break;
    }
    i++;
  }
  if (!sub_str.empty())
    str_list.push_back(sub_str);
  return str_list;
}

int main()
{
  std::cout << "Input the string you want to search for aa*|bb* inside (Input \"end\" to terminate)" << std::endl;
  string a = string();
  while(true)
  {
    std::cin >> a;
    if (a == "end")
      break;
    else
    {
      vector<string> str_list = Split(a);
      std::cout <<"The output is: ";
      for(auto it = str_list.begin(); it != str_list.end(); ++it)
        std::cout << *it << "  ";
      std::cout << std::endl;
    }
  }
  return 0;
}
