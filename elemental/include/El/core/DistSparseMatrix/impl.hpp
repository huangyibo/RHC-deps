/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   Copyright (c) 2012 Jack Poulson, Lexing Ying, and
   The University of Texas at Austin.
   All rights reserved.

   Copyright (c) 2013 Jack Poulson, Lexing Ying, and Stanford University.
   All rights reserved.

   Copyright (c) 2014 Jack Poulson and The Georgia Institute of Technology.
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_CORE_DISTSPARSEMATRIX_IMPL_HPP
#define EL_CORE_DISTSPARSEMATRIX_IMPL_HPP

#include <El/blas_like/level1/Axpy.hpp>
#include <El/blas_like/level1/Scale.hpp>

namespace El {

// Constructors and destructors
// ============================

template<typename Ring>
DistSparseMatrix<Ring>::DistSparseMatrix( const El::Grid& grid )
: distGraph_(grid)
{ }

template<typename Ring>
DistSparseMatrix<Ring>::DistSparseMatrix
( Int height, Int width, const El::Grid& grid )
: distGraph_(height,width,grid)
{ }

template<typename Ring>
DistSparseMatrix<Ring>::DistSparseMatrix( const DistSparseMatrix<Ring>& A )
{
    EL_DEBUG_CSE
    distGraph_.numSources_ = -1;
    distGraph_.numTargets_ = -1;
    distGraph_.grid_ = &A.Grid();
    if( &A != this )
        *this = A;
    EL_DEBUG_ONLY(
      else
          LogicError("Tried to construct DistMultiVec via itself");
    )
}

template<typename Ring>
DistSparseMatrix<Ring>::~DistSparseMatrix()
{ }

// Assignment and reconfiguration
// ==============================

// Change the matrix size
// ----------------------
template<typename Ring>
void DistSparseMatrix<Ring>::Empty( bool freeMemory )
{
    EL_DEBUG_CSE
    distGraph_.Empty( freeMemory );
    if( freeMemory )
        SwapClear( vals_ );
    else
        vals_.resize( 0 );
    distGraph_.multMeta.Clear();

    SwapClear( remoteVals_ );
}

template<typename Ring>
void DistSparseMatrix<Ring>::Resize( Int height, Int width )
{
    EL_DEBUG_CSE
    distGraph_.Resize( height, width );
    vals_.resize( 0 );
    SwapClear( remoteVals_ );
}

// Change the distribution
// -----------------------
template<typename Ring>
void DistSparseMatrix<Ring>::SetGrid( const El::Grid& grid )
{
    EL_DEBUG_CSE
    if( distGraph_.grid_ == &grid )
        return;
    distGraph_.SetGrid( grid );
    vals_.resize( 0 );
    SwapClear( remoteVals_ );
}

// Assembly
// --------
template<typename Ring>
void DistSparseMatrix<Ring>::Reserve( Int numLocalEntries, Int numRemoteEntries )
{
    const Int currSize = vals_.size();
    const Int currRemoteSize = remoteVals_.size();

    distGraph_.Reserve( numLocalEntries, numRemoteEntries );
    vals_.reserve( currSize+numLocalEntries );
    remoteVals_.reserve( currRemoteSize+numRemoteEntries );
}

template<typename Ring>
void DistSparseMatrix<Ring>::FreezeSparsity() EL_NO_EXCEPT
{ distGraph_.frozenSparsity_ = true; }
template<typename Ring>
void DistSparseMatrix<Ring>::UnfreezeSparsity() EL_NO_EXCEPT
{ distGraph_.frozenSparsity_ = false; }
template<typename Ring>
bool DistSparseMatrix<Ring>::FrozenSparsity() const EL_NO_EXCEPT
{ return distGraph_.frozenSparsity_; }

template<typename Ring>
void DistSparseMatrix<Ring>::Update( Int row, Int col, const Ring& value )
{
    EL_DEBUG_CSE
    QueueUpdate( row, col, value, true );
    ProcessLocalQueues();
}

template<typename Ring>
void DistSparseMatrix<Ring>::Update( const Entry<Ring>& entry )
{ Update( entry.i, entry.j, entry.value ); }

template<typename Ring>
void DistSparseMatrix<Ring>::UpdateLocal
( Int localRow, Int col, const Ring& value )
{
    EL_DEBUG_CSE
    QueueLocalUpdate( localRow, col, value );
    ProcessLocalQueues();
}

template<typename Ring>
void DistSparseMatrix<Ring>::UpdateLocal( const Entry<Ring>& localEntry )
{ UpdateLocal( localEntry.i, localEntry.j, localEntry.value ); }

template<typename Ring>
void DistSparseMatrix<Ring>::Zero( Int row, Int col )
{
    EL_DEBUG_CSE
    QueueZero( row, col, true );
    ProcessLocalQueues();
}

template<typename Ring>
void DistSparseMatrix<Ring>::ZeroLocal( Int localRow, Int col )
{
    EL_DEBUG_CSE
    QueueLocalZero( localRow, col );
    ProcessLocalQueues();
}

template<typename Ring>
void DistSparseMatrix<Ring>::QueueUpdate
( Int row, Int col, const Ring& value, bool passive )
EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    // TODO(poulson): Use FrozenSparsity()
    if( row == END ) row = Height() - 1;
    if( col == END ) col = Width() - 1;
    if( row >= FirstLocalRow() && row < FirstLocalRow()+LocalHeight() )
    {
        QueueLocalUpdate( row-FirstLocalRow(), col, value );
    }
    else if( !passive )
    {
        distGraph_.remoteSources_.push_back( row );
        distGraph_.remoteTargets_.push_back( col );
        remoteVals_.push_back( value );
    }
}

