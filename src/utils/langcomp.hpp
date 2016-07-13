
#ifndef includeguard_utils_langcomp_hpp_includeguard
#define includeguard_utils_langcomp_hpp_includeguard

#define assert_unreachable() do { assert ("unreachable" && 0); } while (1)

#define likely(expr) __builtin_expect (expr, 1)
#define unlikely(expr) __builtin_expect (expr, 0)

#define countof(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#endif // includeguard_utils_langcomp_hpp_includeguard
