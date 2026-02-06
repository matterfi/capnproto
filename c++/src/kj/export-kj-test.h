#pragma once

#ifdef KJ_TEST_STATIC_DEFINE
#  define KJ_TEST_API
#  define KJ_TEST_CLASS
#else
#  ifndef KJ_TEST_API
#    ifdef _WIN32
#      ifdef KJ_TEST_BUILD
#        define KJ_TEST_API __declspec(dllexport)
#      else
#        define KJ_TEST_API __declspec(dllimport)
#      endif
#      define KJ_TEST_CLASS
#    else
#      define KJ_TEST_API   __attribute__((__visibility__("default")))
#      define KJ_TEST_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
