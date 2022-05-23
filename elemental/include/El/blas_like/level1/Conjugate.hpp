/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_BLAS_CONJUGATE_HPP
#define EL_BLAS_CONJUGATE_HPP

namespace El {

template<typename Real>
void Conjugate( Matrix<Real>& A )
{ }

template<typename Real>
void Conjugate( Matrix<Complex<Real>>& A )
{
    EL_DEBUG_CSE
    auto conj = [](const Complex<Real> &alpha) { return Conj(alpha); };
    EntrywiseMap( A, MakeFunction(conj) );
}

template<typename T>
void Conjugate( const Matrix<T>& A, Matrix<T>& B )
{
    EL_DEBUG_CSE
    Copy( A, B );
    Conjugate( B );
}

template<typename T>
void Conjugate( AbstractDistMatrix<T>& A )
{
    EL_DEBUG_CSE
    Conjugate( A.Matrix() );
}

template<typename T>
void Conjugate( const AbstractDistMatrix<T>& A, AbstractDistMatrix<T>& B )
{
    EL_DEBUG_CSE
    Copy( A, B );
    Conjugate( B );
}

#ifdef EL_INSTANTIATE_BLAS_LEVEL1
# define EL_EXTERN
#else
# define EL_EXTERN extern
#endif

#define PROTO(T) \
  EL_EXTERN template void Conjugate \
  ( Matrix<T>& A ); \
  EL_EXTERN template void Conjugate \
  ( const Matrix<T>& A, Matrix<T>& B ); \
  EL_EXTERN template void Conjugate \
  ( AbstractDistMatrix<T>& A ); \
  EL_EXTERN template void Conjugate \
  ( const AbstractDistMatrix<T>& A, AbstractDistMatrix<T>& B );

#define EL_ENABLE_DOUBLEDOUBLE
#define EL_ENABLE_QUADDOUBLE
#define EL_ENABLE_QUAD
#define EL_ENABLE_BIGINT
#define EL_ENABLE_BIGFLOAT
#include <El/macros/Instantiate.h>

#undef EL_EXTERN

} // namespace El

#endif // ifndef EL_BLAS_CONJUGATE_HPP
