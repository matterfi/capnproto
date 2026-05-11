#pragma once

#ifdef KJ_STATIC_DEFINE
#  define KJ_API
#  define KJ_CLASS
#else
#  ifndef KJ_API
#    ifdef _WIN32
#      ifdef KJ_BUILD
#        define KJ_API __declspec(dllexport)
#      else
#        define KJ_API __declspec(dllimport)
#      endif
#      define KJ_CLASS
#    else
#      define KJ_API   __attribute__((__visibility__("default")))
#      define KJ_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
