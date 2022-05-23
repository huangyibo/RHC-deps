/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_BLAS_COPY_ALLGATHER_HPP
#define EL_BLAS_COPY_ALLGATHER_HPP

namespace El {
namespace copy {

template<typename T,Dist U,Dist V>
void AllGather
( const DistMatrix<T,        U,           V   >& A,
        DistMatrix<T,Collect<U>(),Collect<V>()>& B )
{
    EL_DEBUG_CSE
    AssertSameGrids( A, B );

    const Int height = A.Height();
    const Int width = A.Width();
    B.SetGrid( A.Grid() );
    B.Resize( height, width );

    if( A.Participating() )
    {
        if( A.DistSize() == 1 )
        {
            Copy( A.LockedMatrix(), B.Matrix() );
        }
        else
        {
            const Int colStride = A.ColStride();
            const Int rowStride = A.RowStride();
            const Int distStride = colStride*rowStride;
            const Int maxLocalHeight = MaxLength(height,colStride);
            const Int maxLocalWidth = MaxLength(width,rowStride);
            const Int portionSize = mpi::Pad( maxLocalHeight*maxLocalWidth );
            vector<T> buf;
            FastResize( buf, (distStride+1)*portionSize );
            T* sendBuf = &buf[0];
            T* recvBuf = &buf[portionSize];

            // Pack
            util::InterleaveMatrix
            ( A.LocalHeight(), A.LocalWidth(),
              A.LockedBuffer(), 1, A.LDim(),
              sendBuf,          1, A.LocalHeight() );

            // Communicate
            mpi::AllGather
            ( sendBuf, portionSize, recvBuf, portionSize, A.DistComm() );

            // Unpack
            util::StridedUnpack
            ( height, width,
              A.ColAlign(), colStride,
              A.RowAlign(), rowStride,
              recvBuf, portionSize,
              B.Buffer(), B.LDim() );
        }
    }
    if( A.Grid().InGrid() && A.CrossComm() != mpi::COMM_SELF )
        El::Broadcast( B, A.CrossComm(), A.Root() );
}

template<typename T,Dist U,Dist V>
void AllGather
( const DistMatrix<T,        U,           V   ,BLOCK>& A,
        DistMatrix<T,Collect<U>(),Collect<V>(),BLOCK>& B )
{
    EL_DEBUG_CSE
    AssertSameGrids( A, B );
    // TODO(poulson): More efficient implementation
    GeneralPurpose( A, B );
}

} // namespace copy
} // namespace El

#endif // ifndef EL_BLAS_COPY_ALLGATHER_HPP
