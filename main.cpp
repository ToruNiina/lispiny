#include <lispiny.hpp>

int main()
{
    using namespace lispiny;

    constexpr auto env  = list<1, string("")>;
    constexpr auto cond = lambda<list<lt, arg<0>, 20>>;
    constexpr auto body = lambda<list<
            list<plus, arg<0>, 1>,
            list<plus, arg<1>,
                list<if_, list<eq, 0, list<modulus, arg<0>, 15>>, string("FizzBuzz\n"),
                list<if_, list<eq, 0, list<modulus, arg<0>, 3>>, string("Fizz\n"),
                list<if_, list<eq, 0, list<modulus, arg<0>, 5>>, string("Buzz\n"),
                          list<plus, list<to_string, arg<0>>, string("\n")>
            >>>>
        >>;

    constexpr auto result = eval<list<while_, env, cond, body>>;
    std::cout << car<cdr<result>> << std::endl;

    return 0;
}
