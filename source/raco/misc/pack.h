#pragma once

#include <tuple>

namespace raco {

   template<typename ... Fs>
   struct pack {
      explicit pack(Fs&&... fs);
      static constexpr size_t size = sizeof...(Fs);
      std::tuple<Fs...> args;

      template <template <typename> class Transformer>
      using transform = pack<typename Transformer<Fs>::type...>;

      template <template <typename ...> class NewContainer>
      using to = NewContainer<Fs...>;
   };

   template <typename ... Args>
   auto tuple_to_pack_f(std::tuple<Args...>) -> pack<Args...>;

   template <typename T>
   using tuple_to_pack_t = std::decay_t<decltype(tuple_to_pack_f(std::declval<T>()))>;
}