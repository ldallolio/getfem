/* -*- c++ -*- (enables emacs c++ mode)                                    */
#ifndef GETFEM_MESH_SLICERS_H
#define GETFEM_MESH_SLICERS_H

#include <bitset>
#include <gmm_kernel.h>
#include <memory> // auto_ptr for g++ 2.95
#include <getfem_mesh_fem.h>
#include <bgeot_rtree.h>

namespace getfem {
  /**
     node data in a slice: contains both real position, and position in the reference convex
   */
  
  struct slice_node {
    typedef std::bitset<32> faces_ct; /// broken for convexes with more than 32 faces
    base_node pt, pt_ref;
    faces_ct faces; 
    slice_node() {}
    slice_node(const base_node& pt_, const base_node& pt_ref_) : pt(pt_), pt_ref(pt_ref_) {}
    void swap(slice_node &other) { 
      std::swap(faces,other.faces); pt.swap(other.pt); pt_ref.swap(other.pt_ref);
    }
  };
  

  /**
     simplex data in a slice: just a list of slice_node ids.
   */
  struct slice_simplex {
    std::vector<size_type> inodes;
    size_type dim() const { return inodes.size()-1; }
    slice_simplex(size_type n) : inodes(n) {} 
    slice_simplex() : inodes(4) {}
    bool operator==(const slice_simplex& o) const { return inodes == o.inodes; }
    bool operator!=(const slice_simplex& o) const { return inodes != o.inodes; }
  };

  /**
     mesh slice: a list of nodes/simplexes, which can be seen as a P1 discontinuous
     mesh_fem on which the interpolation is very fast
  */
  class slicer_action;
  class stored_mesh_slice;

  class mesh_slicer {
    std::deque<slicer_action*> action; /* pointed actions are not deleted */
  public:
    typedef std::vector<slice_node> cs_nodes_ct;
    typedef std::vector<slice_simplex> cs_simplexes_ct;
    const getfem_mesh& m;
    size_type cv, face, cv_dim, cv_nbfaces;
    bgeot::pgeometric_trans pgt;
    /* list of nodes and simplexes for the current convex 
       (including those who are outside of the slice) */
    cs_nodes_ct nodes;    
    cs_simplexes_ct simplexes;
    /* indexes of used simplexes and nodes,
       and index of simplexes who are considered to be in
       the slice */
    dal::bit_vector simplex_index, nodes_index, splx_in;
    size_type fcnt;
    bgeot::pconvex_ref cvr, prev_cvr;

    void pack(); /* not used, indeed */
    void update_nodes_index();

    mesh_slicer(const getfem_mesh& m_) : m(m_), pgt(0), cvr(0) {}
    void push_back_action(slicer_action &a) { action.push_back(&a); }
    void push_front_action(slicer_action &a) { action.push_front(&a); }

    size_type add_simplex(const slice_simplex& s, bool isin) {
      size_type i = simplexes.size();
      simplexes.push_back(s); splx_in[i] = isin; simplex_index.add(i); return i;
    }
    void sup_simplex(size_type i) {
      splx_in.sup(i); simplex_index.sup(i);
    }
    size_type dim() { 
      if (nodes.size()) return nodes[0].pt.size(); 
      else DAL_THROW(dal::internal_error,"");
    }
    /**
       build a new mesh_slice given:
       @param m the mesh that is to be sliced
       @param nrefine number of refinments for each convex of the original mesh
       @param cvlst the list of convex numbers (or convex faces) of m that will 
       be taken into account for the slice
    */
    void exec(size_type nrefine, convex_face_ct& cvlst); 
    void exec(size_type nrefine = 1);
    /**
       build a new mesh slice given:
       @param sl an initial stored_mesh_slice
    */
    void exec(const stored_mesh_slice& sms);
    /**
       build a new mesh_slice than can be used to interpolate a field
       on a fixed set of points.

       @param m the mesh that is to be sliced
       @param ms a slicer
       @param cvlst the list of convex numbers (or convex faces) of m that will 
       be taken into account for the slice       
       @param pts the list of points
     */
     void exec(const std::vector<base_node>& pts);
  private:
    void update_cv_data(size_type cv_, size_type f_ = size_type(-1));
    void init_indexes();
    void apply_slicers();
  };


  /* stupid class in order to use any vector type for field data associated to mesh_fems
     in slices (used for slice deformation and isovalues) */
  class mesh_slice_cv_dof_data_base {
  public:
    const mesh_fem *pmf;    
    virtual void copy(size_type cv, base_vector& coeff) const = 0;
    virtual scalar_type maxval() const = 0;
    virtual ~mesh_slice_cv_dof_data_base() {}
    virtual mesh_slice_cv_dof_data_base* clone() const = 0;
  };

