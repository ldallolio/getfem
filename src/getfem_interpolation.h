/* -*- c++ -*- (enables emacs c++ mode)                                    */
/* *********************************************************************** */
/*                                                                         */
/* Library :  GEneric Tool for Finite Element Methods (Getfem)             */
/* File    :  getfem_interpolation.h : interpolation beetween different    */
/*            meshes or at least mesh-fems.                                */
/*     									   */
/* Date : October 15, 2001.                                                */
/* Author : Yves Renard, Yves.Renard@gmm.insa-tlse.fr                      */
/*          Julien Pommier, pommier@gmm.insa-tlse.fr                       */
/* *********************************************************************** */
/*                                                                         */
/* Copyright (C) 2001-2004  Yves Renard, Julien Pommier.                   */
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


#ifndef GETFEM_INTERPOLATION_H__
#define GETFEM_INTERPOLATION_H__

#include <getfem_mesh_fem.h>
#include <bgeot_geotrans_inv.h>
#include <dal_tree_sorted.h>
#include <getfem_mesh_slice.h>

namespace getfem {

  /* ********************************************************************* */
  /*								   	   */
  /*	I. Ventilation of a set of points on a mesh.         		   */
  /*									   */
  /* ********************************************************************* */

  class mesh_trans_inv : public bgeot::geotrans_inv {

  protected :
    typedef gmm::abstract_null_type void_type;
    const getfem_mesh &mesh;
    std::vector<std::map<size_type, void_type> > pts_cvx;
    typedef std::map<size_type, void_type>::const_iterator map_iterator;
    std::vector<base_node> ref_coords;
    std::vector<double> dist;
    std::vector<size_type> cvx_pts;

  public :

    size_type nb_points_on_convex(size_type i) const
    { return pts_cvx[i].size(); }
    void points_on_convex(size_type i, std::vector<size_type> &itab) const;
    const std::vector<base_node> &reference_coords(void) { return ref_coords; }

    /* projection = false : Only the points inside the mesh are ventilated.
     * projection = true  : Try to project the exterior points.
     * TODO : for projection, verify that all the points have been projected
     *        else project them on the frontiere convexes.
     */
    void ventilate(bool projection = false);
    mesh_trans_inv(const getfem_mesh &m):bgeot::geotrans_inv(1E-12),mesh(m) {}
  };
  

  /* ********************************************************************* */
  /*								   	   */
  /*	II. Interpolation.                                  		   */
  /*									   */
  /* ********************************************************************* */


  /**
     interpolation of a solution on same mesh.
     - &mf_target.linked_mesh() == &mf_source.linked_mesh()
     - mf_target must be of lagrange type.
     - mf_target's qdim should be equal to mf_source qdim, or equal to 1
     - U.size() >= mf_source.get_qdim()
     - V.size() >= (mf_target.nb_dof() / mf_target.get_qdim())
                   * mf_source.get_qdim()
  */

  template<typename VECTU, typename VECTV, typename MAT>
    void interpolation_solution_same_mesh(const mesh_fem &mf_source,
					  const mesh_fem &mf_target,
					  const VECTU &U, VECTV &V,
					  MAT &M, int version) {
    base_matrix G;
    size_type qdim = mf_source.get_qdim();
    base_vector coeff, val(qdim);

    if (qdim != mf_target.get_qdim() && mf_target.get_qdim() != 1)
      DAL_THROW(failure_error, "Attempt to interpolate a field of dimension "
		<< qdim << " on a mesh_fem whose Qdim is " << 
		int(mf_target.get_qdim()));
    size_type qmult = mf_source.get_qdim()/mf_target.get_qdim();
    fem_precomp_pool fppool;
    /* we should sort convexes by their fem */
    for (dal::bv_visitor cv(mf_source.convex_index()); !cv.finished(); ++cv) {
      bgeot::pgeometric_trans pgt=mf_source.linked_mesh().trans_of_convex(cv);
      pfem pf_s = mf_source.fem_of_element(cv);
      pfem pf_t = mf_target.fem_of_element(cv);
      size_type nbd_s = pf_s->nb_dof();
      size_type nbd_t = pf_t->nb_dof();
      ref_mesh_dof_ind_ct::iterator itdof;

      if (version == 0) {
	coeff.resize(nbd_s*qdim);
	itdof = mf_source.ind_dof_of_element(cv).begin();
	for (size_type k = 0; k < mf_source.nb_dof_of_element(cv);
	     ++k, ++itdof) coeff[k] = U[*itdof];
      }
      if (pf_s->need_G()) 
	bgeot::vectors_to_base_matrix(G,
			      mf_source.linked_mesh().points_of_convex(cv));

      if (pf_s->target_dim() != 1 || pf_t->target_dim() != 1)
	DAL_THROW(to_be_done_error,
		  "vector FEM interpolation still to be done ... ");
      pfem_precomp pfp = fppool(pf_s, pf_t->node_tab());
      fem_interpolation_context ctx(pgt,pfp,size_type(-1),G,cv);
      itdof = mf_target.ind_dof_of_element(cv).begin();
      for (size_type i = 0; i < nbd_t; ++i, itdof+=mf_target.get_qdim()) {
	size_type dof_t = *itdof*qmult;
	/* faux dans le cas des �l�ments vectoriel.                        */
	ctx.set_ii(i);
	if (version == 0) {
	  pf_s->interpolation(ctx, coeff, val, qdim);
	  for (size_type k=0; k < qdim; ++k) V[dof_t + k] = val[k];
	}
	else {
	  base_matrix Mloc(qdim, mf_source.nb_dof_of_element(cv));
	  pf_s->interpolation(ctx, Mloc, qdim);
	  for (size_type k=0; k < qdim; ++k) {
	    gmm::clear(gmm::mat_row(M, dof_t + k));
	    gmm::copy(gmm::mat_row(M, k),
		      gmm::sub_vector(gmm::mat_row(M, dof_t+k),
		      gmm::sub_index(mf_source.ind_dof_of_element(cv))));
	  }
	}
      }
    }
  }


