#ifndef _CONFIG_H_
#define _CONFG_H_

#ifdef DEBUG
#define __USE_TRACE__
#define DEBUG_CODE 1
#endif

#undef _DEBUG

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#undef HAVE_LONG_LONG

// TBD: clean up each category of warning starting from end (most severe)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wdelete-incomplete"
#pragma GCC diagnostic ignored "-Wunused-value"

#if defined(__x86_64__)
#define __EXTRA__
#endif

#endif /* _CONFIG_H_ */
