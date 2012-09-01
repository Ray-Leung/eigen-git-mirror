// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2012 Giacomo Po <gpo@ucla.edu>
// Copyright (C) 2011 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.


#ifndef EIGEN_MINRES_H_
#define EIGEN_MINRES_H_


namespace Eigen {
    
    namespace internal {
        
        /** \internal Low-level MINRES algorithm
         * \param mat The matrix A
         * \param rhs The right hand side vector b
         * \param x On input and initial solution, on output the computed solution.
         * \param precond A preconditioner being able to efficiently solve for an
         *                approximation of Ax=b (regardless of b)
         * \param iters On input the max number of iteration, on output the number of performed iterations.
         * \param tol_error On input the tolerance error, on output an estimation of the relative error.
         */
        template<typename MatrixType, typename Rhs, typename Dest, typename Preconditioner>
        EIGEN_DONT_INLINE
        void minres(const MatrixType& mat, const Rhs& rhs, Dest& x,
                    const Preconditioner& precond, int& iters,
                    typename Dest::RealScalar& tol_error)
        {
            typedef typename Dest::RealScalar RealScalar;
            typedef typename Dest::Scalar Scalar;
            typedef Matrix<Scalar,Dynamic,1> VectorType;
            
            
            // initialize
            const int maxIters(iters);  // initialize maxIters to iters
            const int N(mat.cols());    // the size of the matrix
            const RealScalar rhsNorm2(rhs.squaredNorm());
//            const RealScalar threshold(tol_error); // threshold for original convergence criterion, see below
            const RealScalar threshold2(tol_error*tol_error*rhsNorm2); // convergence threshold
          

//            VectorType v(VectorType::Zero(N));
//            VectorType v_hat(rhs-mat*x);

            // Compute initial residual
            VectorType residual(rhs-mat*x);

            
            // Initialize preconditioned Lanczos
            VectorType v_old(N); // will be initialized inside loop
            VectorType v = VectorType::Zero(N); //initialize v
            VectorType v_new = residual; //initialize v_new
            VectorType w(N); // will be initialized inside loop
            VectorType w_new = precond.solve(v_new); // initialize w_new
            RealScalar beta; // will be initialized inside loop
            RealScalar beta_new = sqrt(v_new.dot(w_new));
            v_new /= beta_new;
            w_new /= beta_new;
            
            
            
            // RealScalar beta(v_hat.norm());
            RealScalar c(1.0); // the cosine of the Givens rotation
            RealScalar c_old(1.0);
            RealScalar s(0.0); // the sine of the Givens rotation
            RealScalar s_old(0.0); // the sine of the Givens rotation
            VectorType p_oold(VectorType::Zero(N)); // initialize p_oold=0
            VectorType p_old(p_oold); // initialize p_old=0
            VectorType p(N); // will be initialized in loop

            //RealScalar eta(beta); // CHANGE THIS
            RealScalar norm_rMR=beta;
            const RealScalar norm_r0(beta);
            
            RealScalar eta(1.0);
            
          //  VectorType v_old(N), Av(N), w_oold(N); // preallocate temporaty vectors used in iteration
            RealScalar residualNorm2; // not needed for original convergnce criterion
            
            int n = 0;
            while ( n < maxIters ){
                
                
                // Preconditioned Lanczos
                /* Note that there are 4 variants on the Lanczos algorithm. These are
                 * described in Paige, C. C. (1972). Computational variants of
                 * the Lanczos method for the eigenproblem. IMA Journal of Applied
                 * Mathematics, 10(3), 373–381. The current implementation corresonds 
                 * to the case A(2,7) in the paper. It also corresponds to 
                 * algorithm 6.14 in Y. Saad, Iterative Methods ￼￼￼for Sparse Linear 
                 * Systems, 2003 p.173. For the preconditioned version see 
                 * A. Greenbaum, Iterative Methods for Solving Linear Systems, SIAM (1987).
                 */
                beta = beta_new;
                v_old = v; // update: at first time step, this makes v_old = 0 so value of beta doesn't matter
                v = v_new; // update
                w = w_new; // update
                v_new.noalias() = mat*w - beta*v_old; // compute v_new
                const RealScalar alpha = v_new.dot(w);
                v_new -= alpha*v; // overwrite v_new
                w_new = precond.solve(v_new); // overwrite w_new
                beta_new = sqrt(v_new.dot(w_new)); // compute beta_new
                v_new /= beta_new; // overwrite v_new
                w_new /= beta_new; // overwrite w_new
                
//                
//                
//                
//                
//                
//                
//                
//                
// //               VectorType v_old(v); // now pre-allocated
//                v_old = v;
//                v=v_hat/beta;
////                VectorType Av(mat*v); // now pre-allocated
//                Av = mat*v;
//                RealScalar alpha(v.transpose()*Av);
//                v_hat=Av-alpha*v-beta*v_old;
//                RealScalar beta_old(beta);
//                beta=v_hat.norm();
                
                // Apply QR
//                RealScalar c_oold(c_old);   // store old-old cosine
//                c_old=c;    // store old cosine
//                RealScalar s_oold(s_old); // store old-old sine
//                s_old=s; // store old sine
//                const RealScalar r1_hat=c_old *alpha-c_oold*s_old *beta_old;
//                const RealScalar r1 =std::pow(std::pow(r1_hat,2)+std::pow(beta,2),0.5);
                const RealScalar r2 =s*alpha+c*c_old*beta; // s, s_old, c and c_old are still from previous iteration
                const RealScalar r3 =s_old*beta; // s, s_old, c and c_old are still from previous iteration
                
                // Compute new Givens rotation
                const RealScalar r1_hat=c*alpha-c_old*s*beta;
                const RealScalar r1 =std::pow(std::pow(r1_hat,2)+std::pow(beta_new,2),0.5);
                c_old = c; // store for next iteration
                s_old = s; // store for next iteration
                c=r1_hat/r1; // new cosine
                s=beta/r1; // new sine
                
                // update w
  //              VectorType w_oold(w_old); // now pre-allocated
                p_oold = p_old;
                p_old = p;
                p=(w-r2*p_old-r3*p_oold) /r1;
                // update x
                x += c*eta*p;
                norm_rMR *= std::fabs(s);
                
                residualNorm2 = (mat*x-rhs).squaredNorm(); // DOES mat*x NEED TO BE RECOMPUTED ????
                //if(norm_rMR/norm_r0 < threshold){ // original convergence criterion, does not require "mat*x"
                if ( residualNorm2 < threshold2){
                    break;
                }
                
                eta=-s*eta; // update eta
                n++;    // increment iteration
            }
            tol_error = std::sqrt(residualNorm2 / rhsNorm2); // return error 
            iters = n;  // return number of iterations
        }
        
    }
    
