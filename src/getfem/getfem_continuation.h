// -*- c++ -*- (enables emacs c++ mode)
//===========================================================================
//
// Copyright (C) 2011-2011 Tomas Ligursky, Yves Renard
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
// As a special exception, you  may use  this file  as it is a part of a free
// software  library  without  restriction.  Specifically,  if   other  files
// instantiate  templates  or  use macros or inline functions from this file,
// or  you compile this  file  and  link  it  with other files  to produce an
// executable, this file  does  not  by itself cause the resulting executable
// to be covered  by the GNU Lesser General Public License.  This   exception
// does not  however  invalidate  any  other  reasons why the executable file
// might be covered by the GNU Lesser General Public License.
//
//===========================================================================

/** @file getfem_continuation.h
    @author Yves Renard <Yves.Renard@insa-lyon.fr>
    @author Tomas Ligursky <tomas.ligursky@gmail.com>
    @date October 17, 2011.
    @brief (approximate) Moore-Penrose (also called Gauss-Newton) continuation method.
*/
#ifndef GETFEM_CONTINUATION_H__
#define GETFEM_CONTINUATION_H__

#include <getfem/getfem_model_solvers.h>

namespace getfem {


  //=========================================================================
  // Abstract Moore-Penrose continuation method
  //=========================================================================

  
  template <typename S, typename VECT> 
  void compute_gamma_derivative_(const S &s, const VECT &y,
				 double gamma, VECT &d) {
    if (s.has_gamma_derivative()) {
//       s.gamma_derivative(y, gamma, d);
    } else {
      VECT F0(y), F1(y);
      s.F(y, gamma, F0);
      s.F(y, gamma + s.epsilon(), F1);
      s.scaled_add(F1, F0, -1., d);
      s.scale(d, 1./s.epsilon());
    }
  }


  template <typename S, typename VECT> 
  double sp_(const S &s, const VECT &y1, const VECT &y2,
	     double gamma1, double gamma2) {
    double r = s.sp(y1, y2) + gamma1 * gamma2;
    return r;
  }

  template <typename S, typename VECT> 
  double norm_(const S &s, const VECT &y) {
    double no = sqrt(s.sp(y, y));
    return no;
  }


  template <typename S, typename VECT> 
  double norm_(const S &s, const VECT &y, double gamma) {
    double no = sqrt(sp_(s, y, y, gamma, gamma));
    return no;
  }


  template <typename S, typename VECT>
  void compute_tangent(const S &s, const VECT &y, double gamma,
		       VECT &t_y, double &t_gamma) {

    VECT d(y), w(y);

    compute_gamma_derivative_(s, y, gamma, d);
    s.solve_grad(y, gamma, d, w);
    t_gamma = 1. / (t_gamma - s.sp(t_y, w));
    s.scale(w, -t_gamma); s.copy(w, t_y);
    
    double no = norm_(s, t_y, t_gamma);
    s.scale(t_y, 1./no); t_gamma /= no;
  }
  

  template <typename S, typename VECT>
  bool is_tangent(const S &s, const VECT &y, double gamma,
		  const VECT &t_y, double t_gamma) {

    double Gamma, T_gamma=t_gamma, ang;
    VECT Y(y), T_y(t_y);
    
    s.scaled_add(y, T_y, s.h_min(), Y); Gamma = gamma + s.h_min() * T_gamma;
    compute_tangent(s, Y, Gamma, T_y, T_gamma);
    
    ang = sp_(s, t_y, T_y, t_gamma, T_gamma);
    return (ang >= s.minang());
  }


  template <typename S, typename VECT>
  void init_continuation(const S &s, const VECT &y, double gamma,
			 VECT &t_y, double &t_gamma, double &h) {
    s.clear(t_y); t_gamma = (t_gamma >= 0) ? 1. : -1.;
    if (s.noisy() > 1) cout << "computing initial tangent" << endl;
    compute_tangent(s, y, gamma, t_y, t_gamma);
    h = s.h_init();
  }


