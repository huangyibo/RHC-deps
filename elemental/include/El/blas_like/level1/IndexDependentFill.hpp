/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_BLAS_INDEXDEPENDENTFILL_HPP
#define EL_BLAS_INDEXDEPENDENTFILL_HPP

namespace El {

template<typename T>
void IndexDependentFill( Matrix<T>& A, function<T(Int,Int)> func )
{
    EL_DEBUG_CSE
    const Int m = A.Height();
    const Int n = A.Width();
    T* ABuf = A.Buffer();
    const Int ALDim = A.LDim();

    // Use entry-wise parallelization for column vectors. Otherwise
    // use column-wise parallelization.
    if( n == 1 )
    {
        EL_PARALLEL_FOR
        for( Int i=0; i<m; ++i )
        {
            ABuf[i] = func(i,0);
        }
    }
    else
    {
        EL_PARALLEL_FOR
        for( Int j=0; j<n; ++j )
        {
            EL_SIMD
            for( Int i=0; i<m; ++i )
            {
                ABuf[i+j*ALDim] = func(i,j);
            }
        }
    }

}

template<typename T>
void IndexDependentFill
( AbstractDistMatrix<T>& A, function<T(Int,Int)> func )
{
    EL_DEBUG_CSE
    const Int mLoc = A.LocalHeight();
    const Int nLoc = A.LocalWidth();
    T* ALocBuf = A.Buffer();
    const Int ALocLDim = A.LDim();

    // Use entry-wise parallelization for column vectors. Otherwise
    // use column-wise parallelization.
    if( nLoc == 1 )
    {
        EL_PARALLEL_FOR
        for( Int iLoc=0; iLoc<mLoc; ++iLoc )
        {
            const Int i = A.GlobalRow(iLoc);
            const Int j = A.GlobalCol(0);
            ALocBuf[iLoc] = func(i,j);
        }
    }
    else
    {
        EL_PARALLEL_FOR
        for( Int jLoc=0; jLoc<nLoc; ++jLoc )
        {
            EL_SIMD
            for( Int iLoc=0; iLoc<mLoc; ++iLoc )
            {
                const Int i = A.GlobalRow(iLoc);
                const Int j = A.GlobalCol(jLoc);
                ALocBuf[iLoc+jLoc*ALocLDim] = func(i,j);
            }
        }
    }

}

#ifdef EL_INSTANTIATE_BLAS_LEVEL1
# define EL_EXTERN
#else
# define EL_EXTERN extern
#endif

#define PROTO(T) \
  EL_EXTERN template void IndexDependentFill \
  ( Matrix<T>& A, function<T(Int,Int)> func ); \
  EL_EXTERN template void IndexDependentFill \
  ( AbstractDistMatrix<T>& A, function<T(Int,Int)> func );

#define EL_ENABLE_DOUBLEDOUBLE
#define EL_ENABLE_QUADDOUBLE
#define EL_ENABLE_QUAD
#define EL_ENABLE_BIGINT
#define EL_ENABLE_BIGFLOAT
#include <El/macros/Instantiate.h>

#undef EL_EXTERN

} // namespace El

#endif // ifndef EL_BLAS_INDEXDEPENDENTFILL_HPP
