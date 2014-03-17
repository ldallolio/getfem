/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2012-2012 Liang Jin Lim
 
 This file is a part of GETFEM++
 
 Getfem++  is  free software;  you  can  redistribute  it  and/or modify it
 under  the  terms  of the  GNU  Lesser General Public License as published
 by  the  Free Software Foundation;  either version 3 of the License,  or
 (at your option) any later version along with the GCC Runtime Library
 Exception either version 3.1 or (at your option) any later version.
 This program  is  distributed  in  the  hope  that it will be useful,  but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or  FITNESS  FOR  A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 License and GCC Runtime Library Exception for more details.
 You  should  have received a copy of the GNU Lesser General Public License
 along  with  this program;  if not, write to the Free Software Foundation,
 Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA.
 
 As a special exception, you  may use  this file  as it is a part of a free
 software  library  without  restriction.  Specifically,  if   other  files
 instantiate  templates  or  use macros or inline functions from this file,
 or  you compile this  file  and  link  it  with other files  to produce an
 executable, this file  does  not  by itself cause the resulting executable
 to be covered  by the GNU Lesser General Public License.  This   exception
 does not  however  invalidate  any  other  reasons why the executable file
 might be covered by the GNU Lesser General Public License.
 
===========================================================================*/
/**
@file getfem_im_data.h
@brief Provides indexing of integration points for mesh_im.
@date Feb 2014
@author Liang Jin Lim
*/

#pragma once

#ifndef GETFEM_IM_DATA_H__
#define GETFEM_IM_DATA_H__

#include <getfem/getfem_mesh_im.h>

namespace getfem{
  using bgeot::size_type;
  using bgeot::scalar_type;

  /** im_data provides indexing to the integration points of a mesh
  im object. The im_data data contains a reference of mesh_im object
  . The index can be filtered by region, and each im_data has its 
  own tensorial size.

  Filtered methods will provide filtered index on the region.
  This class also provides reading and writing tensor( including
  matrix, vector and scalar) from a vector data (generally a
  fixed-size variable from the model.)
  */
  class im_data : public context_dependencies {
  public:
    /**
    * Constructor
    * @param meshIm Reference mesh_im object
    * @param tensorSize tensor dimension of each integration points
    * @param filteredRegion index not in the region will be filtered
    *        out.
    */
    im_data(const mesh_im& meshIm, bgeot::multi_index tensor_size,
      size_type filtered_region = size_type(-1));

    /**
    * Constructor. The tensor size by default is a scalar value.
    * @param meshIm Reference mesh_im object
    * @param filteredRegion index not in the region will be filtered
    *        out.
    */
    im_data(const mesh_im& mesh_im, size_type filtered_region = size_type(-1));

    /**set filtered region id*/
    void set_region(size_type region);

    /**return filtered region id*/
    inline size_type filtered_region() const {return filtered_region_;}

    /**Returns the index of an integration point with no filtering*/
    size_type index_of_point(size_type cv, size_type i) const;

    /**Returns the index of an integration point with filtering*/
    size_type filtered_index_of_point(size_type cv, size_type i) const;

    /**Total numbers of index (integration points)*/
    size_type nb_index() const;

    /**Total numbers of filtered index (integration points)*/
    size_type nb_filtered_index() const;

    /**Number of points in element cv*/
    size_type nb_points_of_element(size_type cv) const;

    /**sum of tensor elements, M(3,3) will have 3*3=9 elements*/
    size_type nb_tensor_elem() const;

    /**List of convex in filtered region*/
    dal::bit_vector filtered_convex_index() const;

    /**List of index of first point in filtered region*/
    std::vector<size_type> filtered_index_of_first_point () const;

    /**called automatically when there is a change in dependencies*/
    void update_from_context () const;

    /**get mesh im*/
    inline const mesh_im &get_mesh_im() const {return im_;}

    /**implicit conversion to mesh im*/
    inline operator const mesh_im &() const {return im_;}

    inline const bgeot::multi_index& tensor_size () const { return tensor_size_;}

    void set_tensor_size (const bgeot::multi_index& tensor_size);

    inline gmm::uint64_type version_number(void) const { context_check(); return v_num_; }

    /**Extend a vector from filtered size to full size and copy the data to correct index*/
    template <typename VECT>
    void extend_vector(const VECT &V1, VECT &V2) const{
      if(V1.size() == 0 && V2.size() == 0) return;

      size_type nb_data = V1.size()/nb_filtered_index();
      GMM_ASSERT1(nb_data == nb_tensor_elem_, "Invalid tensorial size for vector V1");
      GMM_ASSERT1(V1.size()%nb_filtered_index() == 0, "Invalid size of vector V1");
      GMM_ASSERT1(V2.size() == nb_data*nb_index(), "Invalid size of vector V2");

      for(dal::bv_visitor cv(filtered_convex_index_); !cv.finished(); ++cv)
      {
        size_type iPt         = index_of_point(cv,0);
        size_type iFilteredPt = filtered_index_of_point(cv,0);
        for(size_type i = 0; i <nb_points_of_element(cv); ++i)
        {
          for(size_type i_comp = 0; i_comp < nb_data; ++i_comp)
            V2[(iPt+i)*nb_data + i_comp] = V1[(iFilteredPt+i)*nb_data + i_comp];
        }
      }
    }