    template< typename _MatrixType, int _UpLo=Lower,
    typename _Preconditioner = DiagonalPreconditioner<typename _MatrixType::Scalar> >
    class MINRES;
    
    namespace internal {
        
        template< typename _MatrixType, int _UpLo, typename _Preconditioner>
        struct traits<MINRES<_MatrixType,_UpLo,_Preconditioner> >
        {
            typedef _MatrixType MatrixType;
            typedef _Preconditioner Preconditioner;
        };
        
    }
    
    /** \ingroup IterativeLinearSolvers_Module
     * \brief A minimal residual solver for sparse symmetric problems
     *
     * This class allows to solve for A.x = b sparse linear problems using the MINRES algorithm
     * of Paige and Saunders (1975). The sparse matrix A must be symmetric (possibly indefinite).
     * The vectors x and b can be either dense or sparse.
     *
     * \tparam _MatrixType the type of the sparse matrix A, can be a dense or a sparse matrix.
     * \tparam _UpLo the triangular part that will be used for the computations. It can be Lower
     *               or Upper. Default is Lower.
     * \tparam _Preconditioner the type of the preconditioner. Default is DiagonalPreconditioner
     *
     * The maximal number of iterations and tolerance value can be controlled via the setMaxIterations()
     * and setTolerance() methods. The defaults are the size of the problem for the maximal number of iterations
     * and NumTraits<Scalar>::epsilon() for the tolerance.
     *
     * This class can be used as the direct solver classes. Here is a typical usage example:
     * \code
     * int n = 10000;
     * VectorXd x(n), b(n);
     * SparseMatrix<double> A(n,n);
     * // fill A and b
     * MINRES<SparseMatrix<double> > mr;
     * mr.compute(A);
     * x = mr.solve(b);
     * std::cout << "#iterations:     " << mr.iterations() << std::endl;
     * std::cout << "estimated error: " << mr.error()      << std::endl;
     * // update b, and solve again
     * x = mr.solve(b);
     * \endcode
     *
     * By default the iterations start with x=0 as an initial guess of the solution.
     * One can control the start using the solveWithGuess() method. Here is a step by
     * step execution example starting with a random guess and printing the evolution
     * of the estimated error:
     * * \code
     * x = VectorXd::Random(n);
     * mr.setMaxIterations(1);
     * int i = 0;
     * do {
     *   x = mr.solveWithGuess(b,x);
     *   std::cout << i << " : " << mr.error() << std::endl;
     *   ++i;
     * } while (mr.info()!=Success && i<100);
     * \endcode
     * Note that such a step by step excution is slightly slower.
     *
     * \sa class ConjugateGradient, BiCGSTAB, SimplicialCholesky, DiagonalPreconditioner, IdentityPreconditioner
     */
    template< typename _MatrixType, int _UpLo, typename _Preconditioner>
    class MINRES : public IterativeSolverBase<MINRES<_MatrixType,_UpLo,_Preconditioner> >
    {
        
