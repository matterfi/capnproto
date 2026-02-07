#pragma once

#ifdef KJ_ASYNC_STATIC_DEFINE
#  define KJ_ASYNC_API
#  define KJ_ASYNC_CLASS
#else
#  ifndef KJ_ASYNC_API
#    ifdef _WIN32
#      ifdef KJ_ASYNC_BUILD
#        define KJ_ASYNC_API __declspec(dllexport)
#      else
#        define KJ_ASYNC_API __declspec(dllimport)
#      endif
#      define KJ_ASYNC_CLASS
#    else
#      define KJ_ASYNC_API   __attribute__((__visibility__("default")))
#      define KJ_ASYNC_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
