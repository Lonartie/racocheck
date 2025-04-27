#pragma once

#include <coroutine>

namespace raco {
   template<typename T> struct promise;
   template<typename T> struct task;
   struct test;

   template<typename T>
   struct promise {
      std::optional<T> value;
      static constexpr auto initial_suspend() noexcept -> std::suspend_never;
      static constexpr auto final_suspend() noexcept -> std::suspend_always;
      static constexpr auto suspend_ready() noexcept -> bool;
      static constexpr auto unhandled_exception() noexcept -> void;
      constexpr auto return_value(T&& v) noexcept -> void;
      constexpr auto get_return_object() noexcept -> task<T>;
   };

   template<>
   struct promise<void> {
      static constexpr auto initial_suspend() noexcept -> std::suspend_never;
      static constexpr auto final_suspend() noexcept -> std::suspend_always;
      static constexpr auto suspend_ready() noexcept -> bool;
      static constexpr auto unhandled_exception() noexcept -> void;
      static constexpr auto return_void() noexcept -> void;
      constexpr auto get_return_object() noexcept -> task<void>;
   };

   template<typename T>
   struct task {
      using promise_type = promise<T>;
      constexpr task() noexcept = default;
      constexpr explicit task(std::coroutine_handle<promise_type> handle) noexcept;
      constexpr explicit task(std::coroutine_handle<> handle) noexcept;
      constexpr auto done() const noexcept -> bool;
      constexpr auto resume() const noexcept -> void;
      constexpr auto destroy() const noexcept -> void;
      constexpr auto handle() const noexcept -> std::coroutine_handle<>;
      constexpr auto result() const noexcept -> T;

   private:
      std::coroutine_handle<promise_type> m_handle;
   };

   struct test {
      constexpr test() = default;
      static constexpr auto await_ready() noexcept -> bool;
      template<typename T>
      constexpr auto await_suspend(std::coroutine_handle<promise<T> > handle) const noexcept -> void;
      static constexpr auto await_resume() noexcept -> void;
   };

   struct void_t {};
   template <typename T> struct task_remover { using type = T; };
   template <typename T> struct task_remover<task<T>> { using type = T; };
   template <> struct task_remover<task<void>> { using type = void_t; };
}
