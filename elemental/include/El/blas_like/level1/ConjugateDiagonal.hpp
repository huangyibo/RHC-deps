/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_BLAS_CONJUGATEDIAGONAL_HPP
#define EL_BLAS_CONJUGATEDIAGONAL_HPP

namespace El {

template<typename T>
void ConjugateDiagonal( Matrix<T>& A, Int offset )
{
    EL_DEBUG_CSE
    const Int iStart = Max(-offset,0);
    const Int jStart = Max( offset,0);
    const Int diagLength = A.DiagonalLength(offset);
    EL_PARALLEL_FOR
    for( Int k=0; k<diagLength; ++k )
    {
        const Int i = iStart + k;
        const Int j = jStart + k;
        A.Conjugate( i, j );
    }
}

template<typename T>
void ConjugateDiagonal( AbstractDistMatrix<T>& A, Int offset )
{
    EL_DEBUG_CSE
    const Int height = A.Height();
    const Int localWidth = A.LocalWidth();
    Matrix<T>& ALoc = A.Matrix();
    EL_PARALLEL_FOR
    for( Int jLoc=0; jLoc<localWidth; ++jLoc )
    {
        const Int j = A.GlobalCol(jLoc);
        const Int i = j - offset;
        if( i < height && A.IsLocal(i,j) )
        {
            const Int iLoc = A.LocalRow(i);
            ALoc.Conjugate( iLoc, jLoc );
        }
    }
}

#ifdef EL_INSTANTIATE_BLAS_LEVEL1
# define EL_EXTERN
#else
# define EL_EXTERN extern
#endif

#define PROTO(T) \
  EL_EXTERN template void ConjugateDiagonal \
  ( Matrix<T>& A, Int offset ); \
  EL_EXTERN template void ConjugateDiagonal \
  ( AbstractDistMatrix<T>& A, Int offset );

#define EL_ENABLE_DOUBLEDOUBLE
#define EL_ENABLE_QUADDOUBLE
#define EL_ENABLE_QUAD
#define EL_ENABLE_BIGINT
#define EL_ENABLE_BIGFLOAT
#include <El/macros/Instantiate.h>

#undef EL_EXTERN

} // namespace El

#endif // ifndef EL_BLAS_CONJUGATEDIAGONAL_HPP