    /**Filter a vector from full size to filtered size and copy the data to correct index*/    
    template <typename VECT>
    void reduce_vector(const VECT &V1, VECT &V2) const{
      if(V1.size() == 0 && V2.size() == 0) return;

      size_type nb_data = V1.size()/nb_index_;
      GMM_ASSERT1(nb_data == nb_tensor_elem_, "Invalid tensorial size for vector V1");
      GMM_ASSERT1(V1.size()%nb_index_ == 0, "Invalid size of vector V1");
      GMM_ASSERT1(V2.size() == nb_data*nb_filtered_index(), 
        "Invalid size of vector V2");

      for(dal::bv_visitor cv(filtered_convex_index_); !cv.finished(); ++cv)
      {
        size_type iPt         = index_of_point(cv,0);
        size_type iFilteredPt = filtered_index_of_point(cv,0);
        for(size_type i = 0; i <nb_points_of_element(cv); ++i)
        {
          for(size_type i_comp = 0; i_comp < nb_data; ++i_comp)
            V2[(iFilteredPt+i)*nb_data + i_comp] = V1[(iPt+i)*nb_data + i_comp];
        }
      }
    }

    /**get a scalar value of a integration point 
    from a raw vector data, described by the tensor size.*/
    template <typename VECT>
    typename VECT::value_type get_value (const VECT &V1, 
      size_type cv, size_type i, bool filter = true) const{
      size_type nb_data = V1.size()/nb_filtered_index();
      GMM_ASSERT1(nb_data == nb_tensor_elem_, "Invalid tensorial size for vector V1");
      GMM_ASSERT1(nb_tensor_elem_ == 1, "ImData is not scalar type");
      if(!filter) return V1[index_of_point(cv,i)];
      else return V1[filtered_index_of_point(cv,i)];
    }

    /**get a vector of a integration point 
    from a raw vector data, described by the tensor size.*/
    template <typename VECT1, typename VECT2>
    void get_vector (const VECT1 &V1, size_type cv, size_type i, 
      VECT2& V2, bool filter = true) const{
      if(V1.size() == 0 && V2.size() == 0) return;
      size_type nb_data = V1.size()/nb_filtered_index();
      GMM_ASSERT1(nb_data == nb_tensor_elem_,    "Invalid tensorial size for vector V1");
      GMM_ASSERT1
        ((tensor_size_.size() == 2 
           && ((tensor_size_[0] == V2.size() && tensor_size_[1] == 1)
                || tensor_size_[1] == V2.size() && tensor_size_[0] == 1))
        || (tensor_size_.size() == 1 && tensor_size_[0] == V2.size())
        , "V2 size is incompatible");

      size_type   iPt = 0;
      if(!filter) iPt = index_of_point(cv,i);
      else        iPt = filtered_index_of_point(cv,i);

      for(size_type i_comp = 0; i_comp < nb_tensor_elem_; ++i_comp)
        V2[i_comp] = V1[iPt*nb_tensor_elem_ +i_comp];

    }

    /**get a matrix of a integration point 
    from a raw vector data, described by the tensor size.*/
    template <typename VECT, typename MAT>
    void get_matrix (const VECT &V1, 
      size_type cv, size_type i, 
      MAT& M, bool filter = true) const{
      if(V1.size() == 0 && M.size() == 0) return;
      size_type nb_data = V1.size()/((filter)?nb_filtered_index():nb_index());
      GMM_ASSERT1(nb_data == nb_tensor_elem_, "Invalid tensorial size for vector V1");
      GMM_ASSERT1(tensor_size_.size() == 2, "ImData is not matrix type");
      GMM_ASSERT1(tensor_size_[0] == M.nrows() 
        && tensor_size_[1] == M.ncols(), "M size is incompatible");

      size_type   iPt = 0;
      if(!filter) iPt = index_of_point(cv,i);
      else        iPt = filtered_index_of_point(cv,i);

      for(size_type i_comp = 0; i_comp < nb_tensor_elem_; ++i_comp)
        M.as_vector()[i_comp] = V1[iPt*nb_tensor_elem_ +i_comp];
    }

    /**get a tensor of a integration point 
    from a raw vector data, described by the tensor size.*/
    template <typename VECT, typename TENSOR>
    void get_tensor (const VECT &V1, 
      size_type cv, size_type i, 
      TENSOR& T, bool filter = true) const{
      if(V1.size() == 0 && T.size() == 0) return;
      size_type nb_data = V1.size()/((filter)?nb_filtered_index():nb_index());
      GMM_ASSERT1(nb_data == nb_tensor_elem_, "Invalid tensorial size for vector V1");
      GMM_ASSERT1(tensor_size_ == T.sizes(), "ImData tensor is incompatible with T");

      size_type   iPt = 0;
      if(!filter) iPt = index_of_point(cv,i);
      else        iPt = filtered_index_of_point(cv,i);

      for(size_type i_comp = 0; i_comp < nb_tensor_elem_; ++i_comp)
        T.as_vector()[i_comp] = V1[iPt*nb_tensor_elem_ +i_comp];
    }

