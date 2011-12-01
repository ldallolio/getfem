// -*- c++ -*- (enables emacs c++ mode)
//===========================================================================
//
// Copyright (C) 2000-2010 Yves Renard
//
// This file is a part of GETFEM++
//
// Getfem++  is  free software;  you  can  redistribute  it  and/or modify it
// under  the  terms  of the  GNU  Lesser General Public License as published
// by  the  Free Software Foundation;  either version 2.1 of the License,  or
// (at your option) any later version.
// This program  is  distributed  in  the  hope  that it will be useful,  but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// You  should  have received a copy of the GNU Lesser General Public License
// along  with  this program;  if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
//
//===========================================================================


#include "getfem/getfem_models.h"
#include "getfem/getfem_nonlinear_elasticity.h"

namespace getfem {


  /* Usefull functions to compute the invariants and their derivatives
     Note that the second derivative is symmetrized (see the user
     documentation for more details). The matrix E is assumed to be symmetric.
  */


  static scalar_type frobenius_product_trans(const base_matrix &A,
					     const base_matrix &B) {
    size_type N = gmm::mat_nrows(A);
    scalar_type res = scalar_type(0);
    for (size_type i = 0; i < N; ++i)
      for (size_type j = 0; j < N; ++j)
	res += A(i, j) * B(j, i);
    return res;
  }

  struct compute_invariants {
    
    const base_matrix &E;
    base_matrix Einv;
    size_type N;
    scalar_type i1_, i2_, i3_, j1_, j2_;
    bool i1_c, i2_c, i3_c, j1_c, j2_c;

    base_matrix di1, di2, di3, dj1, dj2; 
    bool di1_c, di2_c, di3_c, dj1_c, dj2_c;

    base_tensor ddi1, ddi2, ddi3, ddj1, ddj2; 
    bool ddi1_c, ddi2_c, ddi3_c, ddj1_c, ddj2_c;


    /* First invariant tr(E) */
    void compute_i1(void) {
      i1_ = gmm::mat_trace(E);
      i1_c = true;
    }

    void compute_di1(void) {
      gmm::resize(di1, N, N);
      gmm::copy(gmm::identity_matrix(), di1);
      di1_c = true;
    }

    void compute_ddi1(void) { // not very usefull, null tensor
      ddi1 = base_tensor(N, N, N, N); 
      ddi1_c = true;
    }

    inline scalar_type i1(void)
    { if (!i1_c) compute_i1(); return i1_; }

    inline const base_matrix &grad_i1(void)
    { if (!di1_c) compute_di1(); return di1; }

    inline const base_tensor &sym_grad_grad_i1(void)
    { if (!ddi1_c) compute_ddi1(); return ddi1; }


    /* Second invariant (tr(E)^2 - tr(E^2))/2 */
    void compute_i2(void) {
      i2_ = (gmm::sqr(gmm::mat_trace(E))
	     - frobenius_product_trans(E, E)) / scalar_type(2);
      i2_c = true;
    }

    void compute_di2(void) {
      gmm::resize(di2, N, N);
      gmm::copy(gmm::identity_matrix(), di2);
      gmm::scale(di2, i1());
      // gmm::add(gmm::scale(gmm::transposed(E), -scalar_type(1)), di2);
      gmm::add(gmm::scaled(E, -scalar_type(1)), di2);
      di2_c = true;
    }

    void compute_ddi2(void) {
      ddi2 = base_tensor(N, N, N, N);
      for (size_type i = 0; i < N; ++i)
	for (size_type k = 0; k < N; ++k)
	  ddi2(i,i,k,k) += scalar_type(1);
      for (size_type i = 0; i < N; ++i)
	for (size_type j = 0; j < N; ++j) {
	  ddi2(i,j,j,i) -= scalar_type(1)/scalar_type(2);
	  ddi2(j,i,j,i) -= scalar_type(1)/scalar_type(2);
	}
      ddi2_c = true;
    }

    inline scalar_type i2(void)
    { if (!i2_c) compute_i2(); return i2_; }

    inline const base_matrix &grad_i2(void)
    { if (!di2_c) compute_di2(); return di2; }

    inline const base_tensor &sym_grad_grad_i2(void)
    { if (!ddi2_c) compute_ddi2(); return ddi2; }

    /* Third invariant det(E) */
    void compute_i3(void) {
      Einv = E;
      i3_ = gmm::lu_inverse(Einv);
      i3_c = true;
    }

    void compute_di3(void) {
      scalar_type det = i3();
      // gmm::resize(di3, N, N);
      // gmm::copy(gmm::transposed(E), di3);
      di3 = Einv;
      // gmm::lu_inverse(di3);
      gmm::scale(di3, det);
      di3_c = true;
    }

    void compute_ddi3(void) {
      ddi3 = base_tensor(N, N, N, N);
      scalar_type det = i3() / scalar_type(2); // computes also E inverse.
      for (size_type i = 0; i < N; ++i)
	for (size_type j = 0; j < N; ++j)
	  for (size_type k = 0; k < N; ++k)
	    for (size_type l = 0; l < N; ++l)
	      ddi3(i,j,k,l) = det*(Einv(j,i)*Einv(l,k) - Einv(j,k)*Einv(l,i)
				 + Einv(i,j)*Einv(l,k) - Einv(i,k)*Einv(l,j));
      ddi3_c = true;
    }

    inline scalar_type i3(void)
    { if (!i3_c) compute_i3(); return i3_; }

    inline const base_matrix &grad_i3(void)
    { if (!di3_c) compute_di3(); return di3; }

    inline const base_tensor &sym_grad_grad_i3(void)
    { if (!ddi3_c) compute_ddi3(); return ddi3; }

    /* Invariant j1(E) = i1(E)*i3(E)^(-1/3) */
    void compute_j1(void) {
      j1_ = i1() * ::pow(gmm::abs(i3()), -scalar_type(1) / scalar_type(3));
      j1_c = true;
    }

