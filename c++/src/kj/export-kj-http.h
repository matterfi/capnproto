#pragma once

#ifdef KJ_HTTP_STATIC_DEFINE
#  define KJ_HTTP_API
#  define KJ_HTTP_CLASS
#else
#  ifndef KJ_HTTP_API
#    ifdef _WIN32
#      ifdef KJ_HTTP_BUILD
#        define KJ_HTTP_API __declspec(dllexport)
#      else
#        define KJ_HTTP_API __declspec(dllimport)
#      endif
#      define KJ_HTTP_CLASS
#    else
#      define KJ_HTTP_API   __attribute__((__visibility__("default")))
#      define KJ_HTTP_CLASS __attribute__((__visibility__("default")))
#    endif
#  endif
#endif