template<typename Ring>
void DistSparseMatrix<Ring>::QueueUpdate
( const Entry<Ring>& entry, bool passive )
EL_NO_RELEASE_EXCEPT
{ QueueUpdate( entry.i, entry.j, entry.value, passive ); }

template<typename Ring>
void DistSparseMatrix<Ring>::QueueLocalUpdate
( Int localRow, Int col, const Ring& value ) EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    if( FrozenSparsity() )
    {
        const Int offset = distGraph_.Offset( localRow, col );
        vals_[offset] += value;
    }
    else
    {
        distGraph_.QueueLocalConnection( localRow, col );
        vals_.push_back( value );
        distGraph_.multMeta.ready = false;
    }
}

template<typename Ring>
void DistSparseMatrix<Ring>::QueueLocalUpdate( const Entry<Ring>& localEntry )
EL_NO_RELEASE_EXCEPT
{ QueueLocalUpdate( localEntry.i, localEntry.j, localEntry.value ); }

template<typename Ring>
void DistSparseMatrix<Ring>::QueueZero( Int row, Int col, bool passive )
EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    if( row == END ) row = Height() - 1;
    if( col == END ) col = Width() - 1;
    if( row >= FirstLocalRow() && row < FirstLocalRow()+LocalHeight() )
        QueueLocalZero( row-FirstLocalRow(), col );
    else if( !passive )
        distGraph_.remoteRemovals_.push_back( pair<Int,Int>(row,col) );
}

template<typename Ring>
void DistSparseMatrix<Ring>::QueueLocalZero( Int localRow, Int col )
EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    if( FrozenSparsity() )
    {
        const Int offset = distGraph_.Offset( localRow, col );
        vals_[offset] = 0;
    }
    else
    {
        distGraph_.QueueLocalDisconnection( localRow, col );
        distGraph_.multMeta.ready = false;
    }
}

