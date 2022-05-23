/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_OPTIMIZATION_MODELS_HPP
#define EL_OPTIMIZATION_MODELS_HPP

namespace El {

namespace RegularizationNS {
enum Regularization {
  NO_PENALTY,
  L1_PENALTY,
  L2_PENALTY
};
}
using namespace RegularizationNS;

// Basis pursuit: min || x ||_1 such that A x = b
// ==============================================

namespace bp {

template<typename Real>
struct ADMMCtrl {
  Real rho=Real(1);
  Real alpha=Real(1.2);
  Int maxIter=500;
  Real absTol=Real(1e-6);
  Real relTol=Real(1e-4);
  bool usePinv=false;
  Real pinvTol=0;
  bool progress=true;
};

} // namespace bp

template<typename Real>
struct BPCtrl {
  bool useIPM=true;
  bool useSOCP=false;
  // NOTE: The ADMM implementation is still a prototype
  bp::ADMMCtrl<Real> admmCtrl;
  lp::direct::Ctrl<Real> lpIPMCtrl;
  socp::direct::Ctrl<Real> socpIPMCtrl;

  BPCtrl( bool sparse ) : lpIPMCtrl(sparse) { }
};

template<typename Real>
struct BPCtrl<Complex<Real>> {
  socp::direct::Ctrl<Real> ipmCtrl;
  BPCtrl( bool sparse=true ) { }
};

template<typename Field>
void BP
( const Matrix<Field>& A,
  const Matrix<Field>& b,
        Matrix<Field>& x,
  const BPCtrl<Field>& ctrl=BPCtrl<Field>(false) );
template<typename Field>
void BP
( const AbstractDistMatrix<Field>& A,
  const AbstractDistMatrix<Field>& b,
        AbstractDistMatrix<Field>& x,
  const BPCtrl<Field>& ctrl=BPCtrl<Field>(false) );
template<typename Field>
void BP
( const SparseMatrix<Field>& A,
  const Matrix<Field>& b,
        Matrix<Field>& x,
  const BPCtrl<Field>& ctrl=BPCtrl<Field>(true) );
template<typename Field>
void BP
( const DistSparseMatrix<Field>& A,
  const DistMultiVec<Field>& b,
        DistMultiVec<Field>& x,
  const BPCtrl<Field>& ctrl=BPCtrl<Field>(true) );

// Chebyshev point: min || A x - b||_oo
// ====================================
template<typename Real>
void CP
( const Matrix<Real>& A,
  const Matrix<Real>& b,
        Matrix<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );
template<typename Real>
void CP
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b,
        AbstractDistMatrix<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );
template<typename Real>
void CP
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b,
        Matrix<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );
template<typename Real>
void CP
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b,
        DistMultiVec<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );

// Least Absolute Value: min || A x - b||_1
// ========================================
// TODO(poulson): Generalize to complex now that there is SOCP support
template<typename Real>
void LAV
( const Matrix<Real>& A,
  const Matrix<Real>& b,
        Matrix<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );
template<typename Real>
void LAV
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b,
        AbstractDistMatrix<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );
template<typename Real>
void LAV
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b,
        Matrix<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );
template<typename Real>
void LAV
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b,
        DistMultiVec<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );

// Dantzig selector: min || x ||_1 s.t. || A^T (b - A x) ||_oo <= lambda
// =====================================================================
// TODO(poulson):
//  Add the ability to switch between the (DS1) and (DS2) affine LP
//  formulations described in
//
//  Michael Friedlander and Michael Saunders,
//  "Discussion: The Dantzig Selector: Statistical estimation when p is much
//   larger than n",
//  The Annals of Statistics, Vol. 35, No. 6, pp. 2385--2391, 2007.
//
// Elemental currently defaults to (DS1) for dense matrices and (DS2) for
// sparse matrices.
//
// TODO(poulson): Generalize to complex now that there is SOCP support.

template<typename Real>
void DS
( const Matrix<Real>& A,
  const Matrix<Real>& b,
        Real lambda,
        Matrix<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );
template<typename Real>
void DS
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b,
        Real lambda,
        AbstractDistMatrix<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );
