
#include <iostream>
#include <vector>
#include <utility>

#define print_sig(msg) {std::cout << __FILE__ << ':' << __LINE__ << ':' << __PRETTY_FUNCTION__ << ": " << msg << std::endl;}
#define print_sig_mt(msg) {std::cout << __FILE__ << ':' << __LINE__ << ':' << __PRETTY_FUNCTION__ << " handle=" << number << ": " << msg << std::endl;}

class movable_type {
public:
    movable_type(int num) : number(num) { print_sig_mt("ctor with one arg"); }
    movable_type(movable_type const& v) { number = v.number; print_sig_mt("copy ctor"); }
//    movable_type(movable_type&& v) { number = v.number; print_sig_mt("move ctor"); }
    ~movable_type() { print_sig_mt("dtor"); }
private:
    int number;
};

template<typename... Types>
void our_func(Types&&... args)
{
}

template<typename... Types>
void my_func(Types&&... args)
{
    our_func(std::forward<Types>(args)...);
}

template<typename... Types>
void my_forward(Types&&... args)
{
    print_sig("now calling my_func");
    my_func(std::forward<Types>(args)...);
    print_sig("called my_func");
}

int main(int argc, char **argv)
{
    print_sig("forward test started");
    movable_type obj(1);
    print_sig("movable_type constructed");
    print_sig("now calling my_forward()");
    my_forward(obj, 42, movable_type{2});
    print_sig("called my_forward()");
    return 0;
}