    /**set a value of a integration point 
    from a raw vector data, described by the tensor size.*/
    template <typename VECT>
    typename VECT::value_type &set_value (VECT &V1, 
      size_type cv, size_type i, bool filter = true) const{
      size_type nb_data = V1.size()/((filter)?nb_filtered_index():nb_index());
      GMM_ASSERT1(nb_data == nb_tensor_elem_, "Invalid tensorial size for vector V1");
      GMM_ASSERT1(nb_tensor_elem_ == 1, "ImData is not scalar type");
      if(!filter) return V1[index_of_point(cv,i)];
      else return V1[filtered_index_of_point(cv,i)];
    }

    /**set a vector of a integration point 
    from a raw vector data, described by the tensor size.*/
    template <typename VECT1, typename VECT2>
    void set_vector (VECT1 &V1, 
      size_type cv, size_type i, 
      const VECT2& V2, bool filter = true) const{
      if(V1.size() == 0 && V2.size() == 0) return;
      size_type nb_data = V1.size()/((filter)?nb_filtered_index():nb_index());
      GMM_ASSERT1(nb_data == nb_tensor_elem_,    "Invalid tensorial size for vector V1");
      GMM_ASSERT1
        ((tensor_size_.size() == 2 
           && ((tensor_size_[0] == V2.size() && tensor_size_[1] == 1)
                || tensor_size_[1] == V2.size() && tensor_size_[0] == 1))
        || (tensor_size_.size() == 1 && tensor_size_[0] == V2.size())
        , "V2 size is incompatible");

      size_type   iPt = 0;
      if(!filter) iPt = index_of_point(cv,i);
      else        iPt = filtered_index_of_point(cv,i);

      for(size_type i_comp = 0; i_comp < nb_tensor_elem_; ++i_comp)
        V1[iPt*nb_tensor_elem_ +i_comp] = V2[i_comp];
    }

    /**set a matrix of a integration point 
    from a raw vector data, described by the tensor size.*/
    template <typename VECT, typename MAT>
    void set_matrix (VECT &V1, 
      size_type cv, size_type i, 
      const MAT& M, bool filter = true) const{
      if(V1.size() == 0 && M.size() == 0) return;
      size_type nb_data = V1.size()/((filter)?nb_filtered_index():nb_index());
      GMM_ASSERT1(nb_data == nb_tensor_elem_,    "Invalid tensorial size for vector V1");
      GMM_ASSERT1(tensor_size_.size() == 2, "ImData is not matrix type");
      GMM_ASSERT1(tensor_size_[0] == M.nrows() 
        && tensor_size_[1] == M.ncols(), "M size is incompatible");

      size_type   iPt = 0;
      if(!filter) iPt = index_of_point(cv,i);
      else        iPt = filtered_index_of_point(cv,i);

      for(size_type i_comp = 0; i_comp < nb_tensor_elem_; ++i_comp)
        V1[iPt*nb_tensor_elem_ +i_comp] = M.as_vector()[i_comp];
    }

    /**set a tensor of a integration point 
    from a raw vector data, described by the tensor size.*/
    template <typename VECT, typename TENSOR>
    void set_tensor (VECT &V1, 
      size_type cv, size_type i, 
      const TENSOR& T, bool filter = true) const{
      if(V1.size() == 0 && T.size() == 0) return;
      size_type nb_data = V1.size()/((filter)?nb_filtered_index():nb_index());
      GMM_ASSERT1(nb_data == nb_tensor_elem_,    "Invalid tensorial size for vector V1");
      GMM_ASSERT1(tensor_size_ == T.sizes(), "ImData tensor is incompatible with T");

      size_type   iPt = 0;
      if(!filter) iPt = index_of_point(cv,i);
      else        iPt = filtered_index_of_point(cv,i);

      for(size_type i_comp = 0; i_comp < nb_tensor_elem_; ++i_comp)
        V1[iPt*nb_tensor_elem_ +i_comp] = T.as_vector()[i_comp];
    }

  private:
    const mesh_im &im_;
    void  update_index_() const;

    mutable size_type              nb_filtered_index_;
    mutable size_type              nb_index_;
    mutable std::vector<size_type> filtered_int_point_index_; 
    mutable std::vector<size_type> int_point_index_; 
    mutable dal::bit_vector        filtered_convex_index_;
    mutable bool                   require_update_;
    mutable gmm::uint64_type       v_num_;

    size_type              filtered_region_;
    bgeot::multi_index     tensor_size_;
    size_type              nb_tensor_elem_;
  };
}
#endif /* GETFEM_IM_DATA_H__  */