#include <concepts>
#include <compare>
#include <iostream>
#include <type_traits>

namespace lispiny
{

/*    ___ ___  _ __  ___    */
/*   / __/ _ \| '_ \/ __|   */
/*  | (_| (_) | | | \__ \   */
/*   \___\___/|_| |_|___/   */
/*                          */

template<auto Car, auto Cdr>
struct cons_t {};
template<auto Car, auto Cdr>
inline constexpr cons_t<Car, Cdr> cons;

template<typename Cons>      struct is_cons : std::false_type{};
template<auto Car, auto Cdr> struct is_cons<cons_t<Car, Cdr>>: std::true_type{};

/*                  __      _        */
/*   ___ __ _ _ __ / /__ __| |_ __   */
/*  / __/ _` | '__/ / __/ _` | '__|  */
/* | (_| (_| | | / / (_| (_| | |     */
/*  \___\__,_|_|/_/ \___\__,_|_|     */
/*                                   */

template<typename Cons>
constexpr auto car_helper(Cons) {return car_helper(Cons::value);}
template<auto Car, auto Cdr>
constexpr decltype(Car) car_helper(cons_t<Car, Cdr>) {return Car;}

template<typename Cons>
constexpr auto cdr_helper(Cons) {return cdr_helper(Cons::value);}
template<auto Car, auto Cdr>
constexpr decltype(Cdr) cdr_helper(cons_t<Car, Cdr>) {return Cdr;}

template<auto Cons>
struct car_t
{
    static constexpr auto value = car_helper(Cons);
};
template<auto Cons>
inline constexpr auto car = car_t<Cons>::value;

template<auto Cons>
struct cdr_t
{
    static constexpr auto value = cdr_helper(Cons);
};
template<auto Cons>
inline constexpr auto cdr = cdr_t<Cons>::value;

/*       _        _               */
/*   ___| |_ _ __(_)_ __   __ _   */
/*  / __| __| '__| | '_ \ / _` |  */
/*  \__ \ |_| |  | | | | | (_| |  */
/*  |___/\__|_|  |_|_| |_|\__, |  */
/*                        |___/   */

template<std::size_t N>
struct string
{
    constexpr string() = default;
    constexpr string(const char (&s)[N])
    {
        for(std::size_t i=0; i<N; ++i) {str[i] = s[i];}
    }
    char str[N] = {};