template<typename Real>
void DS
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b,
        Real lambda,
        Matrix<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );
template<typename Real>
void DS
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b,
        Real lambda,
        DistMultiVec<Real>& x,
  const lp::affine::Ctrl<Real>& ctrl=lp::affine::Ctrl<Real>() );

// Robust least squares
// ====================
// Given || [dA, db] ||_2 <= rho, minimize the worst-case error of
//
//     || (A+dA) x - (b+db) ||_2,
//
// which can be shown to be equal to
//
//     || A x - b ||_2 + rho || [x; 1] ||_2,
//
// which can be formulated as the SOCP
//
//     min t + rho s
//     s.t. || A x - b ||_2 <= t, || [x; 1] ||_2 <= s.
//
// (See [1] or Subsection 2.7 of [2].)
//
// While a Singular Value Decomposition can also be used to solve RLS [1],
// the current implementation is based upon Second-Order Cone Programming.
//
// [1] L. El Ghaoui and H. Lebret, "Robust solutions to least-squares problems
//     with uncertain data", SIAM J. Matrix Anal. and Appl., Vol. 18, No. 4,
//     1997. DOI: http://epubs.siam.org/doi/abs/10.1137/S0895479896298130
//
// [2] M.S. Lobo, L. Vandenberghe, S. Boyd, and H. Lebret,
//     "Applications of second-order cone programming",
//     Linear Algebra and its Applications, Vol. 284, Issues 1-3, 1998.
//     DOI: http://www.sciencedirect.com/science/article/pii/S0024379598100320
//
// TODO(poulson): Complex implementations which simply unpack the real and
// imaginary components of x.
//
template<typename Real>
void RLS
( const Matrix<Real>& A,
  const Matrix<Real>& b,
        Real rho,
        Matrix<Real>& x,
  const socp::affine::Ctrl<Real>& ctrl=socp::affine::Ctrl<Real>() );
template<typename Real>
void RLS
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b,
        Real rho,
        AbstractDistMatrix<Real>& x,
  const socp::affine::Ctrl<Real>& ctrl=socp::affine::Ctrl<Real>() );
template<typename Real>
void RLS
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b,
        Real rho,
        Matrix<Real>& x,
  const socp::affine::Ctrl<Real>& ctrl=socp::affine::Ctrl<Real>() );
template<typename Real>
void RLS
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b,
        Real rho,
        DistMultiVec<Real>& x,
  const socp::affine::Ctrl<Real>& ctrl=socp::affine::Ctrl<Real>() );

// Non-negative least squares
// ==========================
// NOTE: The following can solve a *sequence* of NNLS problems

namespace NNLSApproachNS {
enum NNLSApproach {
    NNLS_ADMM, // The ADMM implementation is still a prototype
    NNLS_QP,
    NNLS_SOCP
};
} // namespace NNLSApproachNS
using namespace NNLSApproachNS;

template<typename Real>
struct NNLSCtrl {
  NNLSApproach approach=NNLS_SOCP;
  ADMMCtrl<Real> admmCtrl;
  qp::direct::Ctrl<Real> qpCtrl;
  socp::affine::Ctrl<Real> socpCtrl;
};

template<typename Real>
void NNLS
( const Matrix<Real>& A,
  const Matrix<Real>& B,
        Matrix<Real>& X,
  const NNLSCtrl<Real>& ctrl=NNLSCtrl<Real>() );
template<typename Real>
void NNLS
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& B,
        AbstractDistMatrix<Real>& X,
  const NNLSCtrl<Real>& ctrl=NNLSCtrl<Real>() );
template<typename Real>
void NNLS
( const SparseMatrix<Real>& A,
  const Matrix<Real>& B,
        Matrix<Real>& X,
  const NNLSCtrl<Real>& ctrl=NNLSCtrl<Real>() );
template<typename Real>
void NNLS
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& B,
        DistMultiVec<Real>& X,
  const NNLSCtrl<Real>& ctrl=NNLSCtrl<Real>() );