  /**
     interpolation of a solution on another mesh.
     - mf_target must be of lagrange type.
     - the solution should be continuous..
     - M should be a row major matrix.
   */
  template<typename VECTU, typename VECTV, typename MAT>
    void interpolation_solution(const mesh_fem &mf_source,
				const mesh_fem &mf_target,
				const VECTU &U, VECTV &V, MAT &M,
				int version, bool projection = false) {

    const getfem_mesh &mesh(mf_source.linked_mesh());
    getfem::mesh_trans_inv mti(mesh);
    size_type qdim_s = mf_source.get_qdim(), qdim_t = mf_target.get_qdim();
    size_type qdim_ratio = qdim_s / qdim_t;
    if (qdim_s != qdim_t && qdim_t != 1)
      DAL_THROW(failure_error, "Attempt to interpolate a field of dimension "
		<< qdim_s << " on a mesh_fem whose Qdim is " << qdim_t);

    /* test if the target mesh_fem is really of Lagrange type.         */
    for (dal::bv_visitor cv(mf_target.convex_index()); !cv.finished();++cv) {
      pfem pf_t = mf_target.fem_of_element(cv);
      if (pf_t->target_dim() != 1 || !(pf_t->is_lagrange()))
	DAL_THROW(failure_error,"Target fem not convenient for interpolation");
    }
    /* initialisation of the mesh_trans_inv */
    size_type nbpts = mf_target.nb_dof() / qdim_ratio;
    for (size_type i = 0; i < nbpts; ++i)
      mti.add_point(mf_target.point_of_dof(i * qdim_ratio));
    mti.ventilate(projection);

    std::vector<size_type> itab;    
    base_matrix G;

    /* interpolation */
    dal::bit_vector dof_done; dof_done.add(0, nbpts);
    base_vector val(qdim_s), coeff;
    base_tensor Z;

    for (dal::bv_visitor cv(mesh.convex_index()); !cv.finished(); ++cv) {
      bgeot::pgeometric_trans pgt=mesh.trans_of_convex(cv);
      mti.points_on_convex(cv, itab);
      if (itab.size() == 0) continue;

      // size_type nb=gti.points_in_convex(mesh.convex(cv), pgt, ptab, itab);
      pfem pf_s = mf_source.fem_of_element(cv);
      if (pf_s->need_G()) 
	bgeot::vectors_to_base_matrix(G, mesh.points_of_convex(cv));

      fem_interpolation_context ctx(pgt, pf_s, base_node(), G, cv);
      if (version == 0) {
	coeff.resize(mf_source.nb_dof_of_element(cv));
	gmm::copy(gmm::sub_vector(U,
		  gmm::sub_index(mf_source.ind_dof_of_element(cv))), coeff);
      }
      
      for (size_type i = 0; i < itab.size(); ++i) {
	size_type dof_t = itab[i];
	if (dof_done[dof_t]) {
	  dof_done.sup(dof_t);
	  ctx.set_xref(mti.reference_coords()[dof_t]);
	  size_type pos = dof_t * qdim_ratio;
	  if (version == 0) {
	    pf_s->interpolation(ctx, coeff, val, qdim_s);
	    for (size_type k=0; k < qdim_s; ++k) V[pos + k] = val[k];
	  }
	  else {
	    base_matrix Mloc(qdim_s, mf_source.nb_dof_of_element(cv));
	    pf_s->interpolation(ctx, Mloc, qdim_s);
	    for (size_type k=0; k < qdim_s; ++k) {
	      gmm::clear(gmm::mat_row(M, pos+k));
	      gmm::copy(gmm::mat_row(Mloc, k),
			gmm::sub_vector(gmm::mat_row(M, pos+k),
			gmm::sub_index(mf_source.ind_dof_of_element(cv))));
	    }
	  }
	}
      }
    }
    if (dof_done.card() != 0) {
      cerr << "WARNING : in interpolation_solution (different meshes),"
	   << dof_done.card() << " dof of target mesh_fem have been missed\n";
      for (dal::bv_visitor d(dof_done); !d.finished(); ++d) {
        cerr << "dof_done[" << d << "]=" << mf_target.point_of_dof(d)  << "\n";
      }
    }
  }

