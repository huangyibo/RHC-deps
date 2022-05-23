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
#ifndef EL_FACTOR_LDL_SPARSE_SYMBOLIC_HPP
#define EL_FACTOR_LDL_SPARSE_SYMBOLIC_HPP

#include <ElSuiteSparse/amd.h>

#include <El/lapack_like/factor/ldl/sparse/symbolic/Separator.hpp>
#include <El/lapack_like/factor/ldl/sparse/symbolic/NodeInfo.hpp>

namespace El {
namespace ldl {

Int Analysis( NodeInfo& rootInfo, Int myOff=0 );
void Analysis( DistNodeInfo& rootInfo, bool storeFactRecvInds=true );

void AMDOrder
( const vector<Int>& subOffsets,
  const vector<Int>& subTargets,
        vector<Int>& amdPerm,
        double* control=nullptr,
        double* info=nullptr );

void NestedDissection
( const Graph& graph,
        vector<Int>& map,
        Separator& rootSep,
        NodeInfo& rootInfo,
  const BisectCtrl& ctrl=BisectCtrl() );
void NestedDissection
( const DistGraph& graph,
        DistMap& map,
        DistSeparator& rootSep,
        DistNodeInfo& rootInfo,
  const BisectCtrl& ctrl=BisectCtrl() );

void NaturalNestedDissection
( Int nx, Int ny, Int nz,
  const Graph& graph,
        vector<Int>& map,
        Separator& rootSep,
        NodeInfo& info,
        Int cutoff );
void NaturalNestedDissection
( Int nx, Int ny, Int nz,
  const DistGraph& graph,
        DistMap& map,
        DistSeparator& rootSep,
        DistNodeInfo& info,
        Int cutoff,
        bool storeFactRecvInds=false );

} // namespace ldl
} // namespace El

#endif // ifndef EL_FACTOR_LDL_SPARSE_SYMBOLIC_HPP
