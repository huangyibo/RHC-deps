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
#ifndef EL_CORE_SPARSEMATRIX_DECL_HPP
#define EL_CORE_SPARSEMATRIX_DECL_HPP

namespace El {

// Forward declaration for constructor
template<typename Ring> class DistSparseMatrix;

template<typename Ring>
class SparseMatrix
{
public:
    // Constructors and destructors
    // ============================
    SparseMatrix();
    SparseMatrix( Int height, Int width );
    SparseMatrix( const SparseMatrix<Ring>& A );
    // NOTE: This requires A to be distributed over a single process
    SparseMatrix( const DistSparseMatrix<Ring>& A );
    // TODO(poulson): Move constructor
    ~SparseMatrix();

    // Assignment and reconfiguration
    // ==============================

    // Change the size of the matrix
    // -----------------------------
    void Empty( bool clearMemory=true );
    void Resize( Int height, Int width );

    // Assembly
    // --------
    void Reserve( Int numEntries );

    void FreezeSparsity() EL_NO_EXCEPT;
    void UnfreezeSparsity() EL_NO_EXCEPT;
    bool FrozenSparsity() const EL_NO_EXCEPT;

    // Expensive independent updates and explicit zeroing
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    void Update( const Entry<Ring>& entry );
    void Update( Int row, Int col, const Ring& value );
    void Zero( Int row, Int col );

    // Batch updating and zeroing (recommended)
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    void QueueUpdate( const Entry<Ring>& entry ) EL_NO_RELEASE_EXCEPT;
    void QueueUpdate
    ( Int row, Int col, const Ring& value ) EL_NO_RELEASE_EXCEPT;
    void QueueZero( Int row, Int col ) EL_NO_RELEASE_EXCEPT;
    void ProcessQueues();

    // Operator overloading
    // ====================

    // Make a copy
    // -----------
    // For copying one matrix to another
    const SparseMatrix<Ring>& operator=( const SparseMatrix<Ring>& A );
    // NOTE: This requires A to be distributed over a single process
    const SparseMatrix<Ring>& operator=( const DistSparseMatrix<Ring>& A );
    // TODO(poulson): Move assignment

    // Make a copy of a submatrix
    // --------------------------
    SparseMatrix<Ring> operator()
    ( Range<Int> I, Range<Int> J ) const;
    SparseMatrix<Ring> operator()
    ( Range<Int> I, const vector<Int>& J ) const;
    SparseMatrix<Ring> operator()
    ( const vector<Int>& I, Range<Int> J ) const;
    SparseMatrix<Ring> operator()
    ( const vector<Int>& I, const vector<Int>& J ) const;

    // Rescaling
    // ---------
    const SparseMatrix<Ring>& operator*=( const Ring& alpha );

    // Addition/subtraction
    // --------------------
    const SparseMatrix<Ring>& operator+=( const SparseMatrix<Ring>& A );
    const SparseMatrix<Ring>& operator-=( const SparseMatrix<Ring>& A );

    // For manually modifying data
    void ForceNumEntries( Int numEntries );
    void ForceConsistency( bool consistent=true ) EL_NO_EXCEPT;
    Int* SourceBuffer() EL_NO_EXCEPT;
    Int* TargetBuffer() EL_NO_EXCEPT;
    Int* OffsetBuffer() EL_NO_EXCEPT;
    Ring* ValueBuffer() EL_NO_EXCEPT;
    const Int* LockedSourceBuffer() const EL_NO_EXCEPT;
    const Int* LockedTargetBuffer() const EL_NO_EXCEPT;
    const Int* LockedOffsetBuffer() const EL_NO_EXCEPT;
    const Ring* LockedValueBuffer() const EL_NO_EXCEPT;

    // Queries
    // =======

    // High-level information
    // ----------------------
    Int Height() const EL_NO_EXCEPT;
    Int Width() const EL_NO_EXCEPT;
    Int NumEntries() const EL_NO_EXCEPT;
    Int Capacity() const EL_NO_EXCEPT;
    bool Consistent() const EL_NO_EXCEPT;
    El::Graph& Graph() EL_NO_EXCEPT;
    const El::Graph& LockedGraph() const EL_NO_EXCEPT;

    // Entrywise information
    // ---------------------
    Int Row( Int index ) const EL_NO_RELEASE_EXCEPT;
    Int Col( Int index ) const EL_NO_RELEASE_EXCEPT;
    Ring Value( Int index ) const EL_NO_RELEASE_EXCEPT;
    Ring Get( Int row, Int col ) const EL_NO_RELEASE_EXCEPT;
    void Set( Int row, Int col, const Ring& val ) EL_NO_RELEASE_EXCEPT;
    Int RowOffset( Int row ) const EL_NO_RELEASE_EXCEPT;
    Int Offset( Int row, Int col ) const EL_NO_RELEASE_EXCEPT;
    Int NumConnections( Int row ) const EL_NO_RELEASE_EXCEPT;

    void AssertConsistent() const;

private:
    El::Graph graph_;
    vector<Ring> vals_;

    struct CompareEntriesFunctor
    {
        bool operator()(const Entry<Ring>& a, const Entry<Ring>& b )
        { return a.i < b.i || (a.i == b.i && a.j < b.j); }
    };

    template<typename U> friend class DistSparseMatrix;
    template<typename U>
    friend void CopyFromRoot
    ( const DistSparseMatrix<U>& ADist, SparseMatrix<U>& A );
};

} // namespace El

#endif // ifndef EL_CORE_SPARSEMATRIX_DECL_HPP
