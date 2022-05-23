/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_CONFIG_H
#define EL_CONFIG_H

/* Build type and version information */
#define EL_GIT_SHA1 "6eb15a0da2a4998bf1cf971ae231b78e06d989d9"
#define EL_VERSION_MAJOR "0"
#define EL_VERSION_MINOR "88-dev"
#define EL_CMAKE_BUILD_TYPE "Release"
#define EL_RELEASE
/* #undef EL_HYBRID */
#define BUILD_SHARED_LIBS
/* #undef MSVC */

/* Compiler information */
#define EL_CMAKE_C_COMPILER   "/opt/rh/devtoolset-7/root/usr/bin/cc"
#define EL_CMAKE_CXX_COMPILER "/opt/rh/devtoolset-7/root/usr/bin/c++"
#define EL_CXX_FLAGS          "-O3 -fcx-fortran-rules -std=gnu++14 "
#define EL_HAVE_F90_INTERFACE
#ifdef EL_HAVE_F90_INTERFACE
# include "El/FCMangle.h"
#endif
#define EL_FORT_LOGICAL int
#define EL_FORT_TRUE    1
#define EL_FORT_FALSE   0

/* Message Passing Interface */
#define EL_MPI_C_COMPILER        "/home/alchemist/install/openmpi/bin/mpicc"
#define EL_MPI_C_INCLUDE_PATH    "/home/alchemist/install/openmpi/include"
#define EL_MPI_C_COMPILE_FLAGS   ""
#define EL_MPI_C_LIBRARIES       "/home/alchemist/install/openmpi/lib/libmpi.so"
#define EL_MPI_C_LINK_FLAGS      "-Wl,-rpath -Wl,/home/alchemist/install/openmpi/lib -Wl,--enable-new-dtags -pthread"
#define EL_MPI_CXX_COMPILER      "/home/alchemist/install/openmpi/bin/mpicxx"
#define EL_MPI_CXX_INCLUDE_PATH  "/home/alchemist/install/openmpi/include"
#define EL_MPI_CXX_COMPILE_FLAGS ""
#define EL_MPI_CXX_LIBRARIES     "/home/alchemist/install/openmpi/lib/libmpi.so"
#define EL_MPI_CXX_LINK_FLAGS    "-Wl,-rpath -Wl,/home/alchemist/install/openmpi/lib -Wl,--enable-new-dtags -pthread"

/* Math libraries */
#define EL_MATH_LIBS "/usr/lib/libopenblas.so;/usr/lib/gcc/x86_64-redhat-linux/4.8.5/libgfortran.so;-lpthread;/usr/lib64/libm.so"

/* #undef EL_BUILT_BLIS_LAPACK */
#define EL_HAVE_OPENBLAS
/* #undef EL_BUILT_OPENBLAS */
#define EL_HAVE_BLAS_SUFFIX
#define EL_HAVE_LAPACK_SUFFIX
#ifdef EL_HAVE_BLAS_SUFFIX
# define EL_BLAS_SUFFIX _
#endif
#ifdef EL_HAVE_LAPACK_SUFFIX
# define EL_LAPACK_SUFFIX _
#endif

/* #undef EL_HAVE_SCALAPACK */
/* #undef EL_BUILT_SCALAPACK */
/* #undef EL_HAVE_SCALAPACK_SUFFIX */
#ifdef EL_HAVE_SCALAPACK_SUFFIX
 #define EL_SCALAPACK_SUFFIX 
#endif

/* #undef EL_HAVE_FLA_BSVD */
/* #undef EL_HAVE_QUAD */
/* #undef EL_HAVE_QUADMATH */
/* #undef EL_HAVE_QD */
/* #undef EL_HAVE_MPC */
/* #undef EL_HAVE_MKL */
/* #undef EL_HAVE_MKL_GEMMT */
#define EL_DISABLE_MKL_CSRMV

/* Miscellaneous configuration options */
#define EL_RESTRICT __restrict__
#define EL_HAVE_PRETTY_FUNCTION
#define EL_HAVE_OPENMP
#define EL_HAVE_OMP_COLLAPSE
#define EL_HAVE_OMP_SIMD
/* #undef EL_HAVE_QT5 */
#define EL_AVOID_COMPLEX_MPI
#define EL_HAVE_CXX11RANDOM
#define EL_HAVE_STEADYCLOCK
#define EL_HAVE_NOEXCEPT
#define EL_HAVE_MPI_REDUCE_SCATTER_BLOCK
#define EL_HAVE_MPI_LONG_LONG
#define EL_HAVE_MPI_LONG_DOUBLE
/* #undef EL_HAVE_MPI_LONG_DOUBLE_COMPLEX */
#define EL_HAVE_MPI_C_COMPLEX
#define EL_HAVE_MPI_COMM_SET_ERRHANDLER
#define EL_HAVE_MPI_INIT_THREAD
#define EL_HAVE_MPI_QUERY_THREAD
#define EL_HAVE_MPI3_NONBLOCKING_COLLECTIVES
/* #undef EL_HAVE_MPIX_NONBLOCKING_COLLECTIVES */
/* #undef EL_REDUCE_SCATTER_BLOCK_VIA_ALLREDUCE */
#define EL_USE_BYTE_ALLGATHERS
/* #undef EL_USE_64BIT_INTS */
/* #undef EL_USE_64BIT_BLAS_INTS */

/* Sparse-direct configuration */
#define EL_USE_CUSTOM_ALLTOALLV
#define EL_HAVE_METIS
#define EL_HAVE_PARMETIS

/* Advanced configuration options */
/* #undef EL_ZERO_INIT */
/* #undef EL_CACHE_WARNINGS */
/* #undef EL_UNALIGNED_WARNINGS */
/* #undef EL_VECTOR_WARNINGS */
/* #undef EL_AVOID_OMP_FMA */

#ifdef BUILD_SHARED_LIBS
# if defined _WIN32 || defined __CYGWIN__
#  ifdef __GNUC__ /* Compiling with GNU on Windows */
#   ifdef EL_EXPORTS
#    define EL_EXPORT __attribute__ ((dllexport))
#   else
#    define EL_EXPORT __attribute__ ((dllimport))
#   endif
#  else  /* Compiling with non-GNU on Windows (check for MSVC?) */
#   ifdef EL_EXPORTS
#    define EL_EXPORT __declspec(dllexport)
#   else
#    define EL_EXPORT __declspec(dllimport)
#   endif
#  endif
#  define EL_LOCAL
# else
#  if __GNUC__ >= 4
#   define EL_EXPORT __attribute__ ((visibility ("default")))
#   define EL_LOCAL  __attribute__ ((visibility ("hidden")))
#  else
#   define EL_EXPORT
#   define EL_LOCAL
#  endif
# endif
#else
# define EL_EXPORT
# define EL_LOCAL
#endif

/* #undef EL_HAVE_VALGRIND */

/* METIS/ParMETIS */
#ifdef BUILD_SHARED_LIBS
# if defined _WIN32 || defined __CYGWIN__
#  ifdef __GNUC__ /* Compiling with GNU on Windows */
#   define METIS_EXPORT __attribute__ ((dllimport))
#  else  /* Compiling with non-GNU on Windows (check for MSVC?) */
#   define METIS_EXPORT __declspec(dllimport)
#  endif
# else
#  define METIS_EXPORT
# endif
#else
# define METIS_EXPORT
#endif

#endif /* EL_CONFIG_H */