    void compute_dj1(void) {
      dj1 = grad_i1();
      gmm::add(gmm::scaled(grad_i3(), -i1() / (scalar_type(3) * i3())), dj1);
      gmm::scale(dj1, ::pow(gmm::abs(i3()), -scalar_type(1) / scalar_type(3)));
      dj1_c = true;
    }

    void compute_ddj1(void) {
      const base_matrix &di1_ = grad_i1(); 
      const base_matrix &di3_ = grad_i3();
      scalar_type coeff1 = scalar_type(1) / (scalar_type(3)*i3());
      scalar_type coeff2 = scalar_type(4) * coeff1 * coeff1 * i1();
      ddj1 = sym_grad_grad_i3();
      gmm::scale(ddj1.as_vector(), -i1() * coeff1);
      
      for (size_type i = 0; i < N; ++i)
 	for (size_type j = 0; j < N; ++j)
 	  for (size_type k = 0; k < N; ++k)
 	    for (size_type l = 0; l < N; ++l)
	      ddj1(i,j,k,l) +=
		(di3_(i, j) * di3_(k, l)) * coeff2
		- (di1_(i, j) * di3_(k, l) + di1_(k, l) * di3_(i, j)) * coeff1;

      gmm::scale(ddj1.as_vector(),
		 ::pow(gmm::abs(i3()), -scalar_type(1)/scalar_type(3)));
      ddj1_c = true;
    }

    inline scalar_type j1(void)
    { if (!j1_c) compute_j1(); return j1_; }

    inline const base_matrix &grad_j1(void)
    { if (!dj1_c) compute_dj1(); return dj1; }

    inline const base_tensor &sym_grad_grad_j1(void)
    { if (!ddj1_c) compute_ddj1(); return ddj1; }

    /* Invariant j2(E) = i2(E)*i3(E)^(-2/3) */
    void compute_j2(void) {
      j2_ = i2() * ::pow(gmm::abs(i3()), -scalar_type(2) / scalar_type(3));
      j2_c = true;
    }

    void compute_dj2(void) {
      dj2 = grad_i2();
      gmm::add(gmm::scaled(grad_i3(), -scalar_type(2) * i2() / (scalar_type(3) * i3())), dj2);
      gmm::scale(dj2, ::pow(gmm::abs(i3()), -scalar_type(2) / scalar_type(3)));
      dj2_c = true;
    }

    void compute_ddj2(void) {
      const base_matrix &di2_ = grad_i2(); 
      const base_matrix &di3_ = grad_i3();
      scalar_type coeff1 = scalar_type(2) / (scalar_type(3)*i3());
      scalar_type coeff2 = scalar_type(5) * coeff1 * coeff1 * i2()
	                   / scalar_type(2);
      ddj2 = sym_grad_grad_i2();
      gmm::add(gmm::scaled(sym_grad_grad_i3().as_vector(), -i2() * coeff1),
	       ddj2.as_vector());
      
      for (size_type i = 0; i < N; ++i)
 	for (size_type j = 0; j < N; ++j)
 	  for (size_type k = 0; k < N; ++k)
 	    for (size_type l = 0; l < N; ++l)
	      ddj2(i,j,k,l) +=
		(di3_(i, j) * di3_(k, l)) * coeff2
		- (di2_(i, j) * di3_(k, l) + di2_(k, l) * di3_(i, j)) * coeff1;

      gmm::scale(ddj2.as_vector(),
		 ::pow(gmm::abs(i3()), -scalar_type(2)/scalar_type(3)));
      ddj2_c = true;
    }


    inline scalar_type j2(void)
    { if (!j2_c) compute_j2(); return j2_; }
   
    inline const base_matrix &grad_j2(void)
    { if (!dj2_c) compute_dj2(); return dj2; }

    inline const base_tensor &sym_grad_grad_j2(void)
    { if (!ddj2_c) compute_ddj2(); return ddj2; }


    compute_invariants(const base_matrix &EE)
      : E(EE), i1_c(false), i2_c(false), i3_c(false),
	j1_c(false), j2_c(false), di1_c(false), di2_c(false), di3_c(false),
	dj1_c(false), dj2_c(false), ddi1_c(false), ddi2_c(false),
	ddi3_c(false), ddj1_c(false), ddj2_c(false)
      { N = gmm::mat_nrows(E); }

  };


 


  /* Symmetry check */

  int check_symmetry(const base_tensor &t) {
    int flags = 7; size_type N = 3;
    for (size_type n = 0; n < N; ++n)
      for (size_type m = 0; m < N; ++m)
	for (size_type l = 0; l < N; ++l)
	  for (size_type k = 0; k < N; ++k) {
	    if (gmm::abs(t(n,m,l,k) - t(l,k,n,m))>1e-5) flags &= (~1); 
	    if (gmm::abs(t(n,m,l,k) - t(m,n,l,k))>1e-5) flags &= (~2); 
	    if (gmm::abs(t(n,m,l,k) - t(n,m,k,l))>1e-5) flags &= (~4);
	  }
    return flags;
  }

  /* Member functions of hyperelastic laws */

  void abstract_hyperelastic_law::random_E(base_matrix &E) {
    size_type N = gmm::mat_nrows(E);
    base_matrix Phi(N,N);
    scalar_type d;
    do {
      gmm::fill_random(Phi);
      d = gmm::lu_det(Phi);
    } while (d < scalar_type(0.01)); 
    gmm::mult(gmm::transposed(Phi),Phi,E);
    gmm::scale(E,-1.); gmm::add(gmm::identity_matrix(),E); 
    gmm::scale(E,-0.5);
  }

