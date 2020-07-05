#ifndef LISPINY_HPP
#define LISPINY_HPP

#include <concepts>
#include <compare>
#include <iostream>
#include <type_traits>

// ascii arts are generated by http://patorhttp://patooftware/taag/ .

namespace lispiny
{

/*   _____  __     _ _    */
/*  |_   _|/ / __ (_) |   */
/*    | | / / '_ \| | |   */
/*    | |/ /| | | | | |   */
/*    |_/_/ |_| |_|_|_|   */
/*                        */

inline constexpr std::nullptr_t nil = nullptr;
inline constexpr bool           T   = true;

template<auto Expr>
struct is_nil: std::bool_constant<
    std::is_same_v<std::remove_cvref_t<decltype(Expr)>, std::nullptr_t>>{};

/*    ___ ___  _ __  ___    */
/*   / __/ _ \| '_ \/ __|   */
/*  | (_| (_) | | | \__ \   */
/*   \___\___/|_| |_|___/   */
/*                          */

template<auto Car, auto Cdr>
struct cons_t {};
template<auto Car, auto Cdr>
inline constexpr cons_t<Car, Cdr> cons;

template<typename Cons>      struct is_cons_t : std::false_type{};
template<auto Car, auto Cdr> struct is_cons_t<cons_t<Car, Cdr>>: std::true_type{};
template<auto Cons>
inline constexpr bool is_cons = is_cons_t<std::remove_cvref_t<decltype(Cons)>>::value;

template<auto Car, auto Cdr>
std::ostream& operator<<(std::ostream& os, cons_t<Car, Cdr>)
{
    os << '(' << Car << ' ' << Cdr << ')';
    return os;
}

/*                  __      _        */
/*   ___ __ _ _ __ / /__ __| |_ __   */
/*  / __/ _` | '__/ / __/ _` | '__|  */
/* | (_| (_| | | / / (_| (_| | |     */
/*  \___\__,_|_|/_/ \___\__,_|_|     */
/*                                   */

template<auto Car, auto Cdr>
constexpr decltype(Car) car_helper(cons_t<Car, Cdr>) {return Car;}
template<auto Car, auto Cdr>
constexpr decltype(Cdr) cdr_helper(cons_t<Car, Cdr>) {return Cdr;}

template<auto Cons>
inline constexpr auto car = car_helper(Cons);
template<auto Cons>
inline constexpr auto cdr = cdr_helper(Cons);

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
inline constexpr auto list = expand_list<Elem...>();

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
    if constexpr (is_cons<Expr>)
    {
        return is_applicable<car<Expr>, cdr<Expr>>() ||
               is_reducible<car<Expr>>() || is_reducible<cdr<Expr>>();
    }
    else
    {
        return false;
    }
}

template<auto Expr>
constexpr auto evaluate()
{
    if constexpr (is_cons<Expr>)
    {
        if constexpr (is_applicable<car<Expr>, cdr<Expr>>())
        {
            return evaluate<evaluate<car<Expr>>().template apply<cdr<Expr>>()>();
        }
        else
        {
            return cons<evaluate<car<Expr>>(), evaluate<cdr<Expr>>()>;
        }
    }
    else
    {
        return Expr;
    }
}

template<auto Cons>
inline constexpr auto eval = evaluate<Cons>();

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

template<typename    T> struct is_arg_t           : std::false_type {};
template<std::size_t I> struct is_arg_t<arg_t<I>> : std::true_type  {};
template<auto Expr>
struct is_arg : is_arg_t<std::remove_cvref_t<decltype(Expr)>> {};

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

// replace all the arg<N> by N-th elem of Args.
template<auto Expr, auto Args>
constexpr auto substitute()
{
    if constexpr (is_cons<Expr>)
    {
        return cons<substitute<car<Expr>, Args>(), substitute<cdr<Expr>, Args>()>;
    }
    else if constexpr (is_arg<Expr>::value)
    {
        return find_helper<Args, Expr.value>();
    }
    else
    {
        return Expr;
    }
}

