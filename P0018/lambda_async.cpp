
#include <iostream>
#include <future>


class Work {
private:

  int value ;

public:

  Work() : value(42) {}
  ~Work() { value = 0 ; }

  std::future<int> spawn()
  {
    return std::async( [=]() -> int { return value ; } );
  }
};

std::future<int> foo()
{
  Work w ;
  return w.spawn();
}


int main()
{
  std::future<int> f = foo();
  std::cout << f.get() << std::endl ;

  Work tmp ;
  f = tmp.spawn();
  std::cout << f.get() << std::endl ;

  return 0 ;
}