  template<typename VECTU, typename VECTV>
  void interpolation_solution(const mesh_fem &mf_source,
			      const mesh_fem &mf_target,
			      const VECTU &U, VECTV &V,
			      bool projection = false) {
    base_matrix M;
    if (mf_source.nb_dof() != gmm::vect_size(U)
	|| (gmm::vect_size(V) % mf_target.nb_dof()) != 0
	|| gmm::vect_size(V) == 0)
      DAL_THROW(dimension_error, "Dimensions mismatch");
    if (&mf_source.linked_mesh() == &mf_target.linked_mesh()) {
      interpolation_solution_same_mesh(mf_source, mf_target, U, V, M, 0);
    }
    else 
      interpolation_solution(mf_source, mf_target, U, V, M, 0, projection);
  }

  template<typename MAT>
  void interpolation_solution(const mesh_fem &mf_source,
			      const mesh_fem &mf_target,
			      MAT &M,
			      bool projection = false) {
    if (mf_source.nb_dof() != gmm::mat_ncols(M)
	|| (gmm::mat_nrows(M) % mf_target.nb_dof()) != 0
	|| gmm::mat_nrows(M) == 0)
      DAL_THROW(dimension_error, "Dimensions mismatch");
    std::vector<scalar_type> U, V;
    if (&mf_source.linked_mesh() == &mf_target.linked_mesh()) {
      interpolation_solution_same_mesh(mf_source, mf_target, U, V, M, 1);
    }
    else 
      interpolation_solution(mf_source, mf_target, U, V, M, 1, projection);
  }



  /** 
      interpolation of a solution on a set of sparse points filled in
      the provided geotrans_inv object.  The gradient is also
      interpolated if PVGRAD is non-null.
   */
  template<class VECT>
  void interpolation_solution(const mesh_fem &mf_source,
			      bgeot::geotrans_inv &gti,
			      const VECT &U, VECT &V, VECT* PVGRAD = 0) {
    size_type mdim = mf_source.linked_mesh().dim();
    size_type qdim = mf_source.get_qdim();
    dal::dynamic_array<base_node> ptab;
    dal::dynamic_array<size_type> itab;
    base_vector coeff;
    base_matrix G;
    
    dal::bit_vector dof_done; dof_done.add(0, gti.nb_points());
    
    for (dal::bv_visitor cv(mf_source.convex_index()); !cv.finished(); ++cv) {
      bgeot::pgeometric_trans pgt=mf_source.linked_mesh().trans_of_convex(cv);
      size_type nb = gti.points_in_convex(mf_source.linked_mesh().convex(cv),
					  pgt, ptab, itab);
      if (nb == 0) continue;
      pfem pf_s = mf_source.fem_of_element(cv);

      if (pf_s->need_G())
	bgeot::vectors_to_base_matrix(G,
			  mf_source.linked_mesh().points_of_convex(cv));
      // cerr << "is_equiv:" << pf_s->is_equivalent() << ",inerp: G="
      //   << G << ",nrow=" << G.nrows() << ", ncols=" << G.ncols() << endl;
      size_type nbd_s = pf_s->nb_dof();
      coeff.resize(nbd_s);

      fem_interpolation_context ctx(pgt,pf_s,base_node(),G,cv);
      gmm::resize(coeff, mf_source.nb_dof_of_element(cv));
      gmm::copy(gmm::sub_vector(U,
		gmm::sub_index(mf_source.ind_dof_of_element(cv))), coeff);

      for (size_type i = 0; i < nb; ++i) {
	size_type dof_t = itab[i];
	if (dof_done[dof_t])
	{ // inverser les deux boucles pour gagner du temps ?
	  // Il faut verifier que le dof est bien de Lagrange ...
	  ctx.set_xref(ptab[i]);
	  // cerr << "cv=" << cv << ", ptab[" << i << "]=" << ptab[i]
	  //      << ", coeff=" << coeff << endl;
	  typename gmm::sub_vector_type<VECT*,
	    gmm::sub_interval>::vector_type dest = 
	    gmm::sub_vector(V,gmm::sub_interval(dof_t*qdim,qdim));
	  pf_s->interpolation(ctx, coeff, dest, qdim);
	  if (PVGRAD) {
	    base_matrix grad(mdim, qdim);
	    pf_s->interpolation_grad(ctx, coeff, grad, qdim);
	    std::copy(grad.begin(), grad.end(), V.begin() + dof_t*qdim*mdim);
	  }
	  dof_done.sup(dof_t, qdim);
	}
      }
    }
    if (dof_done.card() != 0)
      cerr << "WARNING : in interpolation_solution (set of sparse points),"
	   << dof_done.card() << " points have not been done\n";
  }

}  /* end of namespace getfem.                                             */


#endif /* GETFEM_INTERPOLATION_H__  */
