/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_BLAS_UPDATEMAPPEDDIAGONAL_HPP
#define EL_BLAS_UPDATEMAPPEDDIAGONAL_HPP

namespace El {

template<typename T,typename S>
void UpdateMappedDiagonal
(       Matrix<T>& A,
  const Matrix<S>& d,
        function<void(T&,const S&)> func,
        Int offset )
{
    EL_DEBUG_CSE
    const Int iStart = Max(-offset,0);
    const Int jStart = Max( offset,0);
    const Int diagLength = d.Height();
    EL_PARALLEL_FOR
    for( Int k=0; k<diagLength; ++k )
    {
        const Int i = iStart + k;
        const Int j = jStart + k;
        func( A(i,j), d(k) );
    }
}

template<typename T,typename S>
void UpdateMappedDiagonal
(       SparseMatrix<T>& A,
  const Matrix<S>& d,
        function<void(T&,const S&)> func,
        Int offset,
        bool diagExists )
{
    EL_DEBUG_CSE
    const Int iStart = Max(-offset,0);
    const Int jStart = Max( offset,0);
    const Int diagLength = d.Height();
    const S* dBuf = d.LockedBuffer();
    if( diagExists || A.FrozenSparsity() )
    {
        T* valBuf = A.ValueBuffer();
        for( Int k=0; k<diagLength; ++k )
        {
            const Int i = iStart + k;
            const Int j = jStart + k;
            const Int e = A.Offset( i, j );
            func( valBuf[e], dBuf[Min(i,j)]);
        }
    }
    else
    {
        A.Reserve( A.NumEntries() + diagLength );
        for( Int k=0; k<diagLength; ++k )
        {
            const Int i = iStart + k;
            const Int j = jStart + k;

            T alpha = 0;
            func( alpha, dBuf[Min(i,j)] );
            A.QueueUpdate( i, j, alpha );
        }
        A.ProcessQueues();
    }
}

template<typename T,typename S,Dist U,Dist V>
void UpdateMappedDiagonal
(       DistMatrix<T,U,V>& A,
  const AbstractDistMatrix<S>& dPre,
        function<void(T&,const S&)> func,
        Int offset )
{
    EL_DEBUG_CSE
    EL_DEBUG_ONLY(AssertSameGrids( A, dPre ))
    ElementalProxyCtrl ctrl;
    ctrl.colConstrain = true;
    ctrl.colAlign = A.DiagonalAlign(offset);
    ctrl.rootConstrain = true;
    ctrl.root = A.DiagonalRoot(offset);

    DistMatrixReadProxy<S,S,DiagCol<U,V>(),DiagRow<U,V>()> dProx( dPre, ctrl );
    auto& d = dProx.GetLocked();

    if( d.Participating() )
    {
        const Int diagShift = d.ColShift();
        const Int iStart = diagShift + Max(-offset,0);
        const Int jStart = diagShift + Max( offset,0);

        const Int colStride = A.ColStride();
        const Int rowStride = A.RowStride();
        const Int iLocStart = (iStart-A.ColShift()) / colStride;
        const Int jLocStart = (jStart-A.RowShift()) / rowStride;
        const Int iLocStride = d.ColStride() / colStride;
        const Int jLocStride = d.ColStride() / rowStride;

        const Int localDiagLength = d.LocalHeight();
        auto& ALoc = A.Matrix();
        auto& dLoc = d.LockedMatrix();
        EL_PARALLEL_FOR
        for( Int k=0; k<localDiagLength; ++k )
        {
            const Int iLoc = iLocStart + k*iLocStride;
            const Int jLoc = jLocStart + k*jLocStride;
            func( ALoc(iLoc,jLoc), dLoc(k) );
        }
    }
}

template<typename T,typename S,Dist U,Dist V>
void UpdateMappedDiagonal
(       DistMatrix<T,U,V,BLOCK>& A,
  const AbstractDistMatrix<S>& d,
        function<void(T&,const S&)> func,
        Int offset )
{
    EL_DEBUG_CSE
    EL_DEBUG_ONLY(AssertSameGrids( A, d ))
    if( d.Participating() && d.RedundantRank() == 0 )
    {
        const Int iStart = Max(-offset,0);
        const Int jStart = Max( offset,0);
        const Int localDiagLength = d.LocalHeight();
        for( Int kLoc=0; kLoc<localDiagLength; ++kLoc )
        {
            const Int k = d.GlobalRow(kLoc);
            T updateValue(0);
            func( updateValue, d.GetLocal(kLoc,0) );
            A.QueueUpdate( k+iStart, k+jStart, updateValue );
        }
    }
    A.ProcessQueues();
}

template<typename T,typename S>
void UpdateMappedDiagonal
(       DistSparseMatrix<T>& A,
  const DistMultiVec<S>& d,
        function<void(T&,const S&)> func,
        Int offset,
        bool diagExists )
{
    EL_DEBUG_CSE
    if( offset != 0 )
        LogicError("Offset assumed to be zero for distributed sparse matrices");

    const Int localHeight = A.LocalHeight();
    const S* dBuf = d.LockedMatrix().LockedBuffer();
    if( diagExists || A.FrozenSparsity() )
    {
        T* valBuf = A.ValueBuffer();
        for( Int iLoc=0; iLoc<localHeight; ++iLoc )
        {
            const Int i = A.GlobalRow(iLoc);
            const Int e = A.Offset( iLoc, i );
            func( valBuf[e], dBuf[iLoc] );
        }
    }
    else
    {
        A.Reserve( A.NumLocalEntries() + localHeight );
        for( Int iLoc=0; iLoc<localHeight; ++iLoc )
        {
            const Int i = A.GlobalRow(iLoc);
            T updateValue(0);
            func( updateValue, dBuf[iLoc] );
            A.QueueLocalUpdate( iLoc, i, updateValue );
        }
        A.ProcessLocalQueues();
    }
}

#ifdef EL_INSTANTIATE_BLAS_LEVEL1
# define EL_EXTERN
#else
# define EL_EXTERN extern
#endif

#define PROTO(T) \
  EL_EXTERN template void UpdateMappedDiagonal \
  (       Matrix<T>& A, \
    const Matrix<T>& d, \
          function<void(T&,const T&)> func, \
          Int offset ); \
  EL_EXTERN template void UpdateMappedDiagonal \
  (       SparseMatrix<T>& A, \
    const Matrix<T>& d, \
          function<void(T&,const T&)> func, \
          Int offset, \
          bool diagExists );

#define EL_ENABLE_DOUBLEDOUBLE
#define EL_ENABLE_QUADDOUBLE
#define EL_ENABLE_QUAD
#define EL_ENABLE_BIGINT
#define EL_ENABLE_BIGFLOAT
#include <El/macros/Instantiate.h>

#undef EL_EXTERN

} // namespace El

#endif // ifndef EL_BLAS_UPDATEMAPPEDDIAGONAL_HPP