template<auto Body>
struct lambda_t
{
    template<auto Args>
    constexpr auto apply() const
    {
        // lookup body recursively and replace arg<I> by find<Args, arg<I>>.
        // while doing that, we assume the function body is a cons list.
        // then evaluate the list.
        return eval<substitute<Body, Args>()>;
    }
};
template<auto Body>
inline constexpr lambda_t<Body> lambda;

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
            if constexpr(is_nil<cdr<Cons>>::value)                          \
            {                                                               \
                return x op eval<car<Cons>>;                                \
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

#define LISPINY_DECLARE_COMPARISON_OPERATORS(name, op)                             \
    struct name##_t                                                                \
    {                                                                              \
        template<auto Cons>                                                        \
        constexpr auto apply() const                                               \
        {                                                                          \
            using car_type  = std::remove_cvref_t<decltype(eval<car<Cons>>)>;      \
            using cadr_type = std::remove_cvref_t<decltype(eval<car<cdr<Cons>>>)>; \
            if constexpr (std::is_same_v<car_type, cadr_type>)                     \
            {                                                                      \
                if constexpr(eval<car<Cons>> op eval<car<cdr<Cons>>>)              \
                {                                                                  \
                    return T;                                                      \
                }                                                                  \
                else                                                               \
                {                                                                  \
                    return nil;                                                    \
                }                                                                  \
            }                                                                      \
            else                                                                   \
            {                                                                      \
                return nil;                                                        \
            }                                                                      \
        }                                                                          \
    };                                                                             \
    inline constexpr name##_t name; /**/

LISPINY_DECLARE_COMPARISON_OPERATORS(eq,    ==)
LISPINY_DECLARE_COMPARISON_OPERATORS(ne,    !=)
LISPINY_DECLARE_COMPARISON_OPERATORS(lt,    < )
LISPINY_DECLARE_COMPARISON_OPERATORS(lt_eq, <=)
LISPINY_DECLARE_COMPARISON_OPERATORS(gt,    > )
LISPINY_DECLARE_COMPARISON_OPERATORS(gt_eq, >=)

#undef LISPINY_DECLARE_COMPARISON_OPERATORS

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

template<typename    T> struct is_string_t            : std::false_type {};
template<std::size_t N> struct is_string_t<string<N>> : std::true_type  {};
template<auto Expr>
struct is_string: is_string_t<std::remove_cvref_t<decltype(Expr)>> {};

template<std::size_t N, std::size_t M>
constexpr string<N+M-1> operator+(const string<N> lhs, const string<M> rhs)
{
    string<N+M-1> retval;
    for(std::size_t i=0; i<N-1; ++i) {retval.str[i    ] = lhs.str[i];}
    for(std::size_t i=0; i<M-1; ++i) {retval.str[i+N-1] = rhs.str[i];}
    retval.str[N+M-2] = '\0';
    return retval;
}

/*   _               _        _               */
/*  | |_ ___     ___| |_ _ __(_)_ __   __ _   */
/*  | __/ _ \   / __| __| '__| | '_ \ / _` |  */
/*  | || (_) |  \__ \ |_| |  | | | | | (_| |  */
/*   \__\___/___|___/\__|_|  |_|_| |_|\__, |  */
/*         |_____|                    |___/   */

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

template<auto Int>
requires std::is_integral_v<std::remove_cvref_t<decltype(Int)>>
constexpr string<digits_of(Int)+1> to_string_impl()
{
    string<digits_of(Int)+1> retval;
    retval.str[digits_of(Int)] = '\0';

    auto        x = Int;
    std::size_t i = digits_of(Int) - 1;
    if(x < 0)
    {
        retval.str[0] = '-';
        x = -x;
    }
    if(x == 0)
    {
        retval.str[0] = '0';
        return retval;
    }
    while(0 < x)
    {
        retval.str[i] = char(48) + x % 10;
        x /= 10;
        i -= 1;
    }
    return retval;
}

template<auto Str>
requires is_string<Str>::value
constexpr auto to_string_impl()
{
    return Str; // fallback for to_string(string)
}

struct to_string_t
{
    template<auto Expr>
    constexpr auto apply_helper() const
    {
        if constexpr (is_nil<cdr<Expr>>::value)
        {
            return to_string_impl<eval<car<Expr>>>();
        }
        else
        {
            return to_string_impl<eval<car<Expr>>>() + apply_helper<eval<cdr<Expr>>>();
        }
    }

    template<auto Expr>
    constexpr auto apply() const
    {
        return apply_helper<Expr>();
    }
};
inline constexpr to_string_t to_string;

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
        if constexpr(is_cons<Cons>)
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

/*            _     _ _        */
/*  __      _| |__ (_) | ___   */
/*  \ \ /\ / / '_ \| | |/ _ \  */
/*   \ V  V /| | | | | |  __/  */
/*    \_/\_/ |_| |_|_|_|\___|  */
/*                             */

struct while_t
{
    template<auto Env, auto Cond, auto Body>
    constexpr auto apply_impl() const
    {
        if constexpr (eval<cons<Cond, Env>>)
        {
            return apply_impl<eval<cons<Body, Env>>, Cond, Body>();
        }
        else
        {
            return Env;
        }
    }
    template<auto Cons>
    constexpr auto apply() const
    {
        return apply_impl< car<Cons>, car<cdr<Cons>>, car<cdr<cdr<Cons>>> >();
    }
};
inline constexpr while_t while_;

} // lispiny
#endif // LISPINY_HPP