  void abstract_hyperelastic_law::test_derivatives
  (size_type N, scalar_type h, const base_vector& param) const {
    base_matrix E(N,N), E2(N,N), DE(N,N);
    bool ok = true;

    for (size_type count = 0; count < 100; ++count) {
      random_E(E); random_E(DE);
      gmm::scale(DE, h);
      gmm::add(E, DE, E2);
      
      base_matrix sigma1(N,N), sigma2(N,N);
      getfem::base_tensor tdsigma(N,N,N,N);
      base_matrix dsigma(N,N);
      gmm::copy(E, E2); gmm::add(DE, E2);
      sigma(E, sigma1, param); sigma(E2, sigma2, param);
      
      scalar_type d = strain_energy(E2, param) - strain_energy(E, param);
      scalar_type d2 = 0;
      for (size_type i=0; i < N; ++i) 
	for (size_type j=0; j < N; ++j) d2 += sigma1(i,j)*DE(i,j);
      if (gmm::abs(d-d2)/(gmm::abs(d)+1e-40) > 1e-4) {
	cout << "Test " << count << " wrong derivative of strain_energy, d="
	     << d/h << ", d2=" << d2/h << endl;
	ok = false;
      }
      
      grad_sigma(E,tdsigma,param);
      for (size_type i=0; i < N; ++i) {
	for (size_type j=0; j < N; ++j) {
	  dsigma(i,j) = 0;
	  for (size_type k=0; k < N; ++k) {
	    for (size_type m=0; m < N; ++m) {
	      dsigma(i,j) += tdsigma(i,j,k,m)*DE(k,m);
	    }
	  }
	  sigma2(i,j) -= sigma1(i,j);
	  if (gmm::abs(dsigma(i,j) - sigma2(i,j))
	      /(gmm::abs(dsigma(i,j)) + 1e-40) > 1.5e-4) {
	    cout << "Test " << count << " wrong derivative of sigma, i="
		 << i << ", j=" << j << ", dsigma=" << dsigma(i,j)/h
		 << ", var sigma = " << sigma2(i,j)/h << endl;
	    ok = false;
	  }
	}
      }
    }
    GMM_ASSERT1(ok, "Derivative test has failed");
  }
    
  scalar_type SaintVenant_Kirchhoff_hyperelastic_law::strain_energy
  (const base_matrix &E, const base_vector &params) const {
    return gmm::sqr(gmm::mat_trace(E)) * params[0] / scalar_type(2)
      + gmm::mat_euclidean_norm_sqr(E) * params[1];
  }
  
  void SaintVenant_Kirchhoff_hyperelastic_law::sigma
  (const base_matrix &E, base_matrix &result,const base_vector &params) const {
    gmm::copy(gmm::identity_matrix(), result);
    gmm::scale(result, params[0] * gmm::mat_trace(E));
    gmm::add(gmm::scaled(E, 2 * params[1]), result);
  }
  void SaintVenant_Kirchhoff_hyperelastic_law::grad_sigma
  (const base_matrix &E, base_tensor &result,const base_vector &params) const {
    std::fill(result.begin(), result.end(), scalar_type(0));
    size_type N = gmm::mat_nrows(E);
    for (size_type i = 0; i < N; ++i)
      for (size_type l = 0; l < N; ++l) {
	result(i, i, l, l) = params[0];
	result(i, l, i, l) += params[1];
	result(i, l, l, i) += params[1];
      }
  }

  SaintVenant_Kirchhoff_hyperelastic_law::SaintVenant_Kirchhoff_hyperelastic_law(void) {
    // an attempt, the first term is missing grad(h)sigma:grad(v)
//     adapted_tangent_term_assembly_fem_data = "params=data$1(#2,2);"
//       "t=comp(NonLin$2(#1)(i,j).vGrad(#1)(:,i,j).NonLin$2(#1)(k,l).vGrad(#1)(:,k,l).Base(#2)(:));"
//       "u=comp(NonLin$2(#1)(j,i).vGrad(#1)(:,j,k).NonLin$2(#1)(l,i).vGrad(#1)(:,l,k).Base(#2)(:));" 
//       "v=comp(NonLin$2(#1)(j,i).vGrad(#1)(:,j,k).NonLin$2(#1)(l,k).vGrad(#1)(:,l,i).Base(#2)(:));"
//       "M(#1,#1)+= t(:,:,i).params(i,1) + u(:,:,i).params(i,2) + v(:,:,i).params(i,2)";

//     adapted_tangent_term_assembly_cte_data = "params=data$1(2);"
//       "t=sym(comp(NonLin$2(#1)(i,j).vGrad(#1)(:,i,j).NonLin$2(#1)(k,l).vGrad(#1)(:,k,l)));"
//       "u=sym(comp(NonLin$2(#1)(j,i).vGrad(#1)(:,j,k).NonLin$2(#1)(l,i).vGrad(#1)(:,l,k)));" 
//       "v=sym(comp(NonLin$2(#1)(j,i).vGrad(#1)(:,j,k).NonLin$2(#1)(l,k).vGrad(#1)(:,l,i)));"
//       "M(#1,#1)+= t(:,:).params(1) + u(:,:).params(2) + v(:,:).params(2)";

// not efficient at all
//     adapted_tangent_term_assembly_cte_data = "params=data$1(2);"
//       "t=comp(NonLin$2(#1).vGrad(#1).NonLin$2(#1).vGrad(#1));"
//       "M(#1,#1)+= t(i,j,:,i,j,k,l,:,k,l).params(1);"
//       "M(#1,#1)+= t(j,i,:,j,k,l,i,:,l,k).params(2);"
//       "M(#1,#1)+= t(j,i,:,j,k,l,k,:,l,i).params(2);";

    nb_params_ = 2;
  }

  scalar_type membrane_elastic_law::strain_energy
  (const base_matrix & /* E */, const base_vector & /* params */) const {
    // to be done if needed
    GMM_ASSERT1(false, "To be done");
    return 0;
  }
  
  void membrane_elastic_law::sigma
  (const base_matrix &E, base_matrix &result,const base_vector &params) const {
    // should be optimized, maybe deriving sigma from strain energy
    base_tensor tt(2,2,2,2);
    size_type N = gmm::mat_nrows(E);
    grad_sigma(E,tt,params);
    for (size_type i = 0; i < N; ++i)
      for (size_type j = 0; j < N; ++j) {
	result(i,j)=0.0;
	for (size_type k = 0; k < N; ++k)
	  for (size_type l = 0; l < N; ++l) 
	    result(i,j)+=tt(i,j,k,l)*E(k,l);
      }
    // add pretension in X' direction
    if(params[4]!=0) result(0,0)+=params[4];	
    // add pretension in Y' direction
    if(params[5]!=0) result(1,1)+=params[5];
    //	cout<<"sigma="<<result<<endl;
  }
  