  /**
     use this structure to specify that the mesh must be deformed 
     (with a mesh_fem and an associated field)
     before the slicing 
  */
  template<typename VEC> class mesh_slice_cv_dof_data : public mesh_slice_cv_dof_data_base {
    const VEC u;
  public:
    mesh_slice_cv_dof_data(const mesh_fem &mf_, VEC &u_) : u(u_) { pmf = &mf_; }
    virtual void copy(size_type cv, base_vector& coeff) const {
      coeff.resize(pmf->nb_dof_of_element(cv));
      ref_mesh_dof_ind_ct dof = pmf->ind_dof_of_element(cv);
      base_vector::iterator out = coeff.begin();
      for (ref_mesh_dof_ind_ct::iterator it=dof.begin(); it != dof.end(); ++it, ++out)
        *out = u[*it];
    }
    scalar_type maxval() const { return gmm::vect_norminf(u); }
    virtual mesh_slice_cv_dof_data_base* clone() const {
      return new mesh_slice_cv_dof_data<VEC>(*this);
    }
  };
  

  /**
     generic slicer class: given a list of slice_simplex/slice_node, it 
     build a news list of slice_simplex/slice_node, indicating which ones 
     are in the slice with the bit_vector splx_in
  */
  class slicer_action {
  public:
    static const float EPS;
    virtual void exec(mesh_slicer &ms) = 0;
    virtual ~slicer_action() {}
  };

  /**
     extraction of the boundary of a slice
  */
  class slicer_boundary : public slicer_action {
    slicer_action *A;
    std::vector<slice_node::faces_ct> convex_faces;
    bool test_bound(const slice_simplex& s, slice_node::faces_ct& fmask, 
                    const mesh_slicer::cs_nodes_ct& nodes) const;
  public:
    slicer_boundary(const getfem_mesh& m, slicer_action &sA, const convex_face_ct& fbound);
    void exec(mesh_slicer &ms);
  };

  /* apply a precompted deformation to the slice nodes */
  class slicer_apply_deformation : public slicer_action {
    mesh_slice_cv_dof_data_base *defdata;
    pfem pf;
    fem_precomp_pool fprecomp;
    bgeot::stored_point_tab ref_pts;
 public:
    slicer_apply_deformation(mesh_slice_cv_dof_data_base &defdata_) 
      : defdata(&defdata_), pf(0) {
	if (defdata && defdata->pmf->get_qdim() != defdata->pmf->linked_mesh().dim()) 
	  DAL_THROW(dal::dimension_error, "wrong Q(=" << int(defdata->pmf->get_qdim()) 
		    << ") dimension for slice deformation: should be equal to "
		    "the mesh dimension which is " << int(defdata->pmf->linked_mesh().dim()));
      }    
    void exec(mesh_slicer &ms);
  };

  /**
     base class for general slices of a mesh (planar, sphere, cylinder,isosurface)
  */
  class slicer_volume : public slicer_action {
  public:
    enum {VOLIN=-1, VOLBOUND=0, VOLOUT=+1, VOLSPLIT=+2}; 
  protected:
    /*
      orient defines the kind of slicing :
        VOLIN -> keep the inside of the volume,
	VOLBOUND -> its boundary,
	VOLOUT -> its outside,
	VOLSPLIT -> keep everything but make split simplexes 
	untils no simplex crosses the boundary
    */
    int orient;
    dal::bit_vector pt_in, pt_bin;    
    
    /* overload either 'prepare' or 'test_point' */
    virtual void prepare(size_type /*cv*/, const mesh_slicer::cs_nodes_ct& nodes, const dal::bit_vector& nodes_index) {
      pt_in.clear(); pt_bin.clear();
      for (dal::bv_visitor i(nodes_index); !i.finished(); ++i) {
	bool in, bin; test_point(nodes[i].pt, in, bin);        
	if (bin || ((orient > 0) ? !in : in)) pt_in.add(i);
        if (bin) pt_bin.add(i);
      }
    }
    virtual void test_point(const base_node&, bool& in, bool& bound) const { in=true; bound=true; }

    /* edge_intersect should always be overloaded */
    virtual scalar_type edge_intersect(size_type /*i*/, size_type /*j*/, 
				       const mesh_slicer::cs_nodes_ct& /*nodes*/) const = 0;

    slicer_volume(int orient_) : orient(orient_) {}

    /* utility function */
    static scalar_type trinom(scalar_type a, scalar_type b, scalar_type c) {
      scalar_type delta = b*b - 4*a*c;
      if (delta < 0.) return 1./EPS;
      delta = sqrt(delta);
      scalar_type s1 = (-b - delta) / (2*a);
      scalar_type s2 = (-b + delta) / (2*a);
      if (dal::abs(s1-.5) < dal::abs(s2-.5)) return s1; else return s2;
    }
    void split_simplex(mesh_slicer &ms,
                       slice_simplex s, /* s is NOT a reference, it is on purpose (push_back in the function)*/
                       size_type sstart, const std::bitset<32> spin, const std::bitset<32> spbin);
  public:
    void exec(mesh_slicer &ms);
  };