  template <typename S, typename VECT>
    bool Moore_Penrose_continuation(const S &s, VECT &y, double &gamma,
				    VECT &t_y, double &t_gamma, double &h) {
    
    bool converged, finished=false, new_tangent_found=false;
    unsigned long it, step_dec=0;
    double Delta_Gamma, Gamma, T_gamma, r, no, res, diff, ang;
    VECT F(y), d(y), Delta_Y(y), Y(y), T_y(y), W(y);

    do { // step control

      // prediction
      if (s.noisy() > 0) cout << "prediction with h = " << h << endl;
      s.scaled_add(y, t_y, h, Y); Gamma = gamma + h * t_gamma;
      s.copy(t_y, T_y); T_gamma = t_gamma;
      
      // correction
      it = 0;
      s.F(Y, Gamma, F);
      
      do { // Newton iterations
	if (s.noisy() > 0) cout << "starting correction " << endl;
	compute_gamma_derivative_(s, Y, Gamma, d);
	s.solve_grad(Y, Gamma, F, d, Delta_Y, W);
	r = s.sp(T_y, W);

	Delta_Gamma = s.sp(T_y, Delta_Y) / (r - T_gamma);
	s.scaled_add(Delta_Y, W, -Delta_Gamma, Delta_Y);
	s.scaled_add(Y, Delta_Y, -1., Y); Gamma -= Delta_Gamma;
	
	T_gamma = 1. / (T_gamma - r);
	s.scale(W, -T_gamma); s.copy(W, T_y);
	no = norm_(s, T_y, T_gamma);
	s.scale(T_y, 1./no); T_gamma /= no;

	s.F(Y, Gamma, F); res = norm_(s, F); 
	diff = norm_(s, Delta_Y, Delta_Gamma);
	converged = (res <= s.maxres() && diff <= s.maxdiff());
	it++;

	if (s.noisy() > 0) cout << "iter " << it << " residual " << res
				<< " difference " << diff << endl;

      } while (!converged && it < s.maxit());

      if (converged) {
	ang = sp_(s, t_y, T_y, t_gamma, T_gamma);
	if (s.noisy() > 0) cout << "ang " << ang << endl;
	if (ang >= s.minang()) {
	  finished = true;
	  if (step_dec == 0 && it <= s.thrit()) // elongate the tangent
	    h = (s.h_inc() * h < s.h_max()) ? s.h_inc() * h : s.h_max();
	}
      }
      
      if (!finished) {
	if (h > s.h_min()) { // diminish the step length
	  h = (s.h_dec() * h > s.h_min()) ? s.h_dec() * h : s.h_min();
	  step_dec++;
	}
	else {
	  do { // seek a new tangent
	    s.scaled_add(y, t_y, h, Y); Gamma = gamma + h * t_gamma;
	    compute_tangent(s, Y, Gamma, t_y, t_gamma);

	    if (is_tangent(s, y, gamma, t_y, t_gamma))
	      new_tangent_found = true;
	    else {
	      s.scale(t_y, -1.); t_gamma *= -1.;
	      new_tangent_found = is_tangent(s, y, gamma, t_y, t_gamma);
	    }
	  } while (!new_tangent_found);
	  
	  h = s.h_init(); step_dec = 0;
	}
      }
    } while (!finished);

    s.copy(Y, y); gamma = Gamma;
    s.copy(T_y, t_y); t_gamma = T_gamma;
    return true;
  }



  //=========================================================================
  // Moore-Penrose continuation method for Getfem models
  //=========================================================================


#ifdef GETFEM_MODELS_H__
 
  struct S_getfem_model {

    model &md;  // for real models only
    std::string parameter_name;
    rmodel_plsolver_type lsolver;
    unsigned long maxit_, thrit_;
    double maxres_, maxdiff_, minang_, h_init_, h_max_, h_min_, h_inc_,
      h_dec_, epsilon_;
    int noisy_;