template<typename Ring>
void DistSparseMatrix<Ring>::ProcessQueues()
{
    EL_DEBUG_CSE
    EL_DEBUG_ONLY(
      if( distGraph_.sources_.size() != distGraph_.targets_.size() ||
          distGraph_.targets_.size() != vals_.size() )
          LogicError("Inconsistent sparse matrix buffer sizes");
    )

    // Send the remote updates
    // =======================
    mpi::Comm comm = distGraph_.grid_->Comm();
    const int commSize = distGraph_.grid_->Size();
    {
        // Compute the send counts
        // -----------------------
        vector<int> sendCounts(commSize,0);
        for( auto s : distGraph_.remoteSources_ )
            ++sendCounts[RowOwner(s)];

        // Pack the send data
        // ------------------
        vector<int> sendOffs;
        const int totalSend = Scan( sendCounts, sendOffs );
        auto offs = sendOffs;
        vector<Entry<Ring>> sendBuf(totalSend);
        for( Int i=0; i<totalSend; ++i )
        {
            const int owner = RowOwner(distGraph_.remoteSources_[i]);
            sendBuf[offs[owner]++] =
                Entry<Ring>
                { distGraph_.remoteSources_[i],
                  distGraph_.remoteTargets_[i], remoteVals_[i] };
        }
        SwapClear( distGraph_.remoteSources_ );
        SwapClear( distGraph_.remoteTargets_ );
        SwapClear( remoteVals_ );
        // Exchange and unpack
        // -------------------
        auto recvBuf = mpi::AllToAll( sendBuf, sendCounts, sendOffs, comm );
        if( !FrozenSparsity() )
            Reserve( NumLocalEntries()+recvBuf.size() );
        for( auto& entry : recvBuf )
            QueueUpdate( entry );
    }

    // Send the remote entry removals
    // ==============================
    {
        // Compute the send counts
        // -----------------------
        vector<int> sendCounts(commSize,0);
        const Int numRemoteRemovals = distGraph_.remoteRemovals_.size();
        for( Int i=0; i<numRemoteRemovals; ++i )
            ++sendCounts[RowOwner(distGraph_.remoteRemovals_[i].first)];
        // Pack the send data
        // ------------------
        vector<int> sendOffs;
        const int totalSend = Scan( sendCounts, sendOffs );
        auto offs = sendOffs;
        vector<Int> sendRows(totalSend), sendCols(totalSend);
        for( Int i=0; i<totalSend; ++i )
        {
            const int owner = RowOwner(distGraph_.remoteRemovals_[i].first);
            sendRows[offs[owner]] = distGraph_.remoteRemovals_[i].first;
            sendCols[offs[owner]] = distGraph_.remoteRemovals_[i].second;
            ++offs[owner];
        }
        SwapClear( distGraph_.remoteRemovals_ );
        // Exchange and unpack
        // -------------------
        auto recvRows = mpi::AllToAll(sendRows,sendCounts,sendOffs,comm);
        auto recvCols = mpi::AllToAll(sendCols,sendCounts,sendOffs,comm);
        const Int totalRecv = recvRows.size();
        for( Int i=0; i<totalRecv; ++i )
            QueueZero( recvRows[i], recvCols[i] );
    }

    // Ensure that the kept local triplets are sorted and combined
    // ===========================================================
    ProcessLocalQueues();
}