  /**
     slices a mesh with a half-space (or its boundary)
  */
  class slicer_half_space : public slicer_volume {
    const base_node x0, n; /* normal directed from inside toward outside */
    void test_point(const base_node& P, bool& in, bool& bound) const {
      scalar_type s = gmm::vect_sp(P-x0,n);
      in = (s <= 0); bound = (s*s <= EPS); //*gmm::vect_norm2_sqr(P-x0)); NO!No!
      // do not try to be smart with the boundary check .. easily broken with slicer_mesh_with_mesh
    }
    scalar_type edge_intersect(size_type iA, size_type iB, const mesh_slicer::cs_nodes_ct& nodes) const {
      const base_node& A=nodes[iA].pt;
      const base_node& B=nodes[iB].pt;
      scalar_type s1 = 0., s2 = 0.;
      for (unsigned i=0; i < A.size(); ++i) { s1 += (A[i] - B[i])*n[i]; s2 += (A[i]-x0[i])*n[i]; }
      if (dal::abs(s1) < EPS) return 1./EPS;
      else return s2/s1;
    }
  public:
    slicer_half_space(base_node x0_, base_node n_, int orient_) : 
      slicer_volume(orient_), x0(x0_), n(n_/bgeot::vect_norm2(n_)) {
	//n *= (1./bgeot::vect_norm2(n));
    }
  };

  /**
     slices a mesh with a sphere (or its boundary)
  */
  class slicer_sphere : public slicer_volume {
    base_node x0;
    scalar_type R;
    void test_point(const base_node& P, bool& in, bool& bound) const {
      scalar_type R2 = bgeot::vect_dist2_sqr(P,x0);
      bound = (R2 >= (1-EPS)*R*R && R2 <= (1+EPS)*R*R);
      in = R2 <= R*R;
    }
    scalar_type edge_intersect(size_type iA, size_type iB, const mesh_slicer::cs_nodes_ct& nodes) const {
      const base_node& A=nodes[iA].pt;
      const base_node& B=nodes[iB].pt;
      scalar_type a,b,c; // a*x^2 + b*x + c = 0
      a = bgeot::vect_norm2_sqr(B-A); if (a < EPS) return pt_bin.is_in(iA) ? 0. : 1./EPS;
      b = 2*bgeot::vect_sp(A-x0,B-A);
      c = bgeot::vect_norm2_sqr(A-x0)-R*R;
      return slicer_volume::trinom(a,b,c);
    }
  public:
    /* orient = -1 => select interior, 
       orient = 0  => select boundary 
       orient = +1 => select exterior */
    slicer_sphere(base_node x0_, scalar_type R_, int orient_) : 
      slicer_volume(orient_), x0(x0_), R(R_) {} //cerr << "slicer_volume, x0=" << x0 << ", R=" << R << endl; }
  };
  
  /**
     slices a mesh with a cylinder (or its boundary)
  */
  class slicer_cylinder : public slicer_volume {
    base_node x0, d;
    scalar_type R;
    void test_point(const base_node& P, bool& in, bool& bound) const {
      base_node N = P-x0;
      scalar_type axpos = bgeot::vect_sp(d, N);
      scalar_type dist2 = bgeot::vect_norm2_sqr(N) - dal::sqr(axpos);
      bound = dal::abs(dist2-R*R) < EPS;
      in = dist2 < R*R;
    }
    scalar_type edge_intersect(size_type iA, size_type iB, const mesh_slicer::cs_nodes_ct& nodes) const {
      base_node F=nodes[iA].pt-x0; scalar_type Fd = bgeot::vect_sp(F,d);
      base_node D=nodes[iB].pt-nodes[iA].pt; scalar_type Dd = bgeot::vect_sp(D,d);
      scalar_type a = bgeot::vect_norm2_sqr(D) - dal::sqr(Dd); if (a < EPS) return pt_bin.is_in(iA) ? 0. : 1./EPS; assert(a> -EPS);
      scalar_type b = 2*(bgeot::vect_sp(F,D) - Fd*Dd);
      scalar_type c = bgeot::vect_norm2_sqr(F) - dal::sqr(Fd) - dal::sqr(R);
      return slicer_volume::trinom(a,b,c);
    }
  public:
    slicer_cylinder(base_node x0_, base_node x1_, scalar_type R_, int orient_) : 
      slicer_volume(orient_), x0(x0_), d(x1_-x0_), R(R_) {
      d /= bgeot::vect_norm2(d);
    }
  };


