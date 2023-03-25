/* -*- c++ -*- (enables emacs c++ mode) */
/*===========================================================================

 Copyright (C) 2005-2020 Yves Renard, Julien Pommier

 This file is a part of GetFEM

 GetFEM  is  free software;  you  can  redistribute  it  and/or modify it
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
@author  Yves Renard, Julien Pommier
@date 2005.
@brief  region objects (set of convexes and/or convex faces)
*/

#pragma once

#include <atomic>
#include <bitset>
#include <iostream>
#include <map>

#include "dal_bit_vector.h"
#include "bgeot_convex_structure.h"
#include "getfem_config.h"

namespace getfem {
  class mesh;

  /** structure used to hold a set of convexes and/or convex faces.
     @see mesh::region
  */
  class APIDECL mesh_region {
  public:
    using face_bitset = std::bitset<MAX_FACES_PER_CV+1>;
    using map_t = std::map<size_type, face_bitset>;

  private:

    using const_iterator = map_t::const_iterator;

    struct impl {
      mutable map_t m;
      mutable omp_distribute<dal::bit_vector> index_;
      mutable dal::bit_vector serial_index_;
    };
    std::shared_ptr<impl> p;  /* the real region data */

    size_type id_;     /* used temporarily when the
                          mesh_region(size_type) constructor is used */

    omp_distribute<bool> partitioning_allowed; /** specifies that in
                          multithreaded code only a partition of the
                          region is visible in index() and size() methods,
                          as well as during iteration with mr_visitor */
    mesh *parent_mesh; /* used for mesh_region "extracted" from
                          a mesh (to provide feedback) */


    //cashing iterators for partitions
    mutable omp_distribute<const_iterator> itbegin;
    mutable omp_distribute<const_iterator> itend;

    //flags for all the cashes
    mutable omp_distribute<bool> index_updated;
    mutable omp_distribute<bool> partitions_updated;
    mutable bool serial_index_updated;

    void mark_region_changed() const;

    void update_index() const;

    void update_partition_iterators() const;

    impl &wp() { return *p.get(); }
    const impl &rp() const { return *p.get(); }
    void clean();
    /** tells the owner mesh that the region is valid */
    void touch_parent_mesh();

    /**when running while multithreaded, gives the iterator
    for the beginning of the region partition for the current thread*/
    const_iterator partition_begin() const;

    /**when running while multithreaded, gives the iterator
    for the end of the region partition for the current thread*/
    const_iterator partition_end() const;

    /**begin iterator of the region depending if its partitioned or not*/
    const_iterator begin() const;

    /**end iterator of the region depending if its partitioned or not*/
    const_iterator end() const;

    /**number of region entries before partitioning*/
    size_type unpartitioned_size() const;

  public:
    mesh_region(const mesh_region &other);
    mesh_region();
    /** a mesh_region can be built from a integer parameter
    (a region number in a mesh),
    but it won't be usable until 'from_mesh(m)' has been called
    Note that these regions are read-only, this constructor is
    mostly used for backward-compatibility.
    */
    mesh_region(size_type id__);

    /** internal constructor. You should used m.region(id) instead. */
    mesh_region(mesh& m, size_type id__);
    /** build a mesh_region from a convex list stored in a bit_vector. */
    mesh_region(const dal::bit_vector &bv);

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
    /** subtract the second region from the first one */
    static mesh_region subtract(const mesh_region &a,
                                const mesh_region &b);
    /** Test if the region is a boundary of a list of faces of elements of
        region `rg`. Return 0 if not, -1 if only partially, 1 if the region
        contains only some faces which are all faces of elements of `rg`. */
    int region_is_faces_of(const getfem::mesh& m1,
                           const mesh_region &rg2,
                           const getfem::mesh& m2) const;

    size_type id() const { return id_; }

    /** In multithreaded part of the program makes only a partition of the 
    region visible in the index() and size() operations, as well as during 
    iterations with mr_visitor. This is a default behaviour*/
    void  allow_partitioning();

    /** Return the bounding box [Pmin - Pmax] of the mesh_region. */
    void bounding_box(base_node& Pmin, base_node& Pmax) const;

    /** Disregard partitioning, which means being able to see the whole region
    in multithreaded code. Can be used, for instance, for contact problems
    where master region is partitioned, while the slave region is not*/
    void  prohibit_partitioning();

    bool is_partitioning_allowed() const;

    /** Extract the next region number
    that does not yet exists in the mesh*/
    static size_type free_region_id(const getfem::mesh& m);

    /** For regions which have been built with just a number 'id',
    from_mesh(m) sets the current region to 'm.region(id)'.
    (works only once)
    */
    const mesh_region& from_mesh(const mesh &m) const;

    mesh_region& operator=(const mesh_region &mr);

    bool compare(const mesh &m1, const mesh_region &mr, const mesh &m2) const;

    face_bitset operator[](size_t cv) const;

    /** Index of the region convexes, or the convexes from the partition on the
    current thread. */
    const dal::bit_vector& index() const;
    void add(const dal::bit_vector &bv);
    void add(size_type cv, short_type f = short_type(-1));
    void sup(size_type cv, short_type f = short_type(-1));
    void sup_all(size_type cv);
    void clear();
    void swap_convex(size_type cv1, size_type cv2);
    bool is_in(size_type cv, short_type f = short_type(-1)) const;
    bool is_in(size_type cv, short_type f, const mesh &m) const;

    /** Region size, or the size of the region partition on the current
    thread if the region is partitioned*/
    size_type size() const;

    /** Number of convexes in the region, or on the partition on the current
    thread*/
    size_type nb_convex() const { return  index().card();}
    bool is_empty() const;
    /** Return true if the region do contain only convex faces */
    bool is_only_faces() const;
    bool is_boundary() const { return is_only_faces(); }
    /** Return true if the region do not contain any convex face */
    bool is_only_convexes() const;
    face_bitset faces_of_convex(size_type cv) const;
    face_bitset and_mask() const;
    face_bitset or_mask() const;
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

      typedef mesh_region::map_t::const_iterator const_iterator;
      bool whole_mesh;
      dal::bit_const_iterator itb, iteb;
      const_iterator it, ite;
      face_bitset c;
      size_type cv_;
      short_type f_;
      bool finished_;
#if GETFEM_PARA_LEVEL > 1
      std::unique_ptr<mesh_region> mpi_rg;
#endif
      void init(const mesh_region &s);
      void init(const dal::bit_vector &s);

    public:
      visitor(const mesh_region &s);
      visitor(const mesh_region &s, const mesh &m,
        bool intersect_with_mpi = false);
      size_type cv() const { return cv_; }
      size_type is_face() const { return f_ != 0; }
      short_type f() const { return short_type(f_-1); }

      bool next();
      bool operator++() { return next(); }

      bool finished() const { return finished_; }

      bool next_face(){
        if (whole_mesh) return false;
        if (c.none()) return false;
        do { ++f_; } while (!c.test(f_));
        c.set(f_,0);
        return true;
      }
    };

    friend std::ostream & operator <<(std::ostream &os, const mesh_region &w);
  };

  using mr_visitor = mesh_region::visitor;

  /** Dummy mesh_region for default parameter of functions. */
  const mesh_region &dummy_mesh_region();

}  /* end of namespace getfem.                                             */