  void membrane_elastic_law::grad_sigma
  (const base_matrix & /* E */, base_tensor &result,
   const base_vector &params) const {
    // to be optimized!!
    std::fill(result.begin(), result.end(), scalar_type(0));
    scalar_type poisonXY=params[0]*params[1]/params[2];	//Ex*vYX=Ey*vXY
    scalar_type Ghalf=( params[3] == 0) ? params[0]/(4*(1+params[1])) : params[3]/2;	//if no G entered, compute G=E/(2*(1+v))	to be cfmd!!
    std::fill(result.begin(), result.end(), scalar_type(0));
    result(0,0,0,0) = params[0]/(1-params[1]*poisonXY);
    // result(0,0,0,1) = 0;
    // result(0,0,1,0) = 0;
    result(0,0,1,1) = params[1]*params[0]/(1-params[1]*poisonXY);
    result(1,1,0,0) = params[1]*params[0]/(1-params[1]*poisonXY);
    // result(1,1,0,1) = 0;
    // result(1,1,1,0) = 0;
    result(1,1,1,1) = params[2]/(1-params[1]*poisonXY);
    // result(0,1,0,0) = 0;
    result(0,1,0,1) = Ghalf;
    result(0,1,1,0) = Ghalf;
    // result(0,1,1,1) = 0;
    // result(1,0,0,0) = 0;
    result(1,0,0,1) = Ghalf;
    result(1,0,1,0) = Ghalf;
    // result(1,0,1,1) = 0;
  }

  scalar_type Mooney_Rivlin_hyperelastic_law::strain_energy
  (const base_matrix &E, const base_vector &params) const {
    scalar_type C1 = params[0], C2 = params[1];
    size_type N = gmm::mat_nrows(E);
    GMM_ASSERT1(N == 3, "Mooney Rivlin hyperelastic law only defined "
		"on dimension 3, sorry");
    base_matrix C = E;
    gmm::scale(C, scalar_type(2));
    gmm::add(gmm::identity_matrix(), C);
    compute_invariants ci(C);

    return C1*(ci.j1() - scalar_type(3)) + C2*(ci.j2() - scalar_type(3));
  }

  void Mooney_Rivlin_hyperelastic_law::sigma
  (const base_matrix &E, base_matrix &result,
   const base_vector &params) const {
    scalar_type C1 = params[0], C2 = params[1];
    size_type N = gmm::mat_nrows(E);
    GMM_ASSERT1(N == 3, "Mooney Rivlin hyperelastic law only defined "
		"on dimension 3, sorry");
    base_matrix C = E;
    gmm::scale(C, scalar_type(2));
    gmm::add(gmm::identity_matrix(), C);
    compute_invariants ci(C);

    gmm::copy(gmm::scaled(ci.grad_j1(), scalar_type(2)*C1), result);
    gmm::add(gmm::scaled(ci.grad_j2(), scalar_type(2)*C2), result);

  }

  void Mooney_Rivlin_hyperelastic_law::grad_sigma
  (const base_matrix &E, base_tensor &result,
   const base_vector &params) const {
    scalar_type C1 = params[0], C2 = params[1];
    size_type N = gmm::mat_nrows(E);
    GMM_ASSERT1(N == 3, "Mooney Rivlin hyperelastic law only defined "
		"on dimension 3, sorry");
    base_matrix C = E;
    gmm::scale(C, scalar_type(2));
    gmm::add(gmm::identity_matrix(), C);
    compute_invariants ci(C);

    gmm::copy(gmm::scaled(ci.sym_grad_grad_j1().as_vector(),
			  scalar_type(4)*C1), result.as_vector());
    gmm::add(gmm::scaled(ci.sym_grad_grad_j2().as_vector(),
			 scalar_type(4)*C2), result.as_vector());
    
//     GMM_ASSERT1(check_symmetry(result) == 7,
// 		"Fourth order tensor not symmetric : " << result);
  }

  Mooney_Rivlin_hyperelastic_law::Mooney_Rivlin_hyperelastic_law(void) {
    nb_params_ = 2;
  }




  scalar_type generalized_Blatz_Ko_hyperelastic_law::strain_energy
  (const base_matrix &E, const base_vector &params) const {
    scalar_type a = params[0], b = params[1], c = params[2], d = params[3];
    size_type n = size_type(params[4]+0.1);
    size_type N = gmm::mat_nrows(E);
    GMM_ASSERT1(N == 3, "Generalized Blatz Ko hyperelastic law only defined "
		"on dimension 3, sorry");
    base_matrix C = E;
    gmm::scale(C, scalar_type(2));
    gmm::add(gmm::identity_matrix(), C);
    compute_invariants ci(C);

    return pow(a*ci.i1() + b*sqrt(gmm::abs(ci.i3()))
	       + c*ci.i2() / ci.i3() + d, scalar_type(n));
  }

  void generalized_Blatz_Ko_hyperelastic_law::sigma
  (const base_matrix &E, base_matrix &result,
   const base_vector &params) const {
    scalar_type a = params[0], b = params[1], c = params[2], d = params[3];
    size_type n = size_type(params[4]+0.1);
    size_type N = gmm::mat_nrows(E);
    GMM_ASSERT1(N == 3, "Generalized Blatz Ko hyperelastic law only defined "
		"on dimension 3, sorry");
    base_matrix C = E;
    gmm::scale(C, scalar_type(2));
    gmm::add(gmm::identity_matrix(), C);
    compute_invariants ci(C);

    scalar_type z = a*ci.i1() + b*sqrt(gmm::abs(ci.i3()))
      + c*ci.i2() / ci.i3() + d;
    scalar_type nz = scalar_type(n) * pow(z, scalar_type(n-1));
    scalar_type di1 = nz * a;
    scalar_type di2 = nz * c / ci.i3();
    scalar_type di3 = nz *
      (b / (2. * sqrt(gmm::abs(ci.i3()))) - c * ci.i2() / gmm::sqr(ci.i3()));

    gmm::copy(gmm::scaled(ci.grad_i1(), di1 * 2.0), result);
    gmm::add(gmm::scaled(ci.grad_i2(), di2 * 2.0), result);
    gmm::add(gmm::scaled(ci.grad_i3(), di3 * 2.0), result);
  }

