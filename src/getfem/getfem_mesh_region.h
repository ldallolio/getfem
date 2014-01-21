/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================
 
 Copyright (C) 2005-2012 Yves Renard, Julien Pommier
 
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

/**@file getfem_mesh_region.h
  @author  Yves Renard <Yves.Renard@insa-lyon.fr>, Julien Pommier <Julien.Pommier@insa-toulouse.fr>
  @date 2005.
   @brief  region objects (set of convexes and/or convex faces)
*/

#ifndef GETFEM_MESH_REGION
#define GETFEM_MESH_REGION

#include <map>
#include <bitset>
#include <iostream>
#include "dal_bit_vector.h"
#include "dal_shared_ptr.h"
#include "bgeot_convex_structure.h"
#include "getfem_config.h"

//#define MAX_FACES_PER_CV 63

namespace getfem {
  class mesh;

  /** structure used to hold a set of convexes and/or convex faces.
      @see mesh::region
   */
  class mesh_region {
  public:
    typedef std::bitset<MAX_FACES_PER_CV+1> face_bitset;
    typedef std::map<size_type,face_bitset> map_t;
  private:
    struct impl {
      mutable dal::bit_vector index_;
      mutable map_t m;
    };
    dal::shared_ptr<impl> p;  /* the real region data */
    size_type id_;            /* used temporarily when the 
				 mesh_region(size_type) constructor is used */

    size_type type_; //optional type of the region
    mesh *parent_mesh; /* used for mesh_region "extracted" from
				 a mesh (to provide feedback) */
    impl &wp() { return *p.get(); }
    const impl &rp() const { return *p.get(); }
    void clean();
    /** tells the owner mesh that the region is valid */
    void touch_parent_mesh();
  public:
    mesh_region(const mesh_region &other);
    mesh_region() : p(new impl), id_(size_type(-2)), type_(size_type(-1)),
                    parent_mesh(0) {}
    /** a mesh_region can be built from a integer parameter 
	(a region number in a mesh),
	but it won't be usable until 'from_mesh(m)' has been called 
	Note that these regions are read-only, this constructor is
	mostly used for backward-compatibility.
    */
    mesh_region(size_type id__) : id_(id__), type_(size_type(-1)),
                                  parent_mesh(0) {}
    /** internal constructor. You should used m.region(id) instead. */
    mesh_region(mesh& m, size_type id__, size_type type = size_type(-1)) : 
      p(new impl), id_(id__), type_(type), parent_mesh(&m)  {}
    /** build a mesh_region from a convex list stored in a bit_vector. */
    mesh_region(const dal::bit_vector &bv) : 
      p(new impl), id_(size_type(-2)), type_(size_type(-1)),
      parent_mesh(0) { add(bv); }
    /** provide a default value for the mesh_region parameters of assembly
        procedures etc. */
    static mesh_region all_convexes() {
      return mesh_region(size_type(-1)); 
    }
    /** return the intersection of two mesh regions */
    static mesh_region intersection(const mesh_region& a, 
                                    const mesh_region& b); 
    /** return the union of two mesh_regions */
    static mesh_region merge(const mesh_region &a, 
                             const mesh_region &b);
    /** remove the second region from the first one */
    static mesh_region substract(const mesh_region &a, 
                                 const mesh_region &b);
    size_type id() const { return id_; }

    size_type get_type() const { return type_; }

    void  set_type(size_type type)  { type_ = type; }

    /**extract the next region number 
    that does not yet exists in the mesh*/
    static size_type free_region_id(const getfem::mesh& m);


    /** for regions which have been built with just a number 'id',
	from_mesh(m) sets the current region to 'm.region(id)'.  
	(works only once) 
    */
    const mesh_region& from_mesh(const mesh &m) const;

    mesh_region& operator=(const mesh_region &mr);

    face_bitset operator[](size_t cv) const;
    const dal::bit_vector &index() const;
    void add(const dal::bit_vector &bv);
    void add(size_type cv, size_type f = size_type(-1));
    void sup(size_type cv, size_type f = size_type(-1));
    void sup_all(size_type cv);
    void clear();
    void swap_convex(size_type cv1, size_type cv2);
    bool is_in(size_type cv, size_type f = size_type(-1)) const;
    size_type size() const;
    size_type nb_convex() const { return rp().m.size(); }  
    bool is_empty() const;
    /** return true if the region do contain only convex faces */
    bool is_only_faces() const;
    bool is_boundary() const { return is_only_faces(); }
    /** return true if the region do not contain any convex face */
    bool is_only_convexes() const;
    face_bitset faces_of_convex(size_type cv) const;
    face_bitset and_mask() const;
    void error_if_not_faces() const;
    void error_if_not_convexes() const;
    void error_if_not_homogeneous() const;
    const mesh *get_parent_mesh(void) const { return parent_mesh; }
    void set_parent_mesh(mesh *pm) { parent_mesh = pm; }
    
    /** "iterator" class for regions. Usage similar to bv_visitor:
	for (mr_visitor i(region); !i.finished(); ++i) {
	  ...
        }
    */
    class visitor {
      mesh_region::map_t::const_iterator it,ite;
      face_bitset c;
      size_type cv_;
      short_type f_;
      bool finished_;
      void init(const mesh_region &s);
    public: 
      visitor(const mesh_region &s);
      visitor(const mesh_region &s, const mesh &m);
      size_type cv() const { return cv_; }
      size_type is_face() const { return f_ != 0; }
      short_type f() const { return short_type(f_-1); }
      bool next() {
	while (c.none()) {
	  if (it == ite) { finished_=true; return false; }
	  c = (*it).second; cv_ = (*it).first; f_ = short_type(-1);
	  ++it; 
	  if (c.none()) continue;
	}
	next_face();
	return true;
      }
      bool operator++() { return next(); }
      bool finished() const { return finished_; }//it == ite && c.none(); }	
      bool next_face() {
	if (c.none()) return false;
	do { ++f_; } while (!c.test(f_));
	c.set(f_,0);
	return true;
      }
    };

    friend std::ostream & operator <<(std::ostream &os, const mesh_region &w);
  };
  
  typedef mesh_region::visitor mr_visitor;
}


#endif