// Robust non-negative least squares
// =================================
// Given || [dA, db] ||_2 <= rho, minimize the worst-case error of
//
//     || (A+dA) x - (b+db) ||_2,
//
// which can be shown to be equal to
//
//     || A x - b ||_2 + rho || [x; 1] ||_2,
//
// subject to the constraint that x >= 0. Just as in the case of RLS, the
// problem can easily be formulated as an SOCP, which, in this case, is
//
//     min t + rho s
//     s.t. || A x - b ||_2 <= t, || [x; 1] ||_2 <= s, x >= 0.
//
// (See [1] or Subsection 2.7 of [2].)
//
// While a Singular Value Decomposition can also be used to solve RLS [1],
// the current implementation is based upon Second-Order Cone Programming.
//
// [1] L. El Ghaoui and H. Lebret, "Robust solutions to least-squares problems
//     with uncertain data", SIAM J. Matrix Anal. and Appl., Vol. 18, No. 4,
//     1997. DOI: http://epubs.siam.org/doi/abs/10.1137/S0895479896298130
//
// [2] M.S. Lobo, L. Vandenberghe, S. Boyd, and H. Lebret,
//     "Applications of second-order cone programming",
//     Linear Algebra and its Applications, Vol. 284, Issues 1-3, 1998.
//     DOI: http://www.sciencedirect.com/science/article/pii/S0024379598100320
//
template<typename Real>
void RNNLS
( const Matrix<Real>& A,
  const Matrix<Real>& b,
        Real rho,
        Matrix<Real>& x,
  const socp::affine::Ctrl<Real>& ctrl=socp::affine::Ctrl<Real>() );
template<typename Real>
void RNNLS
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b,
        Real rho,
        AbstractDistMatrix<Real>& x,
  const socp::affine::Ctrl<Real>& ctrl=socp::affine::Ctrl<Real>() );
template<typename Real>
void RNNLS
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b,
        Real rho,
        Matrix<Real>& x,
  const socp::affine::Ctrl<Real>& ctrl=socp::affine::Ctrl<Real>() );
template<typename Real>
void RNNLS
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b,
        Real rho,
        DistMultiVec<Real>& x,
  const socp::affine::Ctrl<Real>& ctrl=socp::affine::Ctrl<Real>() );

// Non-negative matrix factorization
// =================================
template<typename Real>
struct NMFCtrl {
  NNLSCtrl<Real> nnlsCtrl;
  Int maxIter=20;
};

template<typename Real>
void NMF
( const Matrix<Real>& A,
        Matrix<Real>& X,
        Matrix<Real>& Y,
  const NMFCtrl<Real>& ctrl=NMFCtrl<Real>() );
template<typename Real>
void NMF
( const AbstractDistMatrix<Real>& A,
        AbstractDistMatrix<Real>& X,
        AbstractDistMatrix<Real>& Y,
  const NMFCtrl<Real>& ctrl=NMFCtrl<Real>() );
// TODO(poulson): Sparse versions

// Basis pursuit denoising (BPDN), a.k.a.,
// Least absolute selection and shrinkage operator (Lasso):
//   min (1/2) || b - A x ||_2^2 + lambda || x ||_1
// ================================================
// TODO(poulson): Generalize to complex now that there is SOCP support

namespace bpdn {

template<typename Real>
struct ADMMCtrl {
  Real rho=Real(1);
  Real alpha=Real(1.2);
  Int maxIter=500;
  Real absTol=Real(1e-6);
  Real relTol=Real(1e-4);
  bool inv=true;
  bool progress=true;
};

} // namespace bpdn

template<typename Real>
struct BPDNCtrl {
  bool useIPM=true;
  // NOTE: The ADMM implementation is still a prototype
  bpdn::ADMMCtrl<Real> admmCtrl;
  qp::affine::Ctrl<Real> ipmCtrl;
};

template<typename Real>
void BPDN
( const Matrix<Real>& A,
  const Matrix<Real>& b,
        Real lambda,
        Matrix<Real>& x,
  const BPDNCtrl<Real>& ctrl=BPDNCtrl<Real>() );
template<typename Real>
void BPDN
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b,
        Real lambda,
        AbstractDistMatrix<Real>& x,
  const BPDNCtrl<Real>& ctrl=BPDNCtrl<Real>() );
