
#include <iostream>
#include <future>


class Work {
private:

  int value ;

  int get_value() const { return value ; }

public:

  Work() : value(42) {}
  ~Work() { value = 0 ; }

  std::future<int> spawn()
  {
    return std::async( [=]() -> int { return value ; } );
  }

  std::future<int> spawn_2()
  {
    // Capture with initializer only for C++14
    return std::async( [=,self=*this]() -> int { return self.get_value() ; } );
  }
};

std::future<int> foo()
{
  Work w ;
  return w.spawn();
}

std::future<int> foo_2()
{
  Work w ;
  return w.spawn_2();
}


int main()
{
  if ( 1 ) {
    Work tmp ;
    std::future<int> f = tmp.spawn();
    std::cout << f.get() << std::endl ;
  }

  if ( 1 ) {
    std::future<int> f = foo();
    std::cout << f.get() << std::endl ;
  }

  if ( 1 ) {
    std::future<int> f = foo_2();
    std::cout << f.get() << std::endl ;
  }

  return 0 ;
}