template<typename Ring>
void DistSparseMatrix<Ring>::ProcessLocalQueues()
{
    EL_DEBUG_CSE
    if( distGraph_.locallyConsistent_ )
        return;

    Int numRemoved = 0;
    const Int numLocalEntries = vals_.size();
    vector<Entry<Ring>> entries( numLocalEntries );
    if( distGraph_.markedForRemoval_.size() != 0 )
    {
        for( Int s=0; s<numLocalEntries; ++s )
        {
            pair<Int,Int> candidate(distGraph_.sources_[s],
                                    distGraph_.targets_[s]);
            if( distGraph_.markedForRemoval_.find(candidate) ==
                distGraph_.markedForRemoval_.end() )
            {
                entries[s-numRemoved].i = distGraph_.sources_[s];
                entries[s-numRemoved].j = distGraph_.targets_[s];
                entries[s-numRemoved].value = vals_[s];
            }
            else
            {
                ++numRemoved;
            }
        }
        SwapClear( distGraph_.markedForRemoval_ );
        entries.resize( numLocalEntries-numRemoved );
    }
    else
    {
        for( Int s=0; s<numLocalEntries; ++s )
            entries[s] = Entry<Ring>{distGraph_.sources_[s],
                                  distGraph_.targets_[s],vals_[s]};
    }
    std::sort( entries.begin(), entries.end(), CompareEntries );
    const Int numSorted = entries.size();

    // Combine duplicates
    // ------------------
    Int lastUnique=0;
    for( Int s=1; s<numSorted; ++s )
    {
        if( entries[s].i != entries[lastUnique].i ||
            entries[s].j != entries[lastUnique].j )
            entries[++lastUnique] = entries[s];
        else
            entries[lastUnique].value += entries[s].value;
    }
    const Int numUnique = lastUnique+1;

    entries.resize( numUnique );
    distGraph_.sources_.resize( numUnique );
    distGraph_.targets_.resize( numUnique );
    vals_.resize( numUnique );
    for( Int s=0; s<numUnique; ++s )
    {
        distGraph_.sources_[s] = entries[s].i;
        distGraph_.targets_[s] = entries[s].j;
        vals_[s] = entries[s].value;
    }
    distGraph_.ComputeSourceOffsets();
    distGraph_.locallyConsistent_ = true;
}

// Operator overloading
// ====================

// Make a copy
// -----------
template<typename Ring>
const DistSparseMatrix<Ring>&
DistSparseMatrix<Ring>::operator=( const DistSparseMatrix<Ring>& A )
{
    EL_DEBUG_CSE
    distGraph_ = A.distGraph_;
    vals_ = A.vals_;
    remoteVals_ = A.remoteVals_;
    return *this;
}

// Make a copy of a submatrix
// --------------------------
template<typename Ring>
DistSparseMatrix<Ring>
DistSparseMatrix<Ring>::operator()
( Range<Int> I, Range<Int> J ) const
{
    EL_DEBUG_CSE
    DistSparseMatrix<Ring> ASub(this->Grid());
    GetSubmatrix( *this, I, J, ASub );
    return ASub;
}

template<typename Ring>
DistSparseMatrix<Ring>
DistSparseMatrix<Ring>::operator()
( const vector<Int>& I, Range<Int> J ) const
{
    EL_DEBUG_CSE
    DistSparseMatrix<Ring> ASub(this->Grid());
    GetSubmatrix( *this, I, J, ASub );
    return ASub;
}

template<typename Ring>
DistSparseMatrix<Ring>
DistSparseMatrix<Ring>::operator()
( Range<Int> I, const vector<Int>& J ) const
{
    EL_DEBUG_CSE
    DistSparseMatrix<Ring> ASub(this->Grid());
    GetSubmatrix( *this, I, J, ASub );
    return ASub;
}

template<typename Ring>
DistSparseMatrix<Ring>
DistSparseMatrix<Ring>::operator()
( const vector<Int>& I, const vector<Int>& J ) const
{
    EL_DEBUG_CSE
    DistSparseMatrix<Ring> ASub(this->Grid());
    GetSubmatrix( *this, I, J, ASub );
    return ASub;
}

// Rescaling
// ---------
template<typename Ring>
const DistSparseMatrix<Ring>& DistSparseMatrix<Ring>::operator*=
( const Ring& alpha )
{
    EL_DEBUG_CSE
    Scale( alpha, *this );
    return *this;
}

// Addition/subtraction
// --------------------
template<typename Ring>
const DistSparseMatrix<Ring>&
DistSparseMatrix<Ring>::operator+=( const DistSparseMatrix<Ring>& A )
{
    EL_DEBUG_CSE
    Axpy( Ring(1), A, *this );
    return *this;
}

template<typename Ring>
const DistSparseMatrix<Ring>&
DistSparseMatrix<Ring>::operator-=( const DistSparseMatrix<Ring>& A )
{
    EL_DEBUG_CSE
    Axpy( Ring(-1), A, *this );
    return *this;
}