    constexpr bool operator==(const string& other) const
    {
        return std::char_traits<char>::compare(str, other.str, N) == 0;
    }
    constexpr std::strong_ordering operator<=>(const string& other) const
    {
        using traits = std::char_traits<char>;
        constexpr int result = traits::compare(str, other.str, N);
        if      (result <  0) {return std::strong_ordering::less;}
        else if (result == 0) {return std::strong_ordering::equal;}
        else                  {return std::strong_ordering::greater;}
    }
};
template<std::size_t N>
std::ostream& operator<<(std::ostream& os, string<N> s)
{
    return os << s.str;
}

template<typename    T> struct is_string            : std::false_type {};
template<std::size_t N> struct is_string<string<N>> : std::true_type  {};

template<std::size_t N, std::size_t M>
constexpr string<N+M-1> operator+(const string<N> lhs, const string<M> rhs)
{
    string<N+M-1> retval;
    for(std::size_t i=0; i<N-1; ++i) {retval.str[i    ] = lhs.str[i];}
    for(std::size_t i=0; i<M-1; ++i) {retval.str[i+N-1] = rhs.str[i];}
    retval.str[N+M-2] = '\0';
    return retval;
}

template<std::integral Integer>
constexpr std::size_t digits_of(Integer i)
{
    std::size_t digit = 1;
    if(i < 0)
    {
        i = -i;
        digit += 1;
    }
    while(i /= 10) {digit += 1;}
    return digit;
}

template<std::integral Integer, Integer X>
constexpr string<digits_of(X)+1> to_string()
{
    string<digits_of(X)+1> retval;
    retval.str[digits_of(X)] = '\0';

    Integer     x = X;
    std::size_t i = digits_of(X) - 1;
    if(x < 0)
    {
        retval.str[0] = '-';
        x = -x;
    }
    while(0 < x)
    {
        retval.str[i] = char(48) + x % 10;
        x /= 10;
        i -= 1;
    }
    return retval;
}

template<typename T, T X>
requires is_string<std::remove_cvref_t<T>>::value
constexpr T to_string()
{
    return X; // fallback for to_string(string)
}

/*    _                 _         _         */
/*   | | __ _ _ __ ___ | |__   __| | __ _   */
/*   | |/ _` | '_ ` _ \| '_ \ / _` |/ _` |  */
/*   | | (_| | | | | | | |_) | (_| | (_| |  */
/*   |_|\__,_|_| |_| |_|_.__/ \__,_|\__,_|  */
/*                                          */

template<std::size_t Index>
struct arg_t
{
    static constexpr auto value = Index;
};
template<std::size_t Index>
inline constexpr arg_t<Index> arg;

template<typename    T> struct is_arg           : std::false_type {};
template<std::size_t I> struct is_arg<arg_t<I>> : std::true_type  {};

template<auto ArgList, std::size_t Arg>
constexpr auto find_helper()
{
    if constexpr (Arg == 0)
    {
        return car<ArgList>;
    }
    else
    {
        return find_helper<cdr<ArgList>, Arg-1>();
    }
}

template<auto ArgList, auto Arg>
constexpr auto find_if_arg()
{
    if constexpr (is_arg<std::remove_cvref_t<decltype(Arg)>>::value)
    {
        return find_helper<ArgList, Arg.value>();
    }
    else
    {
        return Arg;
    }
}

template<auto ArgList, auto Arg>
struct find_t
{
    static constexpr auto value = find_if_arg<ArgList, Arg>();
};
template<auto ArgList, auto Arg>
inline constexpr auto find = find_t<ArgList, Arg>::value;

// TODO

template<auto Body>
struct lambda_t
{
    template<auto Args>
    constexpr auto apply()
    {
        // TODO
        // lookup body recursively and replace arg<I> by find<Args, arg<I>>.
        // while doing that, we assume the function body is a cons list.

        // then evaluate the list.
    }
};
template<auto Body>
inline constexpr lambda_t<Body> lambda;

/*                   _    */
/*    _____   ____ _| |   */
/*   / _ \ \ / / _` | |   */
/*  |  __/\ V / (_| | |   */
/*   \___| \_/ \__,_|_|   */
/*                        */

template<auto Car, auto Cdr>
requires requires {Car.template apply<Cdr>();}
constexpr bool is_applicable()
{
    return true;
}
template<auto Car, auto Cdr>
constexpr auto is_applicable()
{
    return false;
}
template<auto Expr>
constexpr bool is_reducible()
{
    if constexpr (is_cons<std::remove_cvref_t<decltype(Expr)>>::value)
    {
        return is_applicable<car<Expr>, cdr<Expr>>();
    }
    else
    {
        return false;
    }
}

template<auto Expr>
constexpr auto evaluate()
{
    if constexpr (is_reducible<Expr>())
    {
        return evaluate<evaluate<car<Expr>>().template apply<cdr<Expr>>()>();
    }
    else
    {
        return Expr;
    }
}

template<auto Cons>
struct eval_t
{
    static constexpr auto value = evaluate<Cons>();
};
template<auto Cons>
inline constexpr auto eval = eval_t<Cons>::value;

/*       _         */
/*    __| | ___    */
/*   / _` |/ _ \   */
/*  | (_| | (_) |  */
/*   \__,_|\___/   */
/*                 */

// TODO

/*               _ _   _                    _   _         */
/*     __ _ _ __(_) |_| |__  _ __ ___   ___| |_(_) ___    */
/*    / _` | '__| | __| '_ \| '_ ` _ \ / _ \ __| |/ __|   */
/*   | (_| | |  | | |_| | | | | | | | |  __/ |_| | (__    */
/*    \__,_|_|  |_|\__|_| |_|_| |_| |_|\___|\__|_|\___|   */
/*                                                        */

#define LISPINY_DECLARE_ARITHMETIC_OPERATORS(name, op)                      \
    struct name##_t                                                         \
    {                                                                       \
        template<auto Cons, typename Result>                                \
        constexpr auto apply_helper(Result x) const                         \
        {                                                                   \
            if constexpr(std::is_same_v<decltype(Cons), std::nullptr_t>)    \
            {                                                               \
                return x;                                                   \
            }                                                               \
            else                                                            \
            {                                                               \
                return apply_helper<eval<cdr<Cons>>>(x op eval<car<Cons>>); \
            }                                                               \
        }                                                                   \
        template<auto Cons>                                                 \
        constexpr auto apply() const                                        \
        {                                                                   \
            return apply_helper<eval<cdr<Cons>>>(eval<car<Cons>>);          \
        }                                                                   \
    };                                                                      \
    inline constexpr name##_t name; /**/

LISPINY_DECLARE_ARITHMETIC_OPERATORS(plus,       +)
LISPINY_DECLARE_ARITHMETIC_OPERATORS(minus,      -)
LISPINY_DECLARE_ARITHMETIC_OPERATORS(multiplies, *)
LISPINY_DECLARE_ARITHMETIC_OPERATORS(divides,    /)
LISPINY_DECLARE_ARITHMETIC_OPERATORS(modulus,    %)

#undef LISPINY_DECLARE_ARITHMETIC_OPERATORS

/*                                        _                    */
/*    ___ ___  _ __ ___  _ __   __ _ _ __(_)___  ___  _ __     */
/*   / __/ _ \| '_ ` _ \| '_ \ / _` | '__| / __|/ _ \| '_ \    */
/*  | (_| (_) | | | | | | |_) | (_| | |  | \__ \ (_) | | | |   */
/*   \___\___/|_| |_| |_| .__/ \__,_|_|  |_|___/\___/|_| |_|   */
/*                      |_|                                    */

inline constexpr std::nullptr_t nil = nullptr;
inline constexpr bool           T   = true;

#define LISPINY_DECLARE_COMPARISON_OPERATORS(name, op)                    \
    struct name##_t                                                       \
    {                                                                     \
        template<auto Cons>                                               \
        constexpr auto apply() const                                      \
        {                                                                 \
            if constexpr (std::is_same_v<decltype(eval<car<Cons>>),       \
                                         decltype(eval<car<cdr<Cons>>>)>) \
            {                                                             \
                if constexpr(eval<car<Cons>> op eval<car<cdr<Cons>>>)     \
                {                                                         \
                    return T;                                             \
                }                                                         \
                else                                                      \
                {                                                         \
                    return nil;                                           \
                }                                                         \
            }                                                             \
            else                                                          \
            {                                                             \
                return nil;                                               \
            }                                                             \
        }                                                                 \
    };                                                                    \
    inline constexpr name##_t name; /**/

LISPINY_DECLARE_COMPARISON_OPERATORS(eq,    ==)
LISPINY_DECLARE_COMPARISON_OPERATORS(ne,    !=)
LISPINY_DECLARE_COMPARISON_OPERATORS(lt,    < )
LISPINY_DECLARE_COMPARISON_OPERATORS(lt_eq, <=)
LISPINY_DECLARE_COMPARISON_OPERATORS(gt,    > )
LISPINY_DECLARE_COMPARISON_OPERATORS(gt_eq, >=)

#undef LISPINY_DECLARE_COMPARISON_OPERATORS

/*  _  __  */
/* (_)/ _| */
/* | | |_  */
/* | |  _| */
/* |_|_|   */
/*         */

struct if_t
{
    template<auto Cons>
    constexpr auto apply() const
    {
        if constexpr(is_cons<std::remove_cvref_t<decltype(Cons)>>::value)
        {
            if constexpr(static_cast<bool>(eval<car<Cons>>))
            {
                return car<cdr<Cons>>;
            }
            else
            {
                return car<cdr<cdr<Cons>>>;
            }
        }
        else
        {
            return Cons;
        }
    }
};
inline constexpr if_t if_;

/*    _ _     _      */
/*   | (_)___| |_    */
/*   | | / __| __|   */
/*   | | \__ \ |_    */
/*   |_|_|___/\__|   */
/*                   */

// just an alias of `cons< cons< cons< ... > > >`.

template<auto Car, auto ... Cdr>
constexpr auto expand_list()
{
    if constexpr (sizeof...(Cdr) == 0)
    {
        return cons<Car, nil>;
    }
    else
    {
        return cons<Car, expand_list<Cdr...>()>;
    }
}

template<auto ... Elem>
struct list_t
{
    static constexpr auto value = expand_list<Elem...>();
};
template<auto ... Elem>
inline constexpr auto list = list_t<Elem...>::value;

} // lispiny

int main()
{
    using namespace lispiny;

    static_assert(eval<cons<plus, cons<1, cons<2, cons<3, cons<4, nil>>>>>> == 10);
    static_assert(eval<list<plus, 1, 2, 3, 4>                  >            == 10);
    static_assert(eval<list<plus, string("foo"), string("bar")>>            == string("foobar"));
    static_assert(eval<list<multiplies, 3, 4>                  >            == 12);
    static_assert(eval<list<plus, 1, 2, list<multiplies, 3, 4>>>            == 15);

    static_assert(eval<list<if_, list<eq, 1, 2>, list<plus, 1, 2>, list<multiplies, 3, 4>>> == 12);

    return 0;
}
