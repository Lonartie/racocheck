#pragma once

namespace raco {
#ifdef RACO_TEST_ENABLED
   template<typename T> using fun = raco::task<T>;
#  define RACO_CHECKPOINT co_await raco::test{}
#  define RACO_TEST_INLINE(X) (RACO_CHECKPOINT, X)
#  define RACO_TEST(X) RACO_CHECKPOINT; X
#  define RACO_RETURN co_return
#else
   template <typename T> using fun = T;
#  define RACO_CHECKPOINT
#  define RACO_TEST_INLINE(X)
#  define RACO_TEST(X)
#  define RACO_RETURN return
#endif

#define _STRINGIFY(A) #A
#define STRINGIFY(A) _STRINGIFY(A)

   static thread_local size_t nesting_level = 0;

   static void indent() {
      nesting_level += 2;
   }

   static void unindent() {
      nesting_level -= 2;
   }

   static std::string nesting() {
      return std::string(nesting_level, ' ');
   }

   static std::string nesting_indent() {
      auto result = std::string(nesting_level, ' ');
      indent();
      return result;
   }

#define SMAP(NAME) stream << raco::nesting() << STRINGIFY(NAME) << " = " << v.NAME << "\n";
#define DEBUG_STREAM(CLS, MAPPINGS) \
   friend std::ostream& operator<<(std::ostream& stream, const CLS& v) { \
      stream << STRINGIFY(CLS) << " {\n"; \
      raco::indent(); \
      MAPPINGS \
      raco::unindent(); \
      return stream << raco::nesting() << "}"; \
   }
#define EXT_DEBUG_STREAM(CLS, MAPPINGS) \
   std::ostream& operator<<(std::ostream& stream, const CLS& v) { \
      stream << STRINGIFY(CLS) << " {\n"; \
      raco::indent(); \
      MAPPINGS \
      raco::unindent(); \
      return stream << raco::nesting() << "}"; \
   }
}