template<typename Real>
void BPDN
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b,
        Real lambda,
        Matrix<Real>& x,
  const BPDNCtrl<Real>& ctrl=BPDNCtrl<Real>() );
template<typename Real>
void BPDN
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b,
        Real lambda,
        DistMultiVec<Real>& x,
  const BPDNCtrl<Real>& ctrl=BPDNCtrl<Real>() );

// Elastic net (EN):
//   min || b - A x ||_2^2 + lambda_1 || x ||_1 + lambda_2 || x ||_2^2
// ===================================================================
// TODO(poulson): Generalize to complex now that there is SOCP support

template<typename Real>
void EN
( const Matrix<Real>& A,
  const Matrix<Real>& b,
        Real lambda1,
        Real lambda2,
        Matrix<Real>& x,
  const qp::affine::Ctrl<Real>& ctrl=qp::affine::Ctrl<Real>() );
template<typename Real>
void EN
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b,
        Real lambda1,
        Real lambda2,
        AbstractDistMatrix<Real>& x,
  const qp::affine::Ctrl<Real>& ctrl=qp::affine::Ctrl<Real>() );
template<typename Real>
void EN
( const SparseMatrix<Real>& A,
  const Matrix<Real>& b,
        Real lambda1,
        Real lambda2,
        Matrix<Real>& x,
  const qp::affine::Ctrl<Real>& ctrl=qp::affine::Ctrl<Real>() );
template<typename Real>
void EN
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b,
        Real lambda1,
        Real lambda2,
        DistMultiVec<Real>& x,
  const qp::affine::Ctrl<Real>& ctrl=qp::affine::Ctrl<Real>() );

// Robust Principal Component Analysis (RPCA)
// ==========================================

template<typename Real>
struct RPCACtrl
{
    bool useALM=true;
    bool usePivQR=false;
    bool progress=true;

    Int numPivSteps=75;
    Int maxIts=1000;

    Real tau=Real(0);
    Real beta=Real(1);
    Real rho=Real(6);
    Real tol=Real(1e-5);
};

template<typename Field>
void RPCA
( const Matrix<Field>& M,
        Matrix<Field>& L,
        Matrix<Field>& S,
  const RPCACtrl<Base<Field>>& ctrl=RPCACtrl<Base<Field>>() );

template<typename Field>
void RPCA
( const AbstractDistMatrix<Field>& M,
        AbstractDistMatrix<Field>& L,
        AbstractDistMatrix<Field>& S,
  const RPCACtrl<Base<Field>>& ctrl=RPCACtrl<Base<Field>>() );

// Sparse inverse covariance selection
// ===================================
template<typename Real>
struct SparseInvCovCtrl
{
    Real rho=Real(1.);
    Real alpha=Real(1.2);
    Int maxIter=500;
    Real absTol=Real(1e-6);
    Real relTol=Real(1e-4);
    bool progress=true;
};

template<typename Field>
Int SparseInvCov
( const Matrix<Field>& D,
        Base<Field> lambda,
        Matrix<Field>& Z,
  const SparseInvCovCtrl<Base<Field>>& ctrl=SparseInvCovCtrl<Base<Field>>() );
template<typename Field>
Int SparseInvCov
( const AbstractDistMatrix<Field>& D,
        Base<Field> lambda,
        AbstractDistMatrix<Field>& Z,
  const SparseInvCovCtrl<Base<Field>>& ctrl=SparseInvCovCtrl<Base<Field>>() );

// Support Vector Machine (soft-margin)
// ====================================
// TODO(poulson): Use the formulation described in
//       <http://abel.ee.ucla.edu/cvxopt/applications/svm/>
//
// min_{w,beta,z} (1/2) || w ||_2^2 + lambda 1^T z
//
// s.t. | -diag(d) A, -d, -I | | w    | <= | -1 |
//      |       0,     0, -I | | beta |    |  0 |
//                             | z    |
//
// The output, x, is set to the concatenation of w and beta, x := [w; beta].
//

template<typename Real>
struct SVMCtrl
{
    qp::affine::Ctrl<Real> ipmCtrl;
};