// Queries
// =======

// High-level information
// ----------------------
template<typename Ring>
Int DistSparseMatrix<Ring>::Height() const EL_NO_EXCEPT
{ return distGraph_.NumSources(); }
template<typename Ring>
Int DistSparseMatrix<Ring>::Width() const EL_NO_EXCEPT
{ return distGraph_.NumTargets(); }
template<typename Ring>
Int DistSparseMatrix<Ring>::NumEntries() const EL_NO_EXCEPT
{ return distGraph_.NumEdges(); }

template<typename Ring>
El::DistGraph& DistSparseMatrix<Ring>::DistGraph() EL_NO_EXCEPT
{ return distGraph_; }
template<typename Ring>
const El::DistGraph& DistSparseMatrix<Ring>::LockedDistGraph()
const EL_NO_EXCEPT
{ return distGraph_; }

template<typename Ring>
Int DistSparseMatrix<Ring>::FirstLocalRow() const EL_NO_EXCEPT
{ return distGraph_.FirstLocalSource(); }

template<typename Ring>
Int DistSparseMatrix<Ring>::LocalHeight() const EL_NO_EXCEPT
{ return distGraph_.NumLocalSources(); }

template<typename Ring>
Int DistSparseMatrix<Ring>::NumLocalEntries() const EL_NO_EXCEPT
{ return distGraph_.NumLocalEdges(); }

template<typename Ring>
Int DistSparseMatrix<Ring>::Capacity() const EL_NO_EXCEPT
{ return distGraph_.Capacity(); }

template<typename Ring>
bool DistSparseMatrix<Ring>::LocallyConsistent() const EL_NO_EXCEPT
{ return distGraph_.LocallyConsistent(); }

// Distribution information
// ------------------------
template<typename Ring>
const El::Grid& DistSparseMatrix<Ring>::Grid() const EL_NO_EXCEPT
{ return distGraph_.Grid(); }

template<typename Ring>
Int DistSparseMatrix<Ring>::Blocksize() const EL_NO_EXCEPT
{ return distGraph_.Blocksize(); }

template<typename Ring>
int DistSparseMatrix<Ring>::RowOwner( Int i ) const EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    return distGraph_.SourceOwner(i);
}

template<typename Ring>
Int DistSparseMatrix<Ring>::GlobalRow( Int iLoc ) const EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    return distGraph_.GlobalSource(iLoc);
}

template<typename Ring>
Int DistSparseMatrix<Ring>::LocalRow( Int i ) const EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    return distGraph_.LocalSource(i);
}

// Detailed local information
// --------------------------
template<typename Ring>
Int DistSparseMatrix<Ring>::Row( Int localInd ) const
EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    return distGraph_.Source( localInd );
}

template<typename Ring>
Int DistSparseMatrix<Ring>::Col( Int localInd ) const
EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    return distGraph_.Target( localInd );
}

template<typename Ring>
Int DistSparseMatrix<Ring>::RowOffset( Int localRow ) const
EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    return distGraph_.SourceOffset( localRow );
}

template<typename Ring>
Int DistSparseMatrix<Ring>::Offset( Int localRow, Int col ) const
EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    return distGraph_.Offset( localRow, col );
}

template<typename Ring>
Int DistSparseMatrix<Ring>::NumConnections( Int localRow ) const
EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    return distGraph_.NumConnections( localRow );
}

template<typename Ring>
double DistSparseMatrix<Ring>::Imbalance() const
EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    return distGraph_.Imbalance();
}

template<typename Ring>
Ring DistSparseMatrix<Ring>::Value( Int localInd ) const
EL_NO_RELEASE_EXCEPT
{
    EL_DEBUG_CSE
    EL_DEBUG_ONLY(
      if( localInd < 0 || localInd >= (Int)vals_.size() )
          LogicError("Entry number out of bounds");
      AssertLocallyConsistent();
    )
    return vals_[localInd];
}

