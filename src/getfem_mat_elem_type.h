/* *********************************************************************** */
/*                                                                         */
/* Library :  GEneric Tool for Finite Element Methods (getfem)             */
/* File    :  getfem_mat_elem_type.h : types of elementary matrices for    */
/*            regular scalar finite element.                               */
/*     									   */
/*                                                                         */
/* Date : December 21, 2000.                                               */
/* Author : Yves Renard, Yves.Renard@gmm.insa-tlse.fr                      */
/*                                                                         */
/* *********************************************************************** */
/*                                                                         */
/* Copyright (C) 2000-2002  Yves Renard.                                   */
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


#ifndef GETFEM_MAT_ELEM_TYPE_H__
#define GETFEM_MAT_ELEM_TYPE_H__

#include <getfem_config.h>
#include <getfem_integration.h>
#include <getfem_fem.h>

namespace getfem
{

  enum constituant_type
    { GETFEM_BASE_, GETFEM_GRAD_, GETFEM_HESSIAN_, GETFEM_NONLINEAR_ };

  class nonlinear_elem_term {
    public :
      virtual dim_type dim(void) const = 0;
      virtual dim_type size(dim_type i)  const = 0;
      virtual void compute(fem_interpolation_context& ctx,
			   base_tensor &t)  const = 0;
      virtual size_type nb_pfi(void) const { return 0; };
      virtual pfem pfi(size_type i) const { return 0; };
  };

  struct constituant {
    constituant_type t;
    pfem pfi;
    const nonlinear_elem_term *nlt;
  };

  /** Description of an elementary matrix.  This class 
   *       is not to be manipulate by itself. Use pmat\_elem\_type and
   *       the functions written to produce those descriptions.
   */ 
  struct mat_elem_type : public std::vector<constituant>
  {
    bgeot::multi_index mi;
  };

   /** @name functions on elementary matrix descriptions
   */
  //@{

  typedef const mat_elem_type * pmat_elem_type;
  
  /** Gives a pointer to the structure describing the elementary matrix
   *   which compute the integral of the basic functions described by pfi.
   *    pfi is of type bgeot::pfem\_interpolation.
   */
  pmat_elem_type mat_elem_base(pfem pfi);
  /** Gives a pointer to the structure describing the elementary matrix
   *   which compute the integral of the gradient of the basic functions
   *    described by pfi. pfi is of type bgeot::pfem\_interpolation.
   */
  pmat_elem_type mat_elem_grad(pfem pfi);
  /** Gives a pointer to the structure describing the elementary matrix
   *   which compute the integral of the hessian of the basic functions
   *    described by pfi. pfi is of type bgeot::pfem\_interpolation. 
   */
  pmat_elem_type mat_elem_hessian(pfem pfi);
  /** Gives a pointer to the structure describing the elementary matrix
   *   which compute the integral of a nonlinear term.
   */
  pmat_elem_type mat_elem_nonlinear(const nonlinear_elem_term &);
  /** Gives a pointer to the structure describing the elementary matrix
   *   which compute the integral of product of the integrals described by
   *   *pet1 and *pet2.
   */
  pmat_elem_type mat_elem_product(pmat_elem_type a, pmat_elem_type b);
  
   //@}

}  /* end of namespace getfem.                                             */


#endif /* GETFEM_PRE_INTERPOLATION_H__                                     */
