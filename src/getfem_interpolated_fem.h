/* -*- c++ -*- (enables emacs c++ mode)                                    */
/* *********************************************************************** */
/*                                                                         */
/* Library : GEneric Tool for Finite Element Methods (getfem)              */
/* File    : getfem_interpolated_fem.C : definition of a finite element    */
/*           method which interpoles a fem on a different mesh.            */
/*                                                                         */
/* Date : October 29, 2004.                                                */
/* Author : Yves Renard, Yves.Renard@gmm.insa-tlse.fr                      */
/*                                                                         */
/* *********************************************************************** */
/*                                                                         */
/* Copyright (C) 2004  Yves Renard.                                        */
/*                                                                         */
/* This file is a part of GETFEM++                                         */
/*                                                                         */
/* This program is free software; you can redistribute it and/or modify    */
/* it under the terms of the GNU Lesser General Public License as          */
/* published by the Free Software Foundation; version 2.1 of the License.  */
/*                                                                         */
/* This program is distributed in the hope that it will be useful,         */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           */
/* GNU Lesser General Public License for more details.                     */
/*                                                                         */
/* You should have received a copy of the GNU Lesser General Public        */
/* License along with this program; if not, write to the Free Software     */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,  */
/* USA.                                                                    */
/*                                                                         */
/* *********************************************************************** */

#ifndef GETFEM_INTERPOLATED_FEM_H__
#define GETFEM_INTERPOLATED_FEM_H__

#include <getfem_fem.h>
#include <getfem_mesh_fem.h>
#include <bgeot_rtree.h>
#include <bgeot_geotrans_inv.h>

namespace getfem {

  class interpolated_fem : public virtual_fem, public context_dependencies {
    
  protected :

    struct gausspt_interpolation_data {
      size_type elt;        // element of mf1 under this gauss point
      size_type flags;      // flags & 1 : there is an element or not
                            // flags & 2 : base_val is stored
                            // flags & 4 : grad_val is stored
      base_node ptref;      // coords on reference element of mf1 element
      base_tensor base_val; // optional storage of the base values
      base_tensor grad_val; // optional storage of the grad base values
      std::vector<size_type> local_dof; // correspondance between dof of the
                            // mf1 element and dof of the interpolated element.
    };

    struct elt_interpolation_data {
      size_type nb_dof;
      std::vector<gausspt_interpolation_data> gausspt;
      std::vector<size_type> inddof;
    }; 

    const mesh_fem &mf1;    // mf represents the original finite element method
                            // to be interpolated.
    const mesh_fem &mf2;    // mesh on which mf1 is interpolated. contains
                            // also the integration method.
    
    bool store_values;

    // auxiliary variables
    mutable std::vector<elt_interpolation_data> elements;
    mutable bgeot::rtree boxtree; // Tree containing the bounding box
                                  // of mf1 elements
    mutable std::vector<size_type> dofnum, ind_dof;
    mutable size_type cv_stored;
    mutable bgeot::rtree::pbox_set boxlst;
    mutable bgeot::geotrans_inv_convex gic;
    mutable base_tensor taux;
    mutable fem_interpolation_context fictx;
    mutable size_type fictx_cv;
    mutable base_matrix G;
    mutable std::vector<base_node> node_tab_;

    void build_rtree(void) const {
      base_node min, max;
      scalar_type EPS=1E-13;
      boxtree.clear();
      for (dal::bv_visitor cv(mf1.convex_index()); !cv.finished(); ++cv) {
	bounding_box(min, max, mf1.linked_mesh().points_of_convex(cv),
		     mf1.linked_mesh().trans_of_convex(cv));
	for (unsigned k=0; k < min.size(); ++k) { min[k]-=EPS; max[k]+=EPS; }
	boxtree.add_box(min, max, cv);
      }
    }

    bool find_a_point(const base_node &pt, base_node &ptr,
		      size_type &cv) const {
      if (cv_stored != size_type(-1) && gic.invert(pt, ptr))
	{ cv = cv_stored; return true; }
      boxtree.find_boxes_at_point(pt, boxlst);
      bgeot::rtree::pbox_set::const_iterator it = boxlst.begin(),
	ite = boxlst.end();
      for (; it != ite; ++it) {
	gic = bgeot::geotrans_inv_convex
	  (mf1.linked_mesh().convex((*it)->id),
	   mf1.linked_mesh().trans_of_convex((*it)->id));
	cv_stored = (*it)->id;
	if (gic.invert(pt, ptr)) { cv = (*it)->id; return true; }
      }
      return false;
    }