template<typename Ring>
Ring DistSparseMatrix<Ring>::GetLocal( Int localRow, Int col )
const EL_NO_RELEASE_EXCEPT
{
    if( localRow == END ) localRow = LocalHeight() - 1;
    if( col == END ) col = distGraph_.numTargets_ - 1;
    Int index = Offset( localRow, col );
    if( Row(index) != GlobalRow(localRow) || Col(index) != col )
        return Ring(0);
    else
        return Value( index );
}

template<typename Ring>
void DistSparseMatrix<Ring>::Set
( Int row, Int col, const Ring& val ) EL_NO_RELEASE_EXCEPT
{
    if( row == END ) row = distGraph_.numSources_ - 1;
    if( col == END ) col = distGraph_.numTargets_ - 1;
    // NOTE: This routine currently behaves passively
    const Int firstLocalRow = FirstLocalRow();
    const Int localHeight = LocalHeight();
    if( row >= firstLocalRow && row < firstLocalRow+localHeight )
    {
        const Int localRow = row - firstLocalRow;
        Int index = Offset( localRow, col );
        if( Row(index) == row && Col(index) == col )
        {
            vals_[index] = val;
        }
        else
        {
            QueueLocalUpdate( localRow, col, val );
            ProcessLocalQueues();
        }
        ProcessQueues();
    }
}

template<typename Ring>
Int* DistSparseMatrix<Ring>::SourceBuffer() EL_NO_EXCEPT
{ return distGraph_.SourceBuffer(); }
template<typename Ring>
Int* DistSparseMatrix<Ring>::TargetBuffer() EL_NO_EXCEPT
{ return distGraph_.TargetBuffer(); }
template<typename Ring>
Int* DistSparseMatrix<Ring>::OffsetBuffer() EL_NO_EXCEPT
{ return distGraph_.OffsetBuffer(); }
template<typename Ring>
Ring* DistSparseMatrix<Ring>::ValueBuffer() EL_NO_EXCEPT
{ return vals_.data(); }

template<typename Ring>
const Int* DistSparseMatrix<Ring>::LockedSourceBuffer() const EL_NO_EXCEPT
{ return distGraph_.LockedSourceBuffer(); }

template<typename Ring>
const Int* DistSparseMatrix<Ring>::LockedTargetBuffer() const EL_NO_EXCEPT
{ return distGraph_.LockedTargetBuffer(); }

template<typename Ring>
const Int* DistSparseMatrix<Ring>::LockedOffsetBuffer() const EL_NO_EXCEPT
{ return distGraph_.LockedOffsetBuffer(); }

template<typename Ring>
const Ring* DistSparseMatrix<Ring>::LockedValueBuffer() const EL_NO_EXCEPT
{ return vals_.data(); }

template<typename Ring>
void DistSparseMatrix<Ring>::ForceNumLocalEntries( Int numLocalEntries )
{
    EL_DEBUG_CSE
    distGraph_.ForceNumLocalEdges( numLocalEntries );
    vals_.resize( numLocalEntries );
}

template<typename Ring>
void DistSparseMatrix<Ring>::ForceConsistency( bool consistent ) EL_NO_EXCEPT
{
    EL_DEBUG_CSE
    distGraph_.ForceConsistency(consistent);
}

// Auxiliary routines
// ==================
template<typename Ring>
void DistSparseMatrix<Ring>::AssertConsistent() const
{
    EL_DEBUG_CSE
    Int locallyConsistent = LocallyConsistent() ? 1 : 0;
    Int consistent =
      mpi::AllReduce( locallyConsistent, mpi::BINARY_OR, Grid().Comm() );
    if( !consistent )
        LogicError("Distributed sparse matrix must be consistent");
}

template<typename Ring>
void DistSparseMatrix<Ring>::AssertLocallyConsistent() const
{
    EL_DEBUG_CSE
    if( !LocallyConsistent() )
        LogicError("Distributed sparse matrix must be consistent");
}
template<typename Ring>
DistGraphMultMeta DistSparseMatrix<Ring>::InitializeMultMeta() const
{
    EL_DEBUG_CSE
    return distGraph_.InitializeMultMeta();
}

