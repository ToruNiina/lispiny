# lispiny

a spiny lisp which uses a lot of `>` instead of `)`.

It runs at compile time.

## Example Code

```cpp
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
```

```console
$ g++-10 -std=c++20 main.cpp -O2 -Wall -Wextra -Wpedantic -Wfatal-errors -I. -o fizzbuzz
$ ./fizzbuzz
1
2
Fizz
4
Buzz
Fizz
7
8
Fizz
Buzz
11
Fizz
13
14
FizzBuzz
16
17
Fizz
19

```

## Usage

### cons, car, cdr, list

Because it is a LISP, you can use `cons`, `car`, and `cdr`.

`list<1, 2, 3>` is an alias of `cons<1, cons<2, cons<3, nil>>>`.

### basic calculation

You can use basic arithmetic operators and comparison operators.

- `plus`, `minus`, `multiplies`, `divides`, `modulus`, `to_string`.
- `eq`, `ne`, `lt`, `lt_eq`, `gt`, `gt_eq`.

```cpp
#include <lispiny.hpp>

int main()
{
    using namespace lispiny;

    static_assert(eval<list<plus, 1, 2>> ==   3);
    static_assert(eval<list<eq,   1, 2>> == nil);
    static_assert(eval<list<lt,   1, 2>> ==   T);

    static_assert(eval<list<plus, string("foo"), string("bar")>> == string("foobar"));
    static_assert(eval<list<to_string, 42>> == string("42"));

    return 0;
}
```

### function

You can define an anonymous function and capture it as a `constexpr` value.

```cpp
#include <lispiny.hpp>

int main()
{
    using namespace lispiny;

    constexpr auto plus10 = lambda<list<plus, 10, arg<0>>>;

    static_assert(eval<list<plus10, 1>> == 11);
    static_assert(eval<list<plus10, 2>> == 12);

    return 0;
}
```

### control flow

- `list<if_,    [cond], [then], [else]>`

`[cond]` should be evaluated into `T` or `nil`. If `T`, the whole list will be
evaluated into `[then]`. Otherwise, `[else]`.

```cpp
#include <lispiny.hpp>

int main()
{
    using namespace lispiny;

    static_assert(eval<list<if_,   T, 42, string("foo")>> == 42);
    static_assert(eval<list<if_, nil, 42, string("foo")>> == string("foo"));

    return 0;
}
```

- `list<while_, [env],  [cond], [body]>`

`[env]` is a list and updated in every iteration.
`[body]` is a function that takes `[env]` and returns a list. The returned list
will be the next `[env]`.

`[cond]` is a function that takes `[env]` and returns `T` or `nil`.
If `<cond, env>` returns `nil`, then the calculation stops and returns `env` at
that time.

```cpp
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
}
```