  void generalized_Blatz_Ko_hyperelastic_law::grad_sigma
  (const base_matrix &E, base_tensor &result,
   const base_vector &params) const {
    scalar_type a = params[0], b = params[1], c = params[2], d = params[3];
    size_type n = size_type(params[4]+0.1);
    size_type N = gmm::mat_nrows(E);
    GMM_ASSERT1(N == 3, "Generalized Blatz Ko hyperelastic law only defined "
		"on dimension 3, sorry");
    base_matrix C = E;
    gmm::scale(C, scalar_type(2));
    gmm::add(gmm::identity_matrix(), C);
    compute_invariants ci(C);


    scalar_type z = a*ci.i1() + b*sqrt(gmm::abs(ci.i3()))
      + c*ci.i2() / ci.i3() + d;
    scalar_type nz = scalar_type(n) * pow(z, scalar_type(n-1));
    scalar_type di1 = nz * a;
    scalar_type di2 = nz * c / ci.i3();
    scalar_type y = (b / (2. * sqrt(gmm::abs(ci.i3()))) - c * ci.i2() / gmm::sqr(ci.i3()));
    scalar_type di3 = nz * y;

    gmm::copy(gmm::scaled(ci.sym_grad_grad_i1().as_vector(),
			  scalar_type(4)*di1), result.as_vector());
    gmm::add(gmm::scaled(ci.sym_grad_grad_i2().as_vector(),
			 scalar_type(4)*di2), result.as_vector());
    gmm::add(gmm::scaled(ci.sym_grad_grad_i3().as_vector(),
			 scalar_type(4)*di3), result.as_vector());

    scalar_type nnz = scalar_type(n * (n-1)) * pow(z, scalar_type(n-1));
    base_matrix A(3, 3); // second derivatives of W with respect to invariants
    A(0, 0) = nnz * a * a;
    A(1, 0) = A(0, 1) = nnz * a * c / ci.i3();
    A(2, 0) = A(0, 2) = nnz * a * y;
    A(1, 1) = nnz * c * c / gmm::sqr(ci.i3());
    A(2, 1) = A(1, 2) = nnz * y * c / ci.i3();
    A(2, 2) = nnz * y * y + nz * (2. * c * ci.i2() / pow(ci.i3(), 4.) - b / (4. * pow(ci.i3(), 1.5)));

    typedef const base_matrix * pointer_base_matrix__;
    pointer_base_matrix__ di[3];
    di[0] = &(ci.grad_i1()); 
    di[1] = &(ci.grad_i2()); 
    di[2] = &(ci.grad_i3());

    for (size_type j = 0; j < N; ++j)
      for (size_type k = 0; k < N; ++k) {
	for (size_type l1 = 0; l1 < N; ++l1)
	  for (size_type l2 = 0; l2 < N; ++l2)
	    for (size_type l3 = 0; l3 < N; ++l3)
	      for (size_type l4 = 0; l4 < N; ++l4)
		result(l1, l2, l3, l4)
		  += A(j, k) * (*di[j])(l1, l2) * (*di[k])(l3, l4);
      }

//     GMM_ASSERT1(check_symmetry(result) == 7,
// 		"Fourth order tensor not symmetric : " << result);
  }

  generalized_Blatz_Ko_hyperelastic_law::generalized_Blatz_Ko_hyperelastic_law(void) {
    nb_params_ = 5;
  }


  scalar_type Ciarlet_Geymonat_hyperelastic_law::strain_energy
  (const base_matrix &E, const base_vector &params) const {
    size_type N = gmm::mat_nrows(E);
    scalar_type a = params[1] + params[2] / scalar_type(2);
    scalar_type b = -(params[1] + params[2]) / scalar_type(2);
    scalar_type c = params[0]/scalar_type(4)  - b;
    scalar_type d = params[0]/scalar_type(2) + params[1];
    //scalar_type d = params[0] - scalar_type(2)*params[2] - scalar_type(4)*b;
    scalar_type e = -(scalar_type(3)*(a+b) + c);
    base_matrix C(N, N);
    gmm::copy(gmm::scaled(E, scalar_type(2)), C);
    gmm::add(gmm::identity_matrix(), C);
    scalar_type det = gmm::lu_det(C);
    return a * gmm::mat_trace(C)
      + b * (gmm::sqr(gmm::mat_trace(C)) - 
	     gmm::mat_euclidean_norm_sqr(C))/scalar_type(2)
      + c * det - d * log(det) / scalar_type(2) + e;
  }

  void Ciarlet_Geymonat_hyperelastic_law::sigma
  (const base_matrix &E, base_matrix &result,const base_vector &params) const {
    size_type N = gmm::mat_nrows(E);
    scalar_type a = params[1] + params[2] / scalar_type(2);
    scalar_type b = -(params[1] + params[2]) / scalar_type(2);
    scalar_type c = params[0]/scalar_type(4)  - b;
    scalar_type d = params[0]/scalar_type(2) + params[1]; 
    //d=params[0] - scalar_type(2)*params[2] - scalar_type(4)*b;
    base_matrix C(N, N);
    assert(gmm::abs(2*a+4*b+2*c-d)<1e-5);
    gmm::copy(gmm::scaled(E, scalar_type(2)), C);
    gmm::add(gmm::identity_matrix(), C);
    gmm::copy(gmm::identity_matrix(), result);
    gmm::scale(result, scalar_type(2) * (a + b * gmm::mat_trace(C)));
    gmm::add(gmm::scaled(C, -scalar_type(2) * b), result);
    scalar_type det = gmm::lu_inverse(C);
    gmm::add(gmm::scaled(C, scalar_type(2) * c * det - d), result);
  }

