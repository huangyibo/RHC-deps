/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_BLAS_QUASIDIAGONALSOLVE_HPP
#define EL_BLAS_QUASIDIAGONALSOLVE_HPP

namespace El {

// TODO(poulson): Avoid duplication with QuasiDiagonalScale if possible. Only
// difference is the inversions
template<typename Field,typename FieldMain>
void
QuasiDiagonalSolve
( LeftOrRight side, UpperOrLower uplo,
  const Matrix<FieldMain>& d,
  const Matrix<Field>& dSub,
        Matrix<Field>& X,
  bool conjugated )
{
    EL_DEBUG_CSE
    const Int m = X.Height();
    const Int n = X.Width();

    Matrix<Field> D( 2, 2 );
    if( side == LEFT && uplo == LOWER )
    {
        if( m == 0 )
            return;
        EL_DEBUG_ONLY(
          if( d.Height() != m )
              LogicError
              ("d was the wrong size: m=",m,",n=",n,", d ~ ",
               d.Height()," x ",d.Width());
          if( dSub.Height() != m-1 )
              LogicError
              ("dSub was the wrong size: m=",m,",n=",n,", dSub ~ ",
               dSub.Height()," x ",dSub.Width());
        )
        Int i=0;
        while( i < m )
        {
            Int nb;
            if( i < m-1 && Abs(dSub(i)) > 0 )
                nb = 2;
            else
                nb = 1;

            auto XRow = X( IR(i,i+nb), IR(0,n) );
            if( nb == 1 )
            {
                Scale( Field(1)/d(i), XRow );
            }
            else
            {
                D(0,0) = d(i);
                D(1,1) = d(i+1);
                D(1,0) = dSub(i);
                Symmetric2x2Inv( LOWER, D, conjugated );
                MakeSymmetric( LOWER, D, conjugated );

                Transform2x2Rows( D, X, i, i+1 );
            }

            i += nb;
        }
    }
    else if( side == RIGHT && uplo == LOWER )
    {
        if( n == 0 )
            return;
        EL_DEBUG_ONLY(
          if( d.Height() != n )
              LogicError
              ("d was the wrong size: m=",m,",n=",n,", d ~ ",
               d.Height()," x ",d.Width());
          if( dSub.Height() != n-1 )
              LogicError
              ("dSub was the wrong size: m=",m,",n=",n,", dSub ~ ",
               dSub.Height()," x ",dSub.Width());
        )

        Int j=0;
        while( j < n )
        {
            Int nb;
            if( j < n-1 && Abs(dSub(j)) > 0 )
                nb = 2;
            else
                nb = 1;

            auto XCol = X( IR(0,m), IR(j,j+nb) );
            if( nb == 1 )
            {
                Scale( Field(1)/d(j), XCol );
            }
            else
            {
                D(0,0) = d(j);
                D(1,1) = d(j+1);
                D(1,0) = dSub(j);
                Symmetric2x2Inv( LOWER, D, conjugated );
                MakeSymmetric( LOWER, D, conjugated );

                Transform2x2Cols( D, X, j, j+1 );
            }

            j += nb;
        }
    }
    else
        LogicError("This option not yet supported");
}

template<typename Field,typename FieldMain,Dist U,Dist V>
void
LeftQuasiDiagonalSolve
( UpperOrLower uplo,
  const DistMatrix<FieldMain,U,STAR>& d,
  const DistMatrix<FieldMain,U,STAR>& dPrev,
  const DistMatrix<FieldMain,U,STAR>& dNext,
  const DistMatrix<Field,    U,STAR>& dSub,
  const DistMatrix<Field,    U,STAR>& dSubPrev,
  const DistMatrix<Field,    U,STAR>& dSubNext,
        DistMatrix<Field,U,V>& X,
  const DistMatrix<Field,U,V>& XPrev,
  const DistMatrix<Field,U,V>& XNext,
  bool conjugated )
{
    EL_DEBUG_CSE
    if( uplo == UPPER )
        LogicError("This option not yet supported");
    const Int m = X.Height();
    const Int mLocal = X.LocalHeight();
    const Int colStride = X.ColStride();
    EL_DEBUG_ONLY(
      const Int colAlignPrev = Mod(X.ColAlign()+1,colStride);
      const Int colAlignNext = Mod(X.ColAlign()-1,colStride);
      if( d.ColAlign() != X.ColAlign() || dSub.ColAlign() != X.ColAlign() )
          LogicError("data is not properly aligned");
      if( XPrev.ColAlign() != colAlignPrev ||
          dPrev.ColAlign() != colAlignPrev ||
          dSubPrev.ColAlign() != colAlignPrev )
          LogicError("'previous' data is not properly aligned");
      if( XNext.ColAlign() != colAlignNext ||
          dNext.ColAlign() != colAlignNext ||
          dSubNext.ColAlign() != colAlignNext )
          LogicError("'next' data is not properly aligned");
    )
    const Int prevOff = ( XPrev.ColShift()==X.ColShift()-1 ? 0 : -1 );
    const Int nextOff = ( XNext.ColShift()==X.ColShift()+1 ? 0 : +1 );
    if( !X.Participating() )
        return;

    // It is best to separate the case where colStride is 1
    if( colStride == 1 )
    {
        QuasiDiagonalSolve
        ( LEFT, uplo, d.LockedMatrix(), dSub.LockedMatrix(), X.Matrix(),
          conjugated );
        return;
    }

    Matrix<Field> D11( 2, 2 );
    for( Int iLoc=0; iLoc<mLocal; ++iLoc )
    {
        const Int i = X.GlobalRow(iLoc);
        const Int iLocPrev = iLoc + prevOff;
        const Int iLocNext = iLoc + nextOff;

        auto x1Loc = X.Matrix()( IR(iLoc), ALL );

        if( i<m-1 && dSub.GetLocal(iLoc,0) != Field(0) )
        {
            // Handle 2x2 starting at i
            D11.Set( 0, 0, d.GetLocal(iLoc,0) );
            D11.Set( 1, 1, dNext.GetLocal(iLocNext,0) );
            D11.Set( 1, 0, dSub.GetLocal(iLoc,0) );
            Symmetric2x2Inv( LOWER, D11, conjugated );
            MakeSymmetric( LOWER, D11, conjugated );

            auto x1NextLoc = XNext.LockedMatrix()( IR(iLocNext), ALL );
            Scale( D11.Get(0,0), x1Loc );
            Axpy( D11.Get(0,1), x1NextLoc, x1Loc );
        }
        else if( i>0 && dSubPrev.GetLocal(iLocPrev,0) != Field(0) )
        {
            // Handle 2x2 starting at i-1
            D11.Set( 0, 0, dPrev.GetLocal(iLocPrev,0) );
            D11.Set( 1, 1, d.GetLocal(iLoc,0) );
            D11.Set( 1, 0, dSubPrev.GetLocal(iLocPrev,0) );
            Symmetric2x2Inv( LOWER, D11, conjugated );
            MakeSymmetric( LOWER, D11, conjugated );

            auto x1PrevLoc = XPrev.LockedMatrix()( IR(iLocPrev), ALL );
            Scale( D11.Get(1,1), x1Loc );
            Axpy( D11.Get(1,0), x1PrevLoc, x1Loc );
        }
        else
        {
            // Handle 1x1
            Scale( Field(1)/d.GetLocal(iLoc,0), x1Loc );
        }
    }
}

template<typename Field,typename FieldMain,Dist U,Dist V>
void
RightQuasiDiagonalSolve
( UpperOrLower uplo,
  const DistMatrix<FieldMain,V,STAR>& d,
  const DistMatrix<FieldMain,V,STAR>& dPrev,
  const DistMatrix<FieldMain,V,STAR>& dNext,
  const DistMatrix<Field,    V,STAR>& dSub,
  const DistMatrix<Field,    V,STAR>& dSubPrev,
  const DistMatrix<Field,    V,STAR>& dSubNext,
        DistMatrix<Field,U,V>& X,
  const DistMatrix<Field,U,V>& XPrev,
  const DistMatrix<Field,U,V>& XNext,
  bool conjugated )
{
    EL_DEBUG_CSE
    if( uplo == UPPER )
        LogicError("This option not yet supported");
    const Int n = X.Width();
    const Int nLocal = X.LocalWidth();
    const Int rowStride = X.RowStride();
    EL_DEBUG_ONLY(
      const Int rowAlignPrev = Mod(X.RowAlign()+1,rowStride);
      const Int rowAlignNext = Mod(X.RowAlign()-1,rowStride);
      if( d.ColAlign() != X.RowAlign() || dSub.RowAlign() != X.RowAlign() )
          LogicError("data is not properly aligned");
      if( XPrev.RowAlign() != rowAlignPrev ||
          dPrev.ColAlign() != rowAlignPrev ||
          dSubPrev.ColAlign() != rowAlignPrev )
          LogicError("'previous' data is not properly aligned");
      if( XNext.RowAlign() != rowAlignNext ||
          dNext.ColAlign() != rowAlignNext ||
          dSubNext.ColAlign() != rowAlignNext )
          LogicError("'next' data is not properly aligned");
    )
    const Int prevOff = ( XPrev.RowShift()==X.RowShift()-1 ? 0 : -1 );
    const Int nextOff = ( XNext.RowShift()==X.RowShift()+1 ? 0 : +1 );
    if( !X.Participating() )
        return;

    // It is best to separate the case where rowStride is 1
    if( rowStride == 1 )
    {
        QuasiDiagonalSolve
        ( RIGHT, uplo, d.LockedMatrix(), dSub.LockedMatrix(), X.Matrix(),
          conjugated );
        return;
    }

    Matrix<Field> D11( 2, 2 );
    for( Int jLoc=0; jLoc<nLocal; ++jLoc )
    {
        const Int j = X.GlobalCol(jLoc);
        const Int jLocPrev = jLoc + prevOff;
        const Int jLocNext = jLoc + nextOff;

        auto x1Loc = X.Matrix()( ALL, IR(jLoc) );

        if( j<n-1 && dSub.GetLocal(jLoc,0) != Field(0) )
        {
            // Handle 2x2 starting at j
            D11.Set( 0, 0, d.GetLocal(jLoc,0) );
            D11.Set( 1, 1, dNext.GetLocal(jLocNext,0) );
            D11.Set( 1, 0, dSub.GetLocal(jLoc,0) );
            Symmetric2x2Inv( LOWER, D11, conjugated );
            MakeSymmetric( LOWER, D11, conjugated );

            auto x1NextLoc = XNext.LockedMatrix()( ALL, IR(jLocNext) );
            Scale( D11.Get(0,0), x1Loc );
            Axpy( D11.Get(1,0), x1NextLoc, x1Loc );
        }
        else if( j>0 && dSubPrev.GetLocal(jLocPrev,0) != Field(0) )
        {
            // Handle 2x2 starting at j-1
            D11.Set( 0, 0, dPrev.GetLocal(jLocPrev,0) );
            D11.Set( 1, 1, d.GetLocal(jLoc,0) );
            D11.Set( 1, 0, dSubPrev.GetLocal(jLocPrev,0) );
            Symmetric2x2Inv( LOWER, D11, conjugated );
            MakeSymmetric( LOWER, D11, conjugated );

            auto x1PrevLoc = XPrev.LockedMatrix()( ALL, IR(jLocPrev) );
            Scale( D11.Get(1,1), x1Loc );
            Axpy( D11.Get(0,1), x1PrevLoc, x1Loc );
        }
        else
        {
            // Handle 1x1
            Scale( Field(1)/d.GetLocal(jLoc,0), x1Loc );
        }
    }
}

template<typename Field,typename FieldMain,Dist U,Dist V>
void
QuasiDiagonalSolve
( LeftOrRight side, UpperOrLower uplo,
  const AbstractDistMatrix<FieldMain>& d,
  const AbstractDistMatrix<Field>& dSub,
        DistMatrix<Field,U,V>& X,
  bool conjugated )
{
    EL_DEBUG_CSE
    const Grid& g = X.Grid();
    const Int colAlign = X.ColAlign();
    const Int rowAlign = X.RowAlign();
    if( side == LEFT )
    {
        const Int colStride = X.ColStride();
        DistMatrix<FieldMain,U,STAR> d_U_STAR(g);
        DistMatrix<Field,U,STAR> dSub_U_STAR(g);
        d_U_STAR.AlignWith( X );
        dSub_U_STAR.AlignWith( X );
        d_U_STAR = d;
        dSub_U_STAR = dSub;
        if( colStride == 1 )
        {
            QuasiDiagonalSolve
            ( side, uplo, d_U_STAR.LockedMatrix(), dSub_U_STAR.LockedMatrix(),
              X.Matrix(), conjugated );
            return;
        }

        DistMatrix<FieldMain,U,STAR> dPrev_U_STAR(g), dNext_U_STAR(g);
        DistMatrix<Field,U,STAR> dSubPrev_U_STAR(g), dSubNext_U_STAR(g);
        DistMatrix<Field,U,V> XPrev(g), XNext(g);
        const Int colAlignPrev = Mod(colAlign+1,colStride);
        const Int colAlignNext = Mod(colAlign-1,colStride);
        dPrev_U_STAR.AlignCols( colAlignPrev );
        dNext_U_STAR.AlignCols( colAlignNext );
        dSubPrev_U_STAR.AlignCols( colAlignPrev );
        dSubNext_U_STAR.AlignCols( colAlignNext );
        XPrev.Align( colAlignPrev, rowAlign );
        XNext.Align( colAlignNext, rowAlign );
        dPrev_U_STAR = d;
        dNext_U_STAR = d;
        dSubPrev_U_STAR = dSub;
        dSubNext_U_STAR = dSub;
        XPrev = X;
        XNext = X;
        LeftQuasiDiagonalSolve
        ( uplo, d_U_STAR, dPrev_U_STAR, dNext_U_STAR,
          dSub_U_STAR, dSubPrev_U_STAR, dSubNext_U_STAR,
          X, XPrev, XNext, conjugated );
    }
    else
    {
        const Int rowStride = X.RowStride();
        DistMatrix<FieldMain,V,STAR> d_V_STAR(g);
        DistMatrix<Field,V,STAR> dSub_V_STAR(g);
        d_V_STAR.AlignWith( X );
        dSub_V_STAR.AlignWith( X );
        d_V_STAR = d;
        dSub_V_STAR = dSub;
        if( rowStride == 1 )
        {
            QuasiDiagonalSolve
            ( side, uplo,
              d_V_STAR.LockedMatrix(), dSub_V_STAR.LockedMatrix(),
              X.Matrix(), conjugated );
            return;
        }

        DistMatrix<FieldMain,V,STAR> dPrev_V_STAR(g), dNext_V_STAR(g);
        DistMatrix<Field,V,STAR> dSubPrev_V_STAR(g), dSubNext_V_STAR(g);
        DistMatrix<Field,U,V> XPrev(g), XNext(g);
        const Int rowAlignPrev = Mod(rowAlign+1,rowStride);
        const Int rowAlignNext = Mod(rowAlign-1,rowStride);
        dPrev_V_STAR.AlignCols( rowAlignPrev );
        dNext_V_STAR.AlignCols( rowAlignNext );
        dSubPrev_V_STAR.AlignCols( rowAlignPrev );
        dSubNext_V_STAR.AlignCols( rowAlignNext );
        XPrev.Align( colAlign, rowAlignPrev );
        XNext.Align( colAlign, rowAlignNext );
        dPrev_V_STAR = d;
        dNext_V_STAR = d;
        dSubPrev_V_STAR = dSub;
        dSubNext_V_STAR = dSub;
        XPrev = X;
        XNext = X;
        RightQuasiDiagonalSolve
        ( uplo, d_V_STAR, dPrev_V_STAR, dNext_V_STAR,
          dSub_V_STAR, dSubPrev_V_STAR, dSubNext_V_STAR,
          X, XPrev, XNext, conjugated );
    }
}

template<typename Field,typename FieldMain,Dist U,Dist V>
void
QuasiDiagonalSolve
( LeftOrRight side, UpperOrLower uplo,
  const AbstractDistMatrix<FieldMain>& d,
  const AbstractDistMatrix<Field>& dSub,
        DistMatrix<Field,U,V,BLOCK>& X,
  bool conjugated )
{
    EL_DEBUG_CSE
    LogicError("This routine is not yet supported");
}

} // namespace El

#endif // ifndef EL_BLAS_QUASIDIAGONALSOLVE_HPP