    typedef base_vector VECT;

    S_getfem_model(model &m, std::string pn, rmodel_plsolver_type ls,
		   unsigned long mit = 1000, unsigned long tit = 100,
		   double mres = 1.e-6, double mdiff = 1.e-6,
		   double mang = 0.99, double hin = 1.e-3,
		   double hmax = 1.e-1, double hmin = 1.e-5,
		   double hinc = 2., double hdec = 0.5, double eps = 1.e-9,
		   int noi = 1)
      : md(m), parameter_name(pn), lsolver(ls), maxit_(mit) , thrit_(tit),
	maxres_(mres), maxdiff_(mdiff), minang_(mang), h_init_(hin),
	h_max_(hmax), h_min_(hmin), h_inc_(hinc), h_dec_(hdec),
	epsilon_(eps), noisy_(noi)
    {}
    

    // Linear algebra functions
    void clear (VECT &v) const
    { gmm::clear(v); }
    void copy(const VECT &v1, VECT &v) const
    { gmm::copy(v1, v); }
    void scale (VECT &v, double a) const
    { gmm::scale(v, a); }
    void scaled_add(const VECT &v1, const VECT &v2, double a, VECT &v) const
    { gmm::add(v1, gmm::scaled(v2, a), v); }
    double sp (const VECT &v1, const VECT &v2) const
    { return gmm::vect_sp(v1, v2); }


    // Evaluation of  ...
    void F(const VECT &y, double gamma, VECT &f) const {
      md.set_real_variable(parameter_name)[0] = gamma;
      md.to_variables(y);
      md.assembly(model::BUILD_RHS);
      gmm::copy(gmm::scaled(md.real_rhs(), -1.), f);
    }

    void solve_grad(const VECT &y, double gamma,
		    const VECT &L, VECT &g) const {
      md.set_real_variable(parameter_name)[0] = gamma;
      md.to_variables(y);
      if (noisy_ > 1) cout << "starting computing tangent matrix" << endl;
      md.assembly(model::BUILD_MATRIX);
      
      if (noisy_ > 1) cout << "starting linear solver" << endl;
      gmm::iteration iter(maxres_, noisy_, maxit_);
      (*lsolver)(md.real_tangent_matrix(), g, L, iter);
      if (noisy_ > 1) cout << "linear solver done" << endl;
    }

    void solve_grad(const VECT &y, double gamma, const VECT &L1,
		    const VECT &L2, VECT &g1, VECT &g2) const {
      md.set_real_variable(parameter_name)[0] = gamma;
      md.to_variables(y);
      if (noisy_ > 1) cout << "starting computing tangent matrix" << endl;
      md.assembly(model::BUILD_MATRIX);

      gmm::iteration iter(maxres_, noisy_, maxit_);
      (*lsolver)(md.real_tangent_matrix(), g1, L1, iter);
      (*lsolver)(md.real_tangent_matrix(), g2, L2, iter);
    }

//     void gamma_derivative(const VECT &y, double gamma, VECT &d) const {}

    
    // Misc.

    bool has_gamma_derivative(void) const { return false; }
    unsigned long thrit(void) const { return thrit_; }
    unsigned long maxit(void) const { return maxit_; }
    double epsilon(void) const { return epsilon_; }
    double minang(void) const { return minang_; }
    double maxres(void) const { return maxres_; }
    double maxdiff(void) const { return maxdiff_; }
    double h_init(void) const { return h_init_; }
    double h_min(void) const { return h_min_; }
    double h_max(void) const { return h_max_; }
    double h_dec(void) const { return h_dec_; }
    double h_inc(void) const { return h_inc_; }
    int noisy(void) const { return noisy_; }

  };

#endif


}  /* end of namespace getfem.                                             */


#endif /* GETFEM_CONTINUATION_H__ */