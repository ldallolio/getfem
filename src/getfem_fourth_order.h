// -*- c++ -*- (enables emacs c++ mode)
//========================================================================
//
// Library : GEneric Tool for Finite Element Methods (getfem)
// File    : getfem_orderfourspdes.h : assembly procedures and bricks 
//                                     for fourth order pdes.
// Date    : January 6, 2006.
// Authors : Yves Renard <Yves.Renard@insa-toulouse.fr>
//           Julien Pommier <Julien.Pommier@insa-toulouse.fr>
//
//========================================================================
//
// Copyright (C) 2006-2006 Yves Renard
//
// This file is a part of GETFEM++
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//========================================================================

/**@file getfem_orderfourpdes.h
   @brief assembly procedures and bricks for fourth order pdes.
*/
#ifndef GETFEM_FOURTH_ORDER_H_
#define GETFEM_FOURTH_ORDER_H__

#include <getfem_modeling.h>
#include <getfem_assembling_tensors.h>

namespace getfem {
  
  /**
     assembly of @f$\int_\Omega \Delta u \Delta v@f$.
     @ingroup asm
  */
  template<typename MAT, typename VECT>
  void asm_stiffness_matrix_for_bilaplacian
  (const MAT &M, const mesh_im &mim, const mesh_fem &mf,
   const mesh_fem &mf_data, const VECT &A,
   const mesh_region &rg = mesh_region::all_convexes()) {
    generic_assembly assem
      ("a=data$1(#2);"
       "M(#1,#1)+=sym(comp(Hess(#1).Hess(#1).Base(#2))(:,i,i,:,j,j,k).a(k))");
    assem.push_mi(mim);
    assem.push_mf(mf);
    assem.push_mf(mf_data);
    assem.push_data(A);
    assem.push_mat(const_cast<MAT &>(M));
    assem.assembly(rg);
  }  

  /* ******************************************************************** */
  /*		Bilaplacian brick.                                        */
  /* ******************************************************************** */

# define MDBRICK_BILAPLACIAN 783465
  
  /** Bilaplacian brick @f$ \Delta \Delta u @f$.

  @see asm_stiffness_matrix_for_bilaplacian
  @ingroup bricks 
  */
  template<typename MODEL_STATE = standard_model_state>
  class mdbrick_bilaplacian
    : public mdbrick_abstract_linear_pde<MODEL_STATE> {
    
    TYPEDEF_MODEL_STATE_TYPES;

    mdbrick_parameter<VECTOR> coeff_;  /* coeff_ a scalar field */

    void proper_update_K(void) {
      asm_stiffness_matrix_for_bilaplacian
	(this->K, this->mim, this->mf_u, coeff().mf(),  coeff().get(),
	 this->mf_u.linked_mesh().get_mpi_region());
    }
  public :

    /** accessor to the coefficient k */
    mdbrick_parameter<VECTOR> &coeff() { return coeff_; }
    const mdbrick_parameter<VECTOR> &coeff() const { return  coeff_; }

    /** Constructor, the default coeff is a scalar equal to one
	(i.e. it gives the Laplace operator).

        The coeff can be later changed.

	@param mim the integration method that is used. 
	@param mf_u the mesh_fem for the unknown u.
	@param k the scalar value of the coefficient.
    */
    mdbrick_bilaplacian(const mesh_im &mim_,
			const mesh_fem &mf_u_, value_type k = 1.)
      : mdbrick_abstract_linear_pde<MODEL_STATE>(mim_, mf_u_,
						 MDBRICK_BILAPLACIAN),
	coeff_("coeff", mf_u_.linked_mesh(), this) { coeff_.set(k); }
  };


  /* ******************************************************************** */
  /*		Normale derivative source term brick.                     */
  /* ******************************************************************** */

  /**
     assembly of @f$\int_\Gamma{\partial_n u f}@f$.
     @ingroup asm
  */
  template<typename VECT1, typename VECT2>
  void asm_normal_derivative_source_term
  (VECT1 &B, const mesh_im &mim, const mesh_fem &mf, const mesh_fem &mf_data,
   const VECT2 &F, const mesh_region &rg) {
    if (mf_data.get_qdim() != 1)
      DAL_THROW(invalid_argument, "invalid data mesh fem (Qdim=1 required)");

    const char *s;
    if (mf.get_qdim() == 1)
      s = "F=data(#2);"
	"V(#1)+=comp(Grad(#1).Normal().Base(#2))(:,i,i,j).F(j);";
    else
      s = "F=data(qdim(#1),#2);"
	"V(#1)+=comp(vGrad(#1).Normal.Base(#2))(:,i,k,k,j).F(i,j);";
    asm_real_or_complex_1_param(B, mim, mf, mf_data, F, rg, s);
  }

