#pragma once

#include "task.h"

namespace raco {
   template<typename T>
   constexpr auto promise<T>::initial_suspend() noexcept -> std::suspend_never {
      return std::suspend_never();
   }

   template<typename T>
   constexpr auto promise<T>::final_suspend() noexcept -> std::suspend_always {
      return std::suspend_always();
   }

   template<typename T>
   constexpr auto promise<T>::suspend_ready() noexcept -> bool {
      return false;
   }

   template<typename T>
   constexpr auto promise<T>::unhandled_exception() noexcept -> void {
   }

   template<typename T>
   constexpr auto promise<T>::return_value(T&& v) noexcept -> void {
      value = std::forward<T>(v);
   }

   template<typename T>
   constexpr auto promise<T>::return_value(const T& v) noexcept -> void {
      value = v;
   }

   template<typename T>
   constexpr auto promise<T>::get_return_object() noexcept -> task<T> {
      return task<T>(std::coroutine_handle<promise<T> >::from_promise(*this));
   }

   constexpr auto promise<void>::initial_suspend() noexcept -> std::suspend_never {
      return std::suspend_never();
   }

   constexpr auto promise<void>::final_suspend() noexcept -> std::suspend_always {
      return std::suspend_always();
   }

   constexpr auto promise<void>::suspend_ready() noexcept -> bool {
      return false;
   }

   constexpr auto promise<void>::unhandled_exception() noexcept -> void {
   }

   constexpr auto promise<void>::return_void() noexcept -> void {
   }

   constexpr auto promise<void>::get_return_object() noexcept -> task<void> {
      return task<void>(std::coroutine_handle<promise<void> >::from_promise(*this));
   }

   template<typename T>
   constexpr task<T>::task(std::coroutine_handle<promise_type> handle) noexcept
      : m_handle(handle) {
   }

   template<typename T>
   constexpr task<T>::task(const std::coroutine_handle<> handle) noexcept
      : m_handle(std::coroutine_handle<promise_type>::from_address(handle.address())) {
   }

   template<typename T>
   constexpr auto task<T>::done() const noexcept -> bool {
      return m_handle.done();
   }

   template<typename T>
   constexpr auto task<T>::resume() const noexcept -> void {
      return m_handle.resume();
   }

   template<typename T>
   constexpr auto task<T>::destroy() const noexcept -> void {
      return m_handle.destroy();
   }

   template<typename T>
   constexpr auto task<T>::handle() const noexcept -> std::coroutine_handle<> {
      return m_handle;
   }

   template<typename T>
   constexpr auto task<T>::result() const noexcept -> T {
      return m_handle.promise().value.value();
   }

   constexpr auto test::await_ready() noexcept -> bool {
      return false;
   }

   template<typename T>
   constexpr auto test::await_suspend(std::coroutine_handle<promise<T> > handle) const noexcept -> void {
   }

   constexpr auto test::await_resume() noexcept -> void {
   }
}