template<typename Ring>
void DistSparseMatrix<Ring>::MappedSources
( const DistMap& reordering, vector<Int>& mappedSources ) const
{
    EL_DEBUG_CSE
    const int commRank = Grid().Rank();
    Timer timer;
    const bool time = false;
    const Int localHeight = LocalHeight();
    if( Int(mappedSources.size()) == localHeight )
        return;

    // Get the reordered indices of our local rows of the sparse matrix
    if( time && commRank == 0 )
        timer.Start();
    mappedSources.resize( localHeight );
    for( Int iLoc=0; iLoc<localHeight; ++iLoc )
        mappedSources[iLoc] = GlobalRow(iLoc);
    reordering.Translate( mappedSources );
    if( time && commRank == 0 )
        Output("Source translation: ",timer.Stop()," secs");
}

template<typename Ring>
void DistSparseMatrix<Ring>::MappedTargets
( const DistMap& reordering,
  vector<Int>& mappedTargets,
  vector<Int>& colOffs ) const
{
    EL_DEBUG_CSE
    if( mappedTargets.size() != 0 && colOffs.size() != 0 )
        return;

    const int commRank = Grid().Rank();
    Timer timer;
    const bool time = false;

    // Compute the unique set of column indices that our process interacts with
    if( time && commRank == 0 )
        timer.Start();
    const Int* colBuffer = LockedTargetBuffer();
    const Int numLocalEntries = NumLocalEntries();
    colOffs.resize( numLocalEntries );
    vector<ValueInt<Int>> uniqueCols(numLocalEntries);
    for( Int e=0; e<numLocalEntries; ++e )
        uniqueCols[e] = ValueInt<Int>{colBuffer[e],e};
    std::sort( uniqueCols.begin(), uniqueCols.end(), ValueInt<Int>::Lesser );
    {
        Int uniqueOff=-1, lastUnique=-1;
        for( Int e=0; e<numLocalEntries; ++e )
        {
            if( lastUnique != uniqueCols[e].value )
            {
                ++uniqueOff;
                lastUnique = uniqueCols[e].value;
                uniqueCols[uniqueOff] = uniqueCols[e];
            }
            colOffs[uniqueCols[e].index] = uniqueOff;
        }
        uniqueCols.resize( uniqueOff+1 );
    }
    const Int numUniqueCols = uniqueCols.size();
    if( time && commRank == 0 )
        Output("Unique sort: ",timer.Stop()," secs");

    // Get the reordered indices of the targets of our portion of the
    // distributed sparse matrix
    if( time && commRank == 0 )
        timer.Start();
    mappedTargets.resize( numUniqueCols );
    for( Int e=0; e<numUniqueCols; ++e )
        mappedTargets[e] = uniqueCols[e].value;
    reordering.Translate( mappedTargets );
    if( time && commRank == 0 )
        Output("Target translation: ",timer.Stop()," secs");
}

template<typename Ring>
bool DistSparseMatrix<Ring>::CompareEntries
( const Entry<Ring>& a, const Entry<Ring>& b )
{ return a.i < b.i || (a.i == b.i && a.j < b.j); }

#ifdef EL_INSTANTIATE_CORE
# define EL_EXTERN
#else
# define EL_EXTERN extern
#endif

#define PROTO(Ring) EL_EXTERN template class DistSparseMatrix<Ring>;
#define EL_ENABLE_DOUBLEDOUBLE
#define EL_ENABLE_QUADDOUBLE
#define EL_ENABLE_QUAD
#define EL_ENABLE_BIGINT
#define EL_ENABLE_BIGFLOAT
#include <El/macros/Instantiate.h>

#undef EL_EXTERN

} // namespace El
#endif //EL_CORE_DISTSPARSEMATRIX_IMPL_HPP