  /**
     extract an isosurface
  */
  class slicer_isovalues : public slicer_volume {
    std::auto_ptr<const mesh_slice_cv_dof_data_base> mfU;
    scalar_type val;
    scalar_type val_scaling; /* = max(abs(U)) */
    std::vector<scalar_type> Uval;
    void prepare(size_type cv, const mesh_slicer::cs_nodes_ct& nodes, const dal::bit_vector& nodes_index);
    scalar_type edge_intersect(size_type iA, size_type iB, const mesh_slicer::cs_nodes_ct& /*nodes*/) const {
      assert(iA < Uval.size() && iB < Uval.size()); /* this should never happen! */
      if (((Uval[iA] < val) && (Uval[iB] > val)) ||
	  ((Uval[iA] > val) && (Uval[iB] < val)))
	return (val-Uval[iA])/(Uval[iB]-Uval[iA]);
      else return 1./EPS;
    }
  public:
    /* orient = -1: u(x) <= val, 0: u(x) == val, +1: u(x) >= val */
    slicer_isovalues(const mesh_slice_cv_dof_data_base& mfU_, scalar_type val_, int orient_) : 
      slicer_volume(orient_), mfU(mfU_.clone()), val(val_) {
	if (mfU->pmf->get_qdim() != 1) DAL_THROW(dal::failure_error, "can't compute isovalues of a vector field !");
	val_scaling = mfU->maxval();
      }
  };

  /** 
      slices a mesh with another mesh (of same dimension,
      and whose convex are preferably linear). Note that slicing
      a refined mesh with a rough mesh should be faster than slicing 
      a rough mesh with a refined mesh.
  */
  class slicer_mesh_with_mesh : public slicer_action {
    const getfem_mesh& slm;
    bgeot::rtree tree; /* tree of bounding boxes for slm */
  protected:
    virtual void intersection_callback(mesh_slicer &/*ms*/, size_type /*slmcv*/) {}
  public:
    slicer_mesh_with_mesh(const getfem_mesh&);
    void exec(mesh_slicer &ms);
  };

  /**
     union of two slices
  */
  class slicer_union : public slicer_action {
    slicer_action *A, *B;
  public:
    slicer_union(slicer_action &sA, slicer_action &sB) : A(&sA), B(&sB) {}
    void exec(mesh_slicer &ms);
  };

  /**
     intersection of two slices
  */
  class slicer_intersect : public slicer_action {
    slicer_action *A, *B;
  public:
    slicer_intersect(slicer_action &sA, slicer_action &sB) : A(&sA), B(&sB) {}
    void exec(mesh_slicer &ms);
  };

  /**
     complementary of a slice
  */
  class slicer_complementary : public slicer_action {
    slicer_action *A;
  public:
    slicer_complementary(slicer_action &sA) : A(&sA) {}
    void exec(mesh_slicer &ms);
  };
  
  /**
     slicer whose side-effect is to compute the area of the
     slice. Note that if the slice is composed of simplexes of many
     dimensions, the resulting area is nonsense.
  */
  class slicer_compute_area : public slicer_action {
    scalar_type a;
  public:
    slicer_compute_area() : a(0) {}
    void exec(mesh_slicer &ms);
    scalar_type area() const { return a; }
  };

  /**
     slicer whose side-effect is to build the list of edges
     (i.e. segments) and store them in a mesh object (hence all common
     nodes/edges are eliminated) slice_edges contains the list of
     edges which where not part of the original mesh, but became
     apparent when some convex faces were sliced.  (this slicer is not
     useful for anything but visualization of sliced meshes)
  */
  class slicer_build_edges_mesh : public slicer_action {
    getfem_mesh& edges_m;
    dal::bit_vector *pslice_edges;
  public:
    slicer_build_edges_mesh(getfem_mesh& edges_m_) : edges_m(edges_m_), pslice_edges(0) {}
    slicer_build_edges_mesh(getfem_mesh& edges_m_, dal::bit_vector& bv) : edges_m(edges_m_), pslice_edges(&bv) {}
    void exec(mesh_slicer &ms);
  };

  /**
     slicer whose side-effect is to build a mesh from the slice simplexes
  */
  class slicer_build_mesh : public slicer_action {
    getfem_mesh& m;
  public:
    slicer_build_mesh(getfem_mesh& m_) : m(m_) {}
    void exec(mesh_slicer &ms);
  };    

  /**
     contract or expand each convex with respect to its gravity center
  */
  class slicer_explode : public slicer_action {
    scalar_type coef; /* < 1 => contraction, > 1 -> expansion */
  public:
    slicer_explode(scalar_type c) : coef(c) {}
    void exec(mesh_slicer &ms);
  };

  /** 
      this slicer does nothing! 
  */
  class slicer_none : public slicer_action {
  public:
    slicer_none() {}
    void exec(mesh_slicer &/*ms*/) {}
  };
}

#endif /*GETFEM_MESH_SLICERS_H*/
