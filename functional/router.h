//author qicosmos@163.com
#pragma once

#include <string>
#include <unordered_map>
#include "util/function/function_traists.h"
#include "util/tuple_help.h"

namespace util {

    template <typename KEY_T>
    class router {
    private:
      using tuple_10_any_t = std::tuple<std::any,std::any,std::any,std::any,std::any,std::any,std::any,std::any,std::any,std::any>; 

    public:
        static router& get() {
          static router instance;
          return instance;
        }

        template<typename Function>
        void register_handler(KEY_T const& name, const Function& f) {
          func_ptr_to_key_map_.emplace((void*)&f, name);
          return register_nonmember_func(name, f);
        }

        template<typename Function, typename Self>
        void register_handler(KEY_T const& name, const Function& f, Self* self) {
          return register_member_func(name, f, self);
        }

        void remove_handler(KEY_T const& name) { this->map_invokers_.erase(name); }

        KEY_T get_key(void* ptr){
          auto it = func_ptr_to_key_map_.find(ptr);
          if(it!=func_ptr_to_key_map_.end()){
            return it->second;
          }

          return KEY_T();
        }

        template<typename... Args>
        void route(KEY_T const& name, Args&&... args) {
          auto tp = std::make_tuple(std::forward<Args>(args)...);
          constexpr std::size_t any_args_size = std::tuple_size<tuple_10_any_t>::value;
          constexpr std::size_t raw_size = std::tuple_size<decltype(tp)>::value;
          auto sub_tp = make_repeat_tuple<std::any, any_args_size - raw_size>(std::any());
          tuple_10_any_t any_tp(std::tuple_cat(tp, sub_tp));

          auto it = map_invokers_.find(name);
          if (it == map_invokers_.end()) {
            return;
          }
          std::string result;
          it->second(std::move(any_tp), result);
        }

        router() = default;

    private:
        router(const router&) = delete;
        router(router&&) = delete;

        // template<typename F, size_t... I, typename Arg, typename... Args>
        // static typename std::result_of<F(Args...)>::type call_helper(
        //         const F & f, const std::index_sequence<I...>&, std::tuple<Arg, Args...> tup) {
        //   return f(std::move(std::get<I + 1>(tup))...);
        // }

        // template<typename F, typename Arg, typename... Args>
        // static
        // typename std::enable_if<std::is_void<typename std::result_of<F(Args...)>::type>::value>::type
        // call(const F & f, std::string & result, std::tuple<Arg, Args...> tp) {
        //   call_helper(f, std::make_index_sequence<sizeof...(Args)>{}, std::move(tp));
        //   result = "OK";
        // }

        // template<typename F, typename Arg, typename... Args>
        // static
        // typename std::enable_if<!std::is_void<typename std::result_of<F(Args...)>::type>::value>::type
        // call(const F & f, std::string & result, std::tuple<Arg, Args...> tp) {
        //   auto r = call_helper(f, std::make_index_sequence<sizeof...(Args)>{}, std::move(tp));
        //   result = "Ok";
        // }

        template<typename Function>
        struct invoker {
            static inline void apply(const Function& func, tuple_10_any_t&& raw_args, std::string& result) {
              using args_tuple = typename function_traits<Function>::args_tuple;
              constexpr std::size_t dest_args_size = std::tuple_size<args_tuple>::value;
              auto tmp_args = tuple_slice<0, dest_args_size>(raw_args);
              try {
                args_tuple args_tp = any_cast<decltype(tmp_args), args_tuple>(std::move(tmp_args));
                std::apply(func, std::move(args_tp));
              }
              catch (const std::exception & e) {
                result = e.what();
              }
            }
        };

        template<typename Function>
        void register_nonmember_func(std::string const& name, Function f) {
          this->map_invokers_[name] = { std::bind(&invoker<Function>::apply, std::move(f), std::placeholders::_1,
                                                  std::placeholders::_2) };
        }

        std::unordered_map<KEY_T,
                std::function<void(tuple_10_any_t&&, std::string&)>>
                map_invokers_;
        std::unordered_map<void*, std::string> func_ptr_to_key_map_;
    };
}