  void Ciarlet_Geymonat_hyperelastic_law::grad_sigma
  (const base_matrix &E, base_tensor &result,const base_vector &params) const {
    size_type N = gmm::mat_nrows(E);
    scalar_type b2 = -(params[1] + params[2]); // b * 2
    scalar_type c = (params[0]  - 2*b2) / scalar_type(4);
    //scalar_type d = params[0] - scalar_type(2)*params[2] - 2*b2;
    scalar_type d = params[0]/scalar_type(2) + params[1]; 
    base_matrix C(N, N);
    gmm::copy(gmm::scaled(E, scalar_type(2)), C);
    gmm::add(gmm::identity_matrix(), C);
    scalar_type det = gmm::lu_inverse(C);
    std::fill(result.begin(), result.end(), scalar_type(0));
    for (size_type i = 0; i < N; ++i)
      for (size_type j = 0; j < N; ++j) {
	result(i, i, j, j) += 2*b2;
	result(i, j, i, j) -= b2;
	result(i, j, j, i) -= b2;
	for (size_type  k = 0; k < N; ++k)
	  for (size_type  l = 0; l < N; ++l)
	    result(i, j, k, l) += 
	      (C(i, k)*C(l, j) + C(i, l)*C(k, j)) * (d-scalar_type(2)*det*c)
	      + (C(i, j) * C(k, l)) * det*c*scalar_type(4);
      }

//     GMM_ASSERT1(check_symmetry(result) == 7,
// 		"Fourth order tensor not symmetric : " << result);
  }


  int levi_civita(int i, int j, int k) {
    int ii=i+1;
    int jj=j+1;
    int kk=k+1;	//i,j,k from 0 to 2 !
    return static_cast<int>
      (int(- 1)*(static_cast<int>(pow(double(ii-jj),2.))%3)
       * (static_cast<int> (pow(double(ii-kk),2))%3 )
       * (static_cast<int> (pow(double(jj-kk),2))%3)
       * (pow(double(jj-(ii%3))-double(0.5),2)-double(1.25)));
  }



  scalar_type plane_strain_hyperelastic_law::strain_energy
  (const base_matrix &E, const base_vector &params) const {
    GMM_ASSERT1(gmm::mat_nrows(E) == 2, "Plane strain law is for 2D only.");
    base_matrix E3D(3,3);
    E3D(0,0)=E(0,0); E3D(1,0)=E(1,0); E3D(0,1)=E(0,1); E3D(1,1)=E(1,1); 
    return pl->strain_energy(E3D, params);
  }

  void plane_strain_hyperelastic_law::sigma
  (const base_matrix &E, base_matrix &result,const base_vector &params) const {
    GMM_ASSERT1(gmm::mat_nrows(E) == 2, "Plane strain law is for 2D only.");
    base_matrix E3D(3,3), result3D(3,3);
    E3D(0,0)=E(0,0); E3D(1,0)=E(1,0); E3D(0,1)=E(0,1); E3D(1,1)=E(1,1);
    pl->sigma(E3D, result3D, params);
    result(0,0) = result3D(0,0); result(1,0) = result3D(1,0);
    result(0,1) = result3D(0,1); result(1,1) = result3D(1,1);
  }

  void plane_strain_hyperelastic_law::grad_sigma
  (const base_matrix &E, base_tensor &result,const base_vector &params) const {
    GMM_ASSERT1(gmm::mat_nrows(E) == 2, "Plane strain law is for 2D only.");
    base_matrix E3D(3,3);
    base_tensor result3D(3,3,3,3);
    E3D(0,0)=E(0,0); E3D(1,0)=E(1,0); E3D(0,1)=E(0,1); E3D(1,1)=E(1,1);
    pl->grad_sigma(E3D, result3D, params);
    result(0,0,0,0) = result3D(0,0,0,0); result(1,0,0,0) = result3D(1,0,0,0);
    result(0,1,0,0) = result3D(0,1,0,0); result(1,1,0,0) = result3D(1,1,0,0);
    result(0,0,1,0) = result3D(0,0,1,0); result(1,0,1,0) = result3D(1,0,1,0);
    result(0,1,1,0) = result3D(0,1,1,0); result(1,1,1,0) = result3D(1,1,1,0);
    result(0,0,0,1) = result3D(0,0,0,1); result(1,0,0,1) = result3D(1,0,0,1);
    result(0,1,0,1) = result3D(0,1,0,1); result(1,1,0,1) = result3D(1,1,0,1);
    result(0,0,1,1) = result3D(0,0,1,1); result(1,0,1,1) = result3D(1,0,1,1);
    result(0,1,1,1) = result3D(0,1,1,1); result(1,1,1,1) = result3D(1,1,1,1);
  }







  //=========================================================================
  //
  //  Nonlinear elasticity Brick
  //
  //=========================================================================

  struct nonlinear_elasticity_brick : public virtual_brick {

    const abstract_hyperelastic_law &AHL;
    
    virtual void asm_real_tangent_terms(const model &md, size_type /* ib */,
                                        const model::varnamelist &vl,
                                        const model::varnamelist &dl,
                                        const model::mimlist &mims,
                                        model::real_matlist &matl,
                                        model::real_veclist &vecl,
                                        model::real_veclist &,
                                        size_type region,
                                        build_version version) const {
      GMM_ASSERT1(mims.size() == 1,
		  "Nonlinear elasticity brick need a single mesh_im");
      GMM_ASSERT1(vl.size() == 1,
		  "Nonlinear elasticity brick need a single variable");
      GMM_ASSERT1(dl.size() == 1,
		  "Wrong number of data for nonlinear elasticity brick, "
                  << dl.size() << " should be 1 (vector).");
      GMM_ASSERT1(matl.size() == 1,  "Wrong number of terms for nonlinear "
		  "elasticity brick");

      const model_real_plain_vector &u = md.real_variable(vl[0]);
      const mesh_fem &mf_u = *(md.pmesh_fem_of_variable(vl[0]));

      const mesh_fem *mf_params = md.pmesh_fem_of_variable(dl[0]);
      const model_real_plain_vector &params = md.real_variable(dl[0]);
      const mesh_im &mim = *mims[0];

      size_type sl = gmm::vect_size(params);
      if (mf_params) sl = sl * mf_params->get_qdim() / mf_params->nb_dof();
      GMM_ASSERT1(sl == AHL.nb_params(), "Wrong number of coefficients for the "
		  "nonlinear constitutive elastic law");

      mesh_region rg(region);
      mf_u.linked_mesh().intersect_with_mpi_region(rg);

      if (version & model::BUILD_MATRIX) {
	gmm::clear(matl[0]);
	GMM_TRACE2("Nonlinear elasticity stiffness matrix assembly");
	asm_nonlinear_elasticity_tangent_matrix
	  (matl[0], mim, mf_u, u, mf_params, params, AHL, rg);
      }


      if (version & model::BUILD_RHS) {
	asm_nonlinear_elasticity_rhs(vecl[0], mim,
				     mf_u, u, mf_params, params, AHL, rg);
	gmm::scale(vecl[0], scalar_type(-1));
      }

    }

