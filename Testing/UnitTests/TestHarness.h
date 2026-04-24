#ifndef CONVERTMESH_TEST_HARNESS_H
#define CONVERTMESH_TEST_HARNESS_H

#include <cstdio>
#include <cstdlib>
#include <string>

#define CM_CHECK(cond)                                                      \
  do {                                                                       \
    if(!(cond)) {                                                            \
      std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);   \
      std::exit(1);                                                          \
    }                                                                        \
  } while(0)

#define CM_CHECK_EQ(a, b)                                                   \
  do {                                                                       \
    auto _cm_a = (a);                                                        \
    auto _cm_b = (b);                                                        \
    if(!(_cm_a == _cm_b)) {                                                  \
      std::fprintf(stderr, "FAIL %s:%d: " #a " == " #b "\n",                 \
                   __FILE__, __LINE__);                                      \
      std::exit(1);                                                          \
    }                                                                        \
  } while(0)

#define CM_CHECK_THROWS(expr, ExceptionType)                                \
  do {                                                                       \
    bool _cm_threw = false;                                                  \
    try { (void)(expr); }                                                    \
    catch(ExceptionType &) { _cm_threw = true; }                             \
    catch(...) {                                                             \
      std::fprintf(stderr, "FAIL %s:%d: " #expr                              \
                   " threw wrong exception type\n", __FILE__, __LINE__);     \
      std::exit(1);                                                          \
    }                                                                        \
    if(!_cm_threw) {                                                         \
      std::fprintf(stderr, "FAIL %s:%d: " #expr " did not throw " #ExceptionType "\n", \
                   __FILE__, __LINE__);                                      \
      std::exit(1);                                                          \
    }                                                                        \
  } while(0)

#endif
