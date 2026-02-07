#pragma once

#ifdef KJ_TLS_STATIC_DEFINE
#  define KJ_TLS_API
#  define KJ_TLS_CLASS
#else
#  ifndef KJ_TLS_API
#    ifdef _WIN32
#      ifdef KJ_TLS_BUILD
#        define KJ_TLS_API __declspec(dllexport)
#      else
#        define KJ_TLS_API __declspec(dllimport)
#      endif
#      define KJ_TLS_CLASS
#    else
#      define KJ_TLS_API   __attribute__((__visibility__("default")))
#      define KJ_TLS_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