    virtual scalar_type asm_real_pseudo_potential(const model &md, size_type,
						  const model::varnamelist &vl,
						  const model::varnamelist &dl,
						  const model::mimlist &mims,
						  model::real_matlist &,
						  model::real_veclist &,
						  model::real_veclist &,
						  size_type region) const {

      const model_real_plain_vector &u = md.real_variable(vl[0]);
      const mesh_fem &mf_u = *(md.pmesh_fem_of_variable(vl[0]));

      const mesh_fem *mf_params = md.pmesh_fem_of_variable(dl[0]);
      const model_real_plain_vector &params = md.real_variable(dl[0]);
      const mesh_im &mim = *mims[0];

      size_type sl = gmm::vect_size(params);
      if (mf_params) sl = sl * mf_params->get_qdim() / mf_params->nb_dof();
      GMM_ASSERT1(sl == AHL.nb_params(), "Wrong number of coefficients for "
		  "the nonlinear constitutive elastic law");

      mesh_region rg(region);
      mf_u.linked_mesh().intersect_with_mpi_region(rg);

      return asm_elastic_strain_energy(mim,mf_u,u,mf_params,params,AHL,rg);
    }

    nonlinear_elasticity_brick(const abstract_hyperelastic_law &AHL_)
      : AHL(AHL_) {
      set_flags("Nonlinear elasticity brick", false /* is linear*/,
                true /* is symmetric */, true /* is coercive */,
		true /* is real */, false /* is complex */);
    }

  };
  
  //=========================================================================
  //  Add a nonlinear elasticity brick.  
  //=========================================================================

  size_type add_nonlinear_elasticity_brick
  (model &md, const mesh_im &mim, const std::string &varname,
   const abstract_hyperelastic_law &AHL, const std::string &dataname,
   size_type region) {
    pbrick pbr = new nonlinear_elasticity_brick(AHL);

    model::termlist tl;
    tl.push_back(model::term_description(varname, varname, true));
    model::varnamelist dl(1, dataname);
    model::varnamelist vl(1, varname);
    return md.add_brick(pbr, vl, dl, tl, model::mimlist(1,&mim), region);
  }

  //=========================================================================
  //  Von Mises or Tresca stress computation.  
  //=========================================================================

  void compute_Von_Mises_or_Tresca(model &md,
				   const std::string &varname, 
				   const abstract_hyperelastic_law &AHL,
				   const std::string &dataname,
				   const mesh_fem &mf_vm,
				   model_real_plain_vector &VM,
				   bool tresca) {
    GMM_ASSERT1(gmm::vect_size(VM) == mf_vm.nb_dof(),
		"The vector has not the good size");
    const mesh_fem &mf_u = md.mesh_fem_of_variable(varname);
    const model_real_plain_vector &u = md.real_variable(varname);
    const mesh_fem *mf_params = md.pmesh_fem_of_variable(dataname);
    const model_real_plain_vector &params = md.real_variable(dataname);
    
    size_type sl = gmm::vect_size(params);
    if (mf_params) sl = sl * mf_params->get_qdim() / mf_params->nb_dof();
    GMM_ASSERT1(sl == AHL.nb_params(), "Wrong number of coefficients for "
		"the nonlinear constitutive elastic law");
    
    unsigned N = unsigned(mf_u.linked_mesh().dim());
    unsigned NP = unsigned(AHL.nb_params()), NFem = mf_u.get_qdim();
    model_real_plain_vector GRAD(mf_vm.nb_dof()*NFem*N);
    model_real_plain_vector PARAMS(mf_vm.nb_dof()*NP);
    if (mf_params) interpolation(*mf_params, mf_vm, params, PARAMS);
    compute_gradient(mf_u, mf_vm, u, GRAD);
    base_matrix E(N, N), gradphi(NFem,N),gradphit(N,NFem), Id(N, N),
      sigmahathat(N,N),aux(NFem,N), sigma(NFem,NFem),
      IdNFem(NFem, NFem);
    base_vector p(NP);
    if (!mf_params) gmm::copy(params, p);
    base_vector eig(NFem);
    base_vector ez(NFem);	// vector normal at deformed surface, (ex X ey)
    double normEz(0);	//norm of ez
    gmm::copy(gmm::identity_matrix(), Id);
    gmm::copy(gmm::identity_matrix(), IdNFem);
    for (size_type i = 0; i < mf_vm.nb_dof(); ++i) {
      gmm::resize(gradphi,NFem,N);
      std::copy(GRAD.begin()+i*NFem*N, GRAD.begin()+(i+1)*NFem*N,
		gradphit.begin());
      gmm::copy(gmm::transposed(gradphit),gradphi);
      for (unsigned int alpha = 0; alpha <N; ++alpha)
	gradphi(alpha, alpha)+=1;
      gmm::mult(gmm::transposed(gradphi), gradphi, E);
      gmm::add(gmm::scaled(Id, -scalar_type(1)), E);
      gmm::scale(E, scalar_type(1)/scalar_type(2));
      if (mf_params)
	gmm::copy(gmm::sub_vector(PARAMS, gmm::sub_interval(i*NP,NP)), p);
      AHL.sigma(E, sigmahathat, p);
      if (NFem == 3 && N == 2) {
	//jyh : compute ez, normal on deformed surface
	for (unsigned int l = 0; l <NFem; ++l)  {
	  ez[l]=0;
	  for (unsigned int m = 0; m <NFem; ++m) 
	    for (unsigned int n = 0; n <NFem; ++n){
	      ez[l]+=levi_civita(l,m,n)*gradphi(m,0)*gradphi(n,1);
	    }
	  normEz= gmm::vect_norm2(ez);
	}
	//jyh : end compute ez
      }
      gmm::mult(gradphi, sigmahathat, aux);
      gmm::mult(aux, gmm::transposed(gradphi), sigma);
      
      /* jyh : complete gradphi for virtual 3rd dim (perpendicular to
	 deformed surface, same thickness) */
      if (NFem == 3 && N == 2) {
	gmm::resize(gradphi,NFem,NFem);
	for (unsigned int ll = 0; ll <NFem; ++ll) 
	  for (unsigned int ii = 0; ii <NFem; ++ii) 
	    for (unsigned int jj = 0; jj <NFem; ++jj) 
	      gradphi(ll,2)+=(levi_civita(ll,ii,jj)*gradphi(ii,0)
			      *gradphi(jj,1))/normEz;
	//jyh : end complete graphi
      }
      
      gmm::scale(sigma, scalar_type(1) / gmm::lu_det(gradphi));
      
      if (!tresca) {
	/* von mises: norm(deviator(sigma)) */
	gmm::add(gmm::scaled(IdNFem, -gmm::mat_trace(sigma) / NFem), sigma);
	
	//jyh : von mises stress=sqrt(3/2)* norm(sigma) ?
	VM[i] = sqrt(3.0/2)*gmm::mat_euclidean_norm(sigma);
      } else {
	/* else compute the tresca criterion */
	//jyh : to be adapted for membrane if necessary
	gmm::symmetric_qr_algorithm(sigma, eig);
	std::sort(eig.begin(), eig.end());
	VM[i] = eig.back() - eig.front();
      }
    }
  }
  