        typedef IterativeSolverBase<MINRES> Base;
        using Base::mp_matrix;
        using Base::m_error;
        using Base::m_iterations;
        using Base::m_info;
        using Base::m_isInitialized;
    public:
        typedef _MatrixType MatrixType;
        typedef typename MatrixType::Scalar Scalar;
        typedef typename MatrixType::Index Index;
        typedef typename MatrixType::RealScalar RealScalar;
        typedef _Preconditioner Preconditioner;
        
        enum {UpLo = _UpLo};
        
    public:
        
        /** Default constructor. */
        MINRES() : Base() {}
        
        /** Initialize the solver with matrix \a A for further \c Ax=b solving.
         *
         * This constructor is a shortcut for the default constructor followed
         * by a call to compute().
         *
         * \warning this class stores a reference to the matrix A as well as some
         * precomputed values that depend on it. Therefore, if \a A is changed
         * this class becomes invalid. Call compute() to update it with the new
         * matrix A, or modify a copy of A.
         */
        MINRES(const MatrixType& A) : Base(A) {}
        
        /** Destructor. */
        ~MINRES(){}
		
        /** \returns the solution x of \f$ A x = b \f$ using the current decomposition of A
         * \a x0 as an initial solution.
         *
         * \sa compute()
         */
        template<typename Rhs,typename Guess>
        inline const internal::solve_retval_with_guess<MINRES, Rhs, Guess>
        solveWithGuess(const MatrixBase<Rhs>& b, const Guess& x0) const
        {
            eigen_assert(m_isInitialized && "MINRES is not initialized.");
            eigen_assert(Base::rows()==b.rows()
                         && "MINRES::solve(): invalid number of rows of the right hand side matrix b");
            return internal::solve_retval_with_guess
            <MINRES, Rhs, Guess>(*this, b.derived(), x0);
        }
        
        /** \internal */
        template<typename Rhs,typename Dest>
        void _solveWithGuess(const Rhs& b, Dest& x) const
        {
            m_iterations = Base::maxIterations();
            m_error = Base::m_tolerance;
            
            for(int j=0; j<b.cols(); ++j)
            {
                m_iterations = Base::maxIterations();
                m_error = Base::m_tolerance;
                
                typename Dest::ColXpr xj(x,j);
                internal::minres(mp_matrix->template selfadjointView<UpLo>(), b.col(j), xj,
                                 Base::m_preconditioner, m_iterations, m_error);
            }
            
            m_isInitialized = true;
            m_info = m_error <= Base::m_tolerance ? Success : NoConvergence;
        }
        
        /** \internal */
        template<typename Rhs,typename Dest>
        void _solve(const Rhs& b, Dest& x) const
        {
            x.setOnes();
            _solveWithGuess(b,x);
        }
        
    protected:
        
    };
    
    namespace internal {
        
        template<typename _MatrixType, int _UpLo, typename _Preconditioner, typename Rhs>
        struct solve_retval<MINRES<_MatrixType,_UpLo,_Preconditioner>, Rhs>
        : solve_retval_base<MINRES<_MatrixType,_UpLo,_Preconditioner>, Rhs>
        {
            typedef MINRES<_MatrixType,_UpLo,_Preconditioner> Dec;
            EIGEN_MAKE_SOLVE_HELPERS(Dec,Rhs)
            
            template<typename Dest> void evalTo(Dest& dst) const
            {
                dec()._solve(rhs(),dst);
            }
        };
        
    } // end namespace internal
    
} // end namespace Eigen

#endif // EIGEN_MINRES_H