    void build_fem(void) const {
      fictx_cv = cv_stored = size_type(-1);
      dim_ = mf2.linked_mesh().dim();
      if (dim_ == dim_type(-1)) return;
      cout << "dim_ = " << int(dim_) << endl;
      build_rtree();
      elements = std::vector<elt_interpolation_data>(mf2.convex_index().card());
      base_node gpt;
      ind_dof.resize(mf1.nb_dof());
      dofnum.resize(mf1.nb_dof());
      dal::bit_vector alldofs;
      size_type i, max_dof = 0;
      for (dal::bv_visitor cv(mf2.convex_index()); !cv.finished(); ++cv) {
	pintegration_method pim = mf2.int_method_of_element(cv);
	if (pim->type() != IM_APPROX) 
	  DAL_THROW(dal::failure_error, "You have to use approximated "
		    "integration to interpolate an fem");
	papprox_integration pai = pim->approx_method();
	bgeot::pgeometric_trans pgt = mf2.linked_mesh().trans_of_convex(cv);
	elements[cv].gausspt.resize(pai->nb_points());
	dal::bit_vector dofs;
	size_type last_cv = size_type(-1);
	for (size_type k = 0; k < pai->nb_points(); ++k) {
	  gausspt_interpolation_data &gpid = elements[cv].gausspt[k];
	  gpt = pgt->transform(pai->point(k),
			       mf2.linked_mesh().points_of_convex(cv));
	  gpid.flags = find_a_point(gpt, gpid.ptref, gpid.elt) ? 1 : 0;
	  if (gpid.flags && last_cv != gpid.elt) {
	    size_type nbd = mf1.fem_of_element(gpid.elt)->nb_dof(gpid.elt);
	    for (i = 0; i < nbd; ++i)
	      dofs.add(mf1.ind_dof_of_element(gpid.elt)[i]);
	    last_cv = gpid.elt;
	  }
	}
	elements[cv].nb_dof = dofs.card();
	max_dof = std::max(max_dof, dofs.card());
	elements[cv].inddof.resize(dofs.card());
	i = 0;
	for (dal::bv_visitor idof(dofs); !idof.finished(); ++idof, i)
	  { elements[cv].inddof[i] = idof; ind_dof[idof] = i++; }
	for (size_type k = 0; k < pai->nb_points(); ++k) {
	  gausspt_interpolation_data &gpid = elements[cv].gausspt[k];
	  size_type nbd = mf1.fem_of_element(gpid.elt)->nb_dof(gpid.elt);
	  if (gpid.flags) {
	    gpid.local_dof.resize(nbd);
	    for (i = 0; i < nbd; ++i)
	      gpid.local_dof[i] = ind_dof[mf1.ind_dof_of_element(gpid.elt)[i]];
	  }
	}
	alldofs |= dofs;
      }
      i = 0;
      for (dal::bv_visitor idof(alldofs); !idof.finished(); ++idof)
	dofnum[idof] = i++;
      std::fill(ind_dof.begin(), ind_dof.end(), size_type(-1));
      
      base_node P(dim());
      std::fill(P.begin(), P.end(), scalar_type(1)/scalar_type(20));
      node_tab_.resize(max_dof);
      std::fill(node_tab_.begin(), node_tab_.end(), P);
      pspt_valid = false;
      dof_types_.resize(max_dof);
      std::fill(dof_types_.begin(), dof_types_.end(),
		already_numerate_dof(dim()));
    }

  public :

    virtual size_type nb_dof(size_type cv) const
    { if (context_changed()) build_fem(); return elements[cv].nb_dof; }

    virtual size_type index_of_already_numerate_dof(size_type cv, size_type i)
      const { return dofnum[elements[cv].inddof[i]]; }

    virtual bgeot::pconvex_ref ref_convex(size_type cv) const
    { return mf2.fem_of_element(cv)->ref_convex(cv); }

    virtual const bgeot::convex<base_node> &node_convex(size_type cv) const
    { return *(bgeot::generic_dummy_convex_ref(dim(), nb_dof(cv))); }
    virtual bgeot::pstored_point_tab node_tab(size_type) const { 
      if (!pspt_valid)
	{ pspt = bgeot::store_point_tab(node_tab_); pspt_valid = true; }
      return pspt;
    }

    void base_value(const base_node &, base_tensor &) const
    { DAL_THROW(internal_error, "No base values, real only element."); }
    void grad_base_value(const base_node &, base_tensor &) const
    { DAL_THROW(internal_error, "No grad values, real only element."); }
    void hess_base_value(const base_node &, base_tensor &) const
    { DAL_THROW(internal_error, "No hess values, real only element."); }


  protected :

    inline void actualize_fictx(pfem pf, size_type cv,
				const base_node &ptr) const {
      if (fictx_cv != cv) {
	if (pf->need_G()) 
	  bgeot::vectors_to_base_matrix
	    (G, mf1.linked_mesh().points_of_convex(cv));
	fictx = fem_interpolation_context
	  (mf1.linked_mesh().trans_of_convex(cv), pf, base_node(), G, cv);
	fictx_cv = cv;
      }
      fictx.set_xref(ptr);
    }

    mutable bgeot::multi_index mi2, mi3;
    mutable base_node ptref;

  public :