  // ----------------------------------------------------------------------
  //
  // Nonlinear incompressibility brick
  //
  // ----------------------------------------------------------------------

  struct nonlinear_incompressibility_brick : public virtual_brick {
    
    virtual void asm_real_tangent_terms(const model &md, size_type,
					const model::varnamelist &vl,
					const model::varnamelist &dl,
					const model::mimlist &mims,
					model::real_matlist &matl,
					model::real_veclist &vecl,
					model::real_veclist &veclsym,
					size_type region,
					build_version version) const {
      
      GMM_ASSERT1(matl.size() == 2,  "Wrong number of terms for nonlinear "
		  "incompressibility brick");
      GMM_ASSERT1(dl.size() == 0, "Nonlinear incompressibility brick need no "
		  "data");
      GMM_ASSERT1(mims.size() == 1, "Nonlinear incompressibility brick need a "
		  "single mesh_im");
      GMM_ASSERT1(vl.size() == 2, "Wrong number of variables for nonlinear "
		  "incompressibility brick");

      const mesh_fem &mf_u = md.mesh_fem_of_variable(vl[0]);
      const mesh_fem &mf_p = md.mesh_fem_of_variable(vl[1]);
      const model_real_plain_vector &u = md.real_variable(vl[0]);
      const model_real_plain_vector &p = md.real_variable(vl[1]);
      const mesh_im &mim = *mims[0];
      mesh_region rg(region);
      mim.linked_mesh().intersect_with_mpi_region(rg);

      if (version & model::BUILD_MATRIX) {
	gmm::clear(matl[0]);
	gmm::clear(matl[1]);
	asm_nonlinear_incomp_tangent_matrix(matl[0], matl[1],
					    mim, mf_u, mf_p, u, p, rg);
      }

      if (version & model::BUILD_RHS) {
	asm_nonlinear_incomp_rhs(vecl[0], veclsym[1], mim, mf_u, mf_p,u,p, rg);
	gmm::scale(vecl[0], scalar_type(-1));
	gmm::scale(veclsym[1], scalar_type(-1));
      }
    }


    virtual scalar_type asm_real_pseudo_potential(const model &md, size_type,
						  const model::varnamelist &vl,
						  const model::varnamelist &,
						  const model::mimlist &mims,
						  model::real_matlist &,
						  model::real_veclist &,
						  model::real_veclist &,
						  size_type region) const {
      // Corresponds to (1-det(grad(phi))^2 (squared residual with respect
      // to the pressure).
      const mesh_fem &mf_u = md.mesh_fem_of_variable(vl[0]);
      const mesh_fem &mf_p = md.mesh_fem_of_variable(vl[1]);
      const model_real_plain_vector &u = md.real_variable(vl[0]);
      const model_real_plain_vector &p = md.real_variable(vl[1]);
      const mesh_im &mim = *mims[0];
      mesh_region rg(region);
      mim.linked_mesh().intersect_with_mpi_region(rg);

      std::vector<scalar_type> R_U(mf_u.nb_dof()), R_P(mf_p.nb_dof());
      asm_nonlinear_incomp_rhs(R_U, R_P, mim, mf_u, mf_p, u, p, rg);
      return gmm::vect_norm2_sqr(R_P)*scalar_type(1e20); // FIXME : is it ok ?
    }

    nonlinear_incompressibility_brick(void) {
      set_flags("Nonlinear incompressibility brick",
		false /* is linear*/,
		true /* is symmetric */, false /* is coercive */,
		true /* is real */, false /* is complex */);
    }


  };

  size_type add_nonlinear_incompressibility_brick
  (model &md, const mesh_im &mim, const std::string &varname,
   const std::string &multname, size_type region) {
    pbrick pbr = new nonlinear_incompressibility_brick();
    model::termlist tl;
    tl.push_back(model::term_description(varname, varname, true));
    tl.push_back(model::term_description(varname, multname, true));
    model::varnamelist vl(1, varname);
    vl.push_back(multname);
    model::varnamelist dl;
    return md.add_brick(pbr, vl, dl, tl, model::mimlist(1, &mim), region);
  }





}  /* end of namespace getfem.                                             */