  /**
     Normal derivative source term brick ( @f$ F = \int b.\partial_n v @f$ ).
     
     Update the right hand side of the linear system.

     @see asm_source_term
     @ingroup bricks
  */
  template<typename MODEL_STATE = standard_model_state>
  class mdbrick_normal_derivative_source_term
    : public mdbrick_abstract<MODEL_STATE>  {

    TYPEDEF_MODEL_STATE_TYPES;

    mdbrick_parameter<VECTOR> B_;
    VECTOR F_;
    bool F_uptodate;
    size_type boundary, num_fem, i1, nbd;

    void proper_update(void) {
      const mesh_fem &mf_u = this->get_mesh_fem(num_fem);
      i1 = this->mesh_fem_positions[num_fem];
      nbd = mf_u.nb_dof();

      gmm::resize(F_, mf_u.nb_dof());
      gmm::clear(F_);
      F_uptodate = false;
    }

  public :

    mdbrick_parameter<VECTOR> &source_term(void)
    { B_.reshape(this->get_mesh_fem(num_fem).get_qdim()); return B_;  }

    const mdbrick_parameter<VECTOR> &source_term(void) const { return B_; }

    /// gives the right hand side of the linear system.
    const VECTOR &get_F(void) { 
      this->context_check();
      if (!F_uptodate || this->parameters_is_any_modified()) {
	const mesh_fem &mf_u = *(this->mesh_fems[num_fem]);
	F_uptodate = true;
	DAL_TRACE2("Assembling a source term");
	asm_normal_derivative_source_term
	  (F_, *(this->mesh_ims[0]), mf_u, B_.mf(), B_.get(),
	   mf_u.linked_mesh().get_mpi_sub_region(boundary));
	this->parameters_set_uptodate();
      }
      return F_;
    }

    virtual void do_compute_tangent_matrix(MODEL_STATE &, size_type,
					   size_type) { }
    virtual void do_compute_residual(MODEL_STATE &MS, size_type i0,
				   size_type) {
      gmm::add(gmm::scaled(get_F(), value_type(-1)),
	       gmm::sub_vector(MS.residual(), gmm::sub_interval(i0+i1, nbd)));
    }

    /** Constructor defining the rhs
	@param problem the sub-problem to which this brick applies.
	@param mf_data_ the mesh_fem on which B_ is defined.
	@param B_ the value of the source term.
	@param bound the mesh boundary number on which the source term
	is applied.
	@param num_fem_ the mesh_fem number on which this brick is is applied.
    */
    mdbrick_normal_derivative_source_term
    (mdbrick_abstract<MODEL_STATE> &problem, const mesh_fem &mf_data_,
     const VECTOR &B__, size_type bound,
     size_type num_fem_=0) : B_("source_term",mf_data_, this), boundary(bound),
	num_fem(num_fem_) {
      this->add_sub_brick(problem);
      if (bound != size_type(-1))
	this->add_proper_boundary_info(num_fem, bound,
				       MDBRICK_NORMAL_DERIVATIVE_NEUMANN);
      this->force_update();
      B_.reshape(this->get_mesh_fem(num_fem).get_qdim());
      if (gmm::vect_size(B__)) B_.set(B__);
    }
  };

  /* ******************************************************************** */
  /*		Normale derivative Dirichlet condition bricks.            */
  /* ******************************************************************** */

  /**
     Assembly of normal derivative Dirichlet constraints
     @f$ \partial_n u(x) = r(x) @f$ in a weak form
     @f[ \int_{\Gamma} \partial_n u(x)v(x)=\int_{\Gamma} r(x)v(x) \forall v@f],
     where @f$ v @f$ is in
     the space of multipliers corresponding to mf_mult.

     size(r_data) = Q   * nb_dof(mf_rh);

     version = |ASMDIR_BUILDH : build H
     |ASMDIR_BUILDR : build R
     |ASMDIR_BUILDALL : do everything.

     @ingroup asm
  */

  template<typename MAT, typename VECT1, typename VECT2>
  void asm_normal_derivative_dirichlet_constraints
  (MAT &H, VECT1 &R, const mesh_im &mim, const mesh_fem &mf_u,
   const mesh_fem &mf_mult, const mesh_fem &mf_r,
   const VECT2 &r_data, const mesh_region &rg,
   int version =  ASMDIR_BUILDALL) {
    typedef typename gmm::linalg_traits<VECT1>::value_type value_type;
    typedef typename gmm::number_traits<value_type>::magnitude_type magn_type;
    
    rg.from_mesh(mim.linked_mesh()).error_if_not_faces();
    if (mf_r.get_qdim() != 1)
      DAL_THROW(invalid_argument, "invalid data mesh fem (Qdim=1 required)");
    if (version & ASMDIR_BUILDH) {
      const char *s;
      if (mf_u.get_qdim() == 1 && mf_mult.get_qdim() == 1)
	s = "M(#1,#2)+=comp(Base(#1).Grad(#2).Normal())(:,:,i,i)";
      else
	s = "M(#1,#2)+=comp(vBase(#1).vGrad(#2).Normal())(:,i,:,i,j,j);";
      
      generic_assembly assem(s);
      assem.push_mi(mim);
      assem.push_mf(mf_mult);
      assem.push_mf(mf_u);
      assem.push_mat(H);
      assem.assembly(rg);
      gmm::clean(H, gmm::default_tol(magn_type())
		 * gmm::mat_maxnorm(H) * magn_type(1000));
    }
    if (version & ASMDIR_BUILDR)
      asm_source_term(R, mim, mf_mult, mf_r, r_data, rg);
  }