    void real_base_value(const fem_interpolation_context& c, 
			 base_tensor &t) const {
      size_type nbdof = elements[c.convex_num()].nb_dof, cv;
      mi2[1] = target_dim(); mi2[0] = nbdof;
      t.adjust_sizes(mi2);
      if (nbdof == 0) return;
      
      if (c.have_pgp()) { 
	gausspt_interpolation_data &gpid
	  = elements[c.convex_num()].gausspt[c.ii()];
	if (gpid.flags & 1) {
	  cv = gpid.elt;
	  pfem pf = mf1.fem_of_element(cv);
	  if (gpid.flags & 2) { t = gpid.base_val; return; }
	  actualize_fictx(pf, cv, gpid.ptref);
	  pf->real_base_value(fictx, taux);
	  for (size_type i = 0; i < pf->nb_dof(cv); ++i)
	    for (size_type j = 0; j < target_dim(); ++j)
	      t(gpid.local_dof[i],j) = taux(i, j);
	  if (store_values) { gpid.base_val = t; gpid.flags |= 2; }
	  return;
	}
      }
      else {
	if (find_a_point(c.xreal(), ptref, cv)) {
	  pfem pf = mf1.fem_of_element(cv);
	  actualize_fictx(pf, cv, ptref);
	  pf->real_base_value(fictx, taux);
	  for (size_type i = 0; i < nbdof; ++i)
	    ind_dof[elements[cv].inddof[i]] = i;
	  for (size_type i = 0; i < pf->nb_dof(cv); ++i)
	    for (size_type j = 0; j < target_dim(); ++j)
	      if (ind_dof[mf1.ind_dof_of_element(cv)[i]] != size_type(-1))
		t(ind_dof[mf1.ind_dof_of_element(cv)[i]], j) = taux(i, j);
	  for (size_type i = 0; i < nbdof; ++i)
	    ind_dof[elements[cv].inddof[i]] = size_type(-1);
	  return;
	}
      }
      std::fill(t.begin(), t.end(), scalar_type(0));
    }

    void real_grad_base_value(const fem_interpolation_context& c, 
			      base_tensor &t) const {
      size_type nbdof = elements[c.convex_num()].nb_dof, cv;
      mi3[2] = dim(); mi3[1] = target_dim(); mi3[0] = nbdof;
      t.adjust_sizes(mi3);
      if (nbdof == 0) return;
      
      if (c.have_pgp()) { 
	gausspt_interpolation_data &gpid
	  = elements[c.convex_num()].gausspt[c.ii()];
	if (gpid.flags & 1) {
	  cv = gpid.elt;
	  pfem pf = mf1.fem_of_element(cv);
	  if (gpid.flags & 4) { t = gpid.grad_val; return; }
	  actualize_fictx(pf, cv, gpid.ptref);
	  pf->real_grad_base_value(fictx, taux);
	  for (size_type i = 0; i < pf->nb_dof(cv); ++i)
	    for (size_type j = 0; j < target_dim(); ++j)
	      for (size_type k = 0; k < dim(); ++k)
		t(gpid.local_dof[i], j, k) = taux(i, j, k);
	  if (store_values) { gpid.grad_val = t; gpid.flags |= 4; }
	  return;
	}
      }
      else {
	if (find_a_point(c.xreal(), ptref, cv)) {
	  pfem pf = mf1.fem_of_element(cv);
	  actualize_fictx(pf, cv, ptref);
	  fictx.set_xref(ptref);
	  pf->real_grad_base_value(fictx, taux);
	  for (size_type i = 0; i < nbdof; ++i)
	    ind_dof[elements[cv].inddof[i]] = i;
	  for (size_type i = 0; i < pf->nb_dof(cv); ++i)
	    for (size_type j = 0; j < target_dim(); ++j)
		for (size_type k = 0; k < dim(); ++k)
		  if (ind_dof[mf1.ind_dof_of_element(cv)[i]] != size_type(-1))
		  t(ind_dof[mf1.ind_dof_of_element(cv)[i]],j,k) = taux(i,j,k);
	  for (size_type i = 0; i < nbdof; ++i)
	    ind_dof[elements[cv].inddof[i]] = size_type(-1);
	  return;
	}
      }
      std::fill(t.begin(), t.end(), scalar_type(0));
    }

    void real_hess_base_value(const fem_interpolation_context&, 
			      base_tensor &) const
    { DAL_THROW(internal_error, "Sorry, to be done."); }

    
    interpolated_fem(const mesh_fem &mef1, const mesh_fem &mef2,
		     bool store_val = true)
      : mf1(mef1), mf2(mef2), store_values(store_val), mi2(2), mi3(3) {
      this->add_dependency(mef1);
      this->add_dependency(mef2);
      is_pol = is_lag = false; es_degree = 5;
      is_equiv = real_element_defined = true;
      build_fem();
      ntarget_dim = 1; // An extension for vectorial elements should be easy
      // The detection should be done and the multilication of components
      // for scalar elements interpolated.
    }

  };


}  /* end of namespace getfem.                                            */

#endif
