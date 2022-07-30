#ifndef UTIL_TUPLE_HELP_H
#define UTIL_TUPLE_HELP_H

#include <cstddef>
#include <cassert>
#include <tuple>
#include <utility>
#include <type_traits>
#include <any>

namespace util {
    template<typename TupleT, typename T, std::size_t... Is>
    bool isExistHelper(const TupleT& tp, T&& elementToBeSearched, std::index_sequence<Is...>) {
        auto isExist = [&elementToBeSearched](const auto& x) {
            if constexpr(std::is_same_v<std::decay_t<decltype(elementToBeSearched)>, std::decay_t<decltype(x)>>) {
                if constexpr (std::is_arithmetic_v<std::decay_t<T>>) {
                    return x == elementToBeSearched;
                } else {
                    return strcmp(x, elementToBeSearched) == 0;
                }    
            }
            else {
                return false;
            }
        };

        return (isExist(std::get<Is>(tp)) || ...);
    }

    template<typename TupleT, typename T, std::size_t TupSize = std::tuple_size<std::decay_t<TupleT>>::value>
    decltype(auto) isExistInTuple(const TupleT& tp, T&& elementToBeSearched) {
        return isExistHelper(tp, elementToBeSearched, std::make_index_sequence<TupSize>{});
    } 

    /**see http://www.cppsamples.com/common-tasks/apply-tuple-to-function.html*/
    template<typename F, typename Tuple, size_t ...S >
    decltype(auto) apply_tuple_impl(F&& fn, Tuple&& t, std::index_sequence<S...>)
    {
        return std::forward<F>(fn)(std::get<S>(std::forward<Tuple>(t))...);
    }
    /**applies arguments to function*/
    template<typename F, typename Tuple>
    decltype(auto) apply_tuple(F&& fn, Tuple&& t)
    {
        std::size_t constexpr tSize
            = std::tuple_size<typename std::remove_reference<Tuple>::type>::value;
        return apply_tuple_impl(std::forward<F>(fn),
                                std::forward<Tuple>(t),
                                std::make_index_sequence<tSize>());
    }

    template <std::size_t Ofst, class Tuple, std::size_t... I>
    constexpr auto slice_impl(Tuple&& t, std::index_sequence<I...>){
        return std::forward_as_tuple(
            std::get<I + Ofst>(std::forward<Tuple>(t))...);
    }
    /**see http://stackoverflow.com/questions/8569567/get-part-of-stdtuple*/
    /**splices tuple into sections.  Call using tuple_splice<firstindex, secondindex>(tuple).  Make sure that firstindex and secondindex are constexpr*/
    template <std::size_t I1, std::size_t I2, class Cont>
    constexpr auto tuple_slice(Cont&& t)  {
        static_assert(I2 >= I1, "invalid slice");
        static_assert(std::tuple_size<std::decay_t<Cont>>::value >= I2, 
            "slice index out of bounds");

        return slice_impl<I1>(std::forward<Cont>(t),std::make_index_sequence<I2 - I1>{});
    }

    /**see https://codereview.stackexchange.com/questions/51407/stdtuple-foreach-implementation*/
    template <typename Tuple, typename F, std::size_t ...Indices>
    auto for_each_impl(Tuple&& tuple, F&& f, std::index_sequence<Indices...>) {
        //constexpr 
        return std::make_tuple(f(std::get<Indices>(std::forward<Tuple>(tuple)), Indices, tuple_slice<0, Indices>(tuple), tuple_slice<Indices+1, std::tuple_size<std::remove_reference_t<Tuple>>::value>(tuple))...);

    }
    /**iterates over a tuple with callback f(val, index, sequence from 0 to index-1, sequence from index+1 to end)*/
    template <typename Tuple, typename F>
    auto for_each(Tuple&& tuple, F&& f) {
        constexpr std::size_t N = std::tuple_size<std::remove_reference_t<Tuple>>::value;
        return for_each_impl(std::forward<Tuple>(tuple), std::forward<F>(f),
                    std::make_index_sequence<N>{});
    }

    template <typename Tuple_Any_T, typename Tuple_Dest_T, std::size_t ...Indices>
    auto fany_cast_impl(Tuple_Any_T&& tuple, std::index_sequence<Indices...>) {
        //constexpr 
        return std::make_tuple(std::any_cast<std::tuple_element_t<Indices, Tuple_Dest_T>>(std::get<Indices>(std::forward<Tuple_Any_T>(tuple)))...);
    }

    template <typename Tuple_Any_T, typename Tuple_Dest_T>
    auto any_cast(Tuple_Any_T&& tuple) {
        constexpr std::size_t ANY_N = std::tuple_size<std::remove_reference_t<Tuple_Any_T>>::value;
        constexpr std::size_t DEST_N = std::tuple_size<std::remove_reference_t<Tuple_Dest_T>>::value;
        static_assert(ANY_N == DEST_N,"ANY_N is not equal DEST_N!");
        
        return fany_cast_impl<Tuple_Any_T, Tuple_Dest_T>(std::forward<Tuple_Any_T>(tuple), std::make_index_sequence<ANY_N>{});
    }

    template <typename tuple, std::size_t ...Indices>
    auto make_repeat_tuple_impl(tuple&& tuple_one, std::index_sequence<Indices...>) {
        //constexpr 
        return std::make_tuple(std::get<Indices-Indices>(std::forward<tuple>(tuple_one))...);
    }

    template <typename Elemment_T, std::size_t N>
    auto make_repeat_tuple(Elemment_T&& elem) {
        std::tuple<Elemment_T> tuple_one={elem};
        return make_repeat_tuple_impl(std::forward<std::tuple<Elemment_T>>(tuple_one), std::make_index_sequence<N>{});
    }
    
    template<typename Tuple>
    void print_tuple(const Tuple& tp) {
        std::cout << "(";
        if constexpr (std::tuple_size_v<Tuple> > 0)
            std::apply(
                [](const auto& head, const auto&... tails) {
                    std::cout << head;
                    ((std::cout << ", " << tails), ...);
                }, tp
        );
        std::cout << ")";
    }
    
    template<typename Tuple, typename Fn>
    void for_each_tuple(Tuple&& tp, Fn&& fn) {
        std::apply(
            [&fn](auto&&... args) {
                (fn(std::forward<decltype(args)>(args)), ...);
            }, 
            std::forward<Tuple>(tp)
        );
    }
}

#endif //UTIL_TUPLE_HELP_H