  /** Normal derivative Dirichlet condition brick.
   *
   *  This brick represent a Dirichlet condition on the normal derivative
   *  of the unknow for fourth order pdes.
   *  The general form is
   *  @f[ \int \partial_n u(x)v(x) = \int r(x)v(x) \forall v@f]
   *  where @f$ r(x) @f$ is
   *  the right hand side for the Dirichlet condition (0 for
   *  homogeneous conditions) and @f$ v @f$ is in a space of multipliers
   *  defined by the trace of mf_mult on the considered part of boundary.
   *
   *  @see asm_normal_derivative_dirichlet_constraints
   *  @see mdbrick_constraint
   *  @ingroup bricks
   */
  template<typename MODEL_STATE = standard_model_state>
  class mdbrick_normal_derivative_Dirichlet
    : public mdbrick_constraint<MODEL_STATE>  {
    
    TYPEDEF_MODEL_STATE_TYPES;

    mdbrick_parameter<VECTOR> R_;
    
    size_type boundary;
    bool mfdata_set, B_to_be_computed;
    gmm::sub_index SUB_CT;
    const mesh_fem *mf_mult;
    
    const mesh_fem &mf_u() { return *(this->mesh_fems[this->num_fem]); }
    const mesh_im  &mim() { return *(this->mesh_ims[0]); }

    void compute_constraints(unsigned version) {
      size_type ndu = mf_u().nb_dof(), ndm = mf_mult->nb_dof();
      gmm::row_matrix<gmm::rsvector<value_type> > M(ndm, ndu);
      VECTOR V(ndm);
      DAL_TRACE2("Assembling normal derivative Dirichlet constraints, version "
		 << version);
      asm_normal_derivative_dirichlet_constraints
	(M, V, mim(), mf_u(), *mf_mult, rhs().mf(), R_.get(),
	 mf_u().linked_mesh().get_mpi_sub_region(boundary), version);    
      if (version & ASMDIR_BUILDH)
	gmm::copy(gmm::sub_matrix(M, SUB_CT, gmm::sub_interval(0, ndu)), 
		  this->B);
      gmm::copy(gmm::sub_vector(V, SUB_CT), this->CRHS);
    }

    virtual void recompute_B_sizes(void) {
      if (!mfdata_set) {
	rhs().set(classical_mesh_fem(mf_u().linked_mesh(), 0), 0);
 	mfdata_set = true;
      }
      size_type nd = mf_u().nb_dof();
      dal::bit_vector dof_on_bound = mf_mult->dof_on_set(boundary);
      size_type nb_const = dof_on_bound.card();
      std::vector<size_type> ind_ct;
      for (dal::bv_visitor i(dof_on_bound); !i.finished(); ++i)
	ind_ct.push_back(i);
      SUB_CT = gmm::sub_index(ind_ct);
      gmm::resize(this->B, nb_const, nd);
      gmm::resize(this->CRHS, nb_const);
      B_to_be_computed = true;
    }

    virtual void recompute_B(void) {
      unsigned version = 0;
      if (R_.is_modified()) { version = ASMDIR_BUILDR; }
      if (B_to_be_computed) { version = ASMDIR_BUILDR | ASMDIR_BUILDH; }
      if (version) { 
	compute_constraints(version);
	this->parameters_set_uptodate();
	B_to_be_computed = false;
      }
    }

  public :

    /** Change the @f$ r(x) @f$ right hand side.
     *	@param R a vector of size @c Q*mf_data.nb_dof() .
     */
    mdbrick_parameter<VECTOR> &rhs()
    { R_.reshape(mf_u().get_qdim()); return R_; }

    /** Constructor which does not define the rhs (i.e. which sets an
     *	homogeneous Dirichlet condition)
     *	@param problem the sub problem to which this brick is applied.
     *	@param bound the boundary number for the dirichlet condition.
     *  @param mf_mult_ the mesh_fem for the multipliers.
     *	@param num_fem_ the mesh_fem number on which this brick is is applied.
     */
    mdbrick_normal_derivative_Dirichlet
    (mdbrick_abstract<MODEL_STATE> &problem, size_type bound,
     const mesh_fem &mf_mult_ = dummy_mesh_fem(), size_type num_fem_=0)
      : mdbrick_constraint<MODEL_STATE>(problem, num_fem_), R_("R", this),
	boundary(bound) {
      mf_mult = (&mf_mult_ == &dummy_mesh_fem()) ? &(mf_u()) : &mf_mult_;
      this->add_proper_boundary_info(this->num_fem, boundary, 
				     MDBRICK_NORMAL_DERIVATIVE_DIRICHLET);
      this->add_dependency(*mf_mult);
      mfdata_set = false; B_to_be_computed = true;
      this->force_update();
    }
  };




}  /* end of namespace getfem.                                             */


#endif /* GETFEM_FOURTH_ORDER_H__ */
