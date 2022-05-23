/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_BLAS_COPY_SCATTER_HPP
#define EL_BLAS_COPY_SCATTER_HPP

namespace El {
namespace copy {

template<typename T>
void Scatter
( const DistMatrix<T,CIRC,CIRC>& A,
        ElementalMatrix<T>& B )
{
    EL_DEBUG_CSE
    AssertSameGrids( A, B );

    const Int m = A.Height();
    const Int n = A.Width();
    const Int colStride = B.ColStride();
    const Int rowStride = B.RowStride();
    B.Resize( m, n );
    if( B.CrossSize() != 1 || B.RedundantSize() != 1 )
    {
        // TODO(poulson):
        // Broadcast over the redundant communicator and use mpi::Translate
        // rank to determine whether a process is the root of the broadcast.
        GeneralPurpose( A, B );
        return;
    }

    const Int pkgSize = mpi::Pad(MaxLength(m,colStride)*MaxLength(n,rowStride));
    const Int recvSize = pkgSize;
    const Int sendSize = B.DistSize()*pkgSize;

    // Translate the root of A into the DistComm of B (if possible)
    const Int root = A.Root();
    const Int target = mpi::Translate( A.CrossComm(), root, B.DistComm() );
    if( target == mpi::UNDEFINED )
        return;

    if( B.DistSize() == 1 )
    {
        Copy( A.LockedMatrix(), B.Matrix() );
        return;
    }

    vector<T> buffer;
    T* recvBuf=0; // some compilers (falsely) warn otherwise
    if( A.CrossRank() == root )
    {
        FastResize( buffer, sendSize+recvSize );
        T* sendBuf = &buffer[0];
        recvBuf    = &buffer[sendSize];

        // Pack the send buffer
        copy::util::StridedPack
        ( m, n,
          B.ColAlign(), colStride,
          B.RowAlign(), rowStride,
          A.LockedBuffer(), A.LDim(),
          sendBuf,          pkgSize );

        // Scatter from the root
        mpi::Scatter
        ( sendBuf, pkgSize, recvBuf, pkgSize, target, B.DistComm() );
    }
    else
    {
        FastResize( buffer, recvSize );
        recvBuf = &buffer[0];

        // Perform the receiving portion of the scatter from the non-root
        mpi::Scatter
        ( static_cast<T*>(0), pkgSize,
          recvBuf,            pkgSize, target, B.DistComm() );
    }

    // Unpack
    copy::util::InterleaveMatrix
    ( B.LocalHeight(), B.LocalWidth(),
      recvBuf,    1, B.LocalHeight(),
      B.Buffer(), 1, B.LDim() );
}

template<typename T>
void Scatter
( const DistMatrix<T,CIRC,CIRC,BLOCK>& A,
        BlockMatrix<T>& B )
{
    EL_DEBUG_CSE
    AssertSameGrids( A, B );
    // TODO(poulson): More efficient implementation
    GeneralPurpose( A, B );
}

template<typename T>
void Scatter
( const DistMatrix<T,CIRC,CIRC>& A,
        DistMatrix<T,STAR,STAR>& B )
{
    EL_DEBUG_CSE
    AssertSameGrids( A, B );
    B.Resize( A.Height(), A.Width() );
    if( B.Participating() )
    {
        if( A.Participating() )
            B.Matrix() = A.LockedMatrix();
        El::Broadcast( B, A.CrossComm(), A.Root() );
    }
}

template<typename T>
void Scatter
( const DistMatrix<T,CIRC,CIRC,BLOCK>& A,
        DistMatrix<T,STAR,STAR,BLOCK>& B )
{
    EL_DEBUG_CSE
    AssertSameGrids( A, B );
    B.Resize( A.Height(), A.Width() );
    if( B.Participating() )
    {
        if( A.Participating() )
            B.Matrix() = A.LockedMatrix();
        El::Broadcast( B, A.CrossComm(), A.Root() );
    }
}

} // namespace copy
} // namespace El

#endif // ifndef EL_BLAS_COPY_SCATTER_HPP