// TODO(poulson): Switch to explicitly returning w, beta, and z, as it is
// difficult for users to unpack a DistMultiVec
template<typename Real>
void SVM
( const Matrix<Real>& A,
  const Matrix<Real>& d,
        Real lambda,
        Matrix<Real>& x,
  const SVMCtrl<Real>& ctrl=SVMCtrl<Real>() );
template<typename Real>
void SVM
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& d,
        Real lambda,
        AbstractDistMatrix<Real>& x,
  const SVMCtrl<Real>& ctrl=SVMCtrl<Real>() );
template<typename Real>
void SVM
( const SparseMatrix<Real>& A,
  const Matrix<Real>& d,
        Real lambda,
        Matrix<Real>& x,
  const SVMCtrl<Real>& ctrl=SVMCtrl<Real>() );
template<typename Real>
void SVM
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& d,
        Real lambda,
        DistMultiVec<Real>& x,
  const SVMCtrl<Real>& ctrl=SVMCtrl<Real>() );

// 1D total variation denoising (TV):
//
//   min (1/2) || b - x ||_2^2 + lambda || D x ||_1,
//
// where D is the 1D finite-difference operator.
// =================================================
// We follow the formulation used within CVXOPT:
//
//   min (1/2) || b - x ||_2^2 + lambda 1^T y
//   s.t. -y <= D x <= y,
//
// where x is in R^n and y is in R^(n-1).
//
// TODO(poulson): Generalize to complex now that there is SOCP support.

template<typename Real>
void TV
( const AbstractDistMatrix<Real>& b,
        Real lambda,
        AbstractDistMatrix<Real>& x,
  const qp::affine::Ctrl<Real>& ctrl=qp::affine::Ctrl<Real>() );
template<typename Real>
void TV
( const Matrix<Real>& b,
        Real lambda,
        Matrix<Real>& x,
  const qp::affine::Ctrl<Real>& ctrl=qp::affine::Ctrl<Real>() );
template<typename Real>
void TV
( const DistMultiVec<Real>& b,
        Real lambda,
        DistMultiVec<Real>& x,
  const qp::affine::Ctrl<Real>& ctrl=qp::affine::Ctrl<Real>() );

// Long-only portfolio optimization
// ================================
// The long-only version of classical (Markowitz) portfolio optimization
// solves the problem
//
//   min gamma x^T Sigma x - c^T x,
//   s.t. 1^T x = 1, x >= 0,
//
// where 'c' is the vector of expected returns of various assets, 'gamma' is
// the risk-aversion parameter, and the covariance matrix, Sigma, can
// optionally be provided in the factored form
//
//   Sigma = D + F F^T,
//
// where D is diagonal and F has a (hopefully) small number of columns.

template<typename Real>
void LongOnlyPortfolio
( const SparseMatrix<Real>& Sigma,
  const Matrix<Real>& c,
        Real gamma,
        Matrix<Real>& x,
  const qp::direct::Ctrl<Real>& ctrl=qp::direct::Ctrl<Real>() );
template<typename Real>
void LongOnlyPortfolio
( const DistSparseMatrix<Real>& Sigma,
  const DistMultiVec<Real>& c,
        Real gamma,
        DistMultiVec<Real>& x,
  const qp::direct::Ctrl<Real>& ctrl=qp::direct::Ctrl<Real>() );

template<typename Real>
void LongOnlyPortfolio
( const Matrix<Real>& d,
  const SparseMatrix<Real>& F,
  const Matrix<Real>& c,
        Real gamma,
        Matrix<Real>& x,
  const socp::affine::Ctrl<Real>& ctrl=socp::affine::Ctrl<Real>() );
template<typename Real>
void LongOnlyPortfolio
( const DistMultiVec<Real>& d,
  const DistSparseMatrix<Real>& F,
  const DistMultiVec<Real>& c,
        Real gamma,
        DistMultiVec<Real>& x,
  const socp::affine::Ctrl<Real>& ctrl=socp::affine::Ctrl<Real>() );

} // namespace El

#endif // ifndef EL_OPTIMIZATION_MODELS_HPP
