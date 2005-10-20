// -*- c++ -*- (enables emacs c++ mode)
/* *********************************************************************** */
/*                                                                         */
/* Copyright (C) 2002-2005 Yves Renard, Julien Pommier.                    */
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

/**
 * Linear Elastostatic problem with a crack.
 *
 * This program is used to check that getfem++ is working. This is also 
 * a good example of use of Getfem++.
*/

#include <getfem_assembling.h> /* import assembly methods (and norms comp.) */
#include <getfem_export.h>   /* export functions (save solution in a file)  */
#include <getfem_derivatives.h>
#include <getfem_regular_meshes.h>
#include <getfem_modeling.h>
#include <getfem_mesh_im_level_set.h>
#include <getfem_mesh_fem_level_set.h>
#include <getfem_mesh_fem_product.h>
#include <getfem_mesh_fem_global_function.h>
#include <getfem_spider_fem.h>
#include <getfem_mesh_fem_sum.h>
#include <gmm.h>

/* try to enable the SIGFPE if something evaluates to a Not-a-number
 * of infinity during computations
 */
#ifdef GETFEM_HAVE_FEENABLEEXCEPT
#  include <fenv.h>
#endif

/* some Getfem++ types that we will be using */
using bgeot::base_small_vector; /* special class for small (dim<16) vectors */
using bgeot::base_node;  /* geometrical nodes(derived from base_small_vector)*/
using bgeot::scalar_type; /* = double */
using bgeot::size_type;   /* = unsigned long */
using bgeot::base_matrix; /* small dense matrix. */

/* definition of some matrix/vector types. These ones are built
 * using the predefined types in Gmm++
 */
typedef getfem::modeling_standard_sparse_vector sparse_vector;
typedef getfem::modeling_standard_sparse_matrix sparse_matrix;
typedef getfem::modeling_standard_plain_vector  plain_vector;



/**************************************************************************/
/*  Structure for the crack problem.                                      */
/**************************************************************************/

struct crack_problem {

  enum { DIRICHLET_BOUNDARY_NUM = 0, NEUMANN_BOUNDARY_NUM = 1};
  getfem::getfem_mesh mesh;  /* the mesh */
  getfem::mesh_level_set mls;       /* the level set aware mesh.             */
  getfem::mesh_im_level_set mim;    /* the integration methods.              */
  getfem::mesh_im_level_set mim_crack;    /* the integration on the crack.   */
  getfem::mesh_fem mf_pre_u; 
  getfem::mesh_fem_level_set mfls_u; 
  getfem::mesh_fem_global_function mf_sing_u;
  getfem::mesh_fem mf_partition_of_unity;
  getfem::mesh_fem_product mf_product;
  getfem::mesh_fem_sum mf_u_sum;

  getfem::mesh_fem& mf_u() { return mf_u_sum; }
  

  getfem::mesh_fem mf_rhs;   /* mesh_fem for the right hand side (f(x),..)   */
  getfem::mesh_fem mf_coef;  /* mesh_fem used to represent pde coefficients  */
  
  scalar_type lambda, mu;    /* Lam� coefficients.                           */
  scalar_type neumann_force;

  getfem::level_set ls;      /* The two level sets defining the crack.       */
  
  scalar_type residue;       /* max residue for the iterative solvers        */
  bool dir_with_mult;
  scalar_type enr_area_radius;
  int enrichment_option;
  
  std::string datafilename;
  ftool::md_param PARAM;

  bool solve(plain_vector &U);
  void init(void);
  crack_problem(void) : mls(mesh), mim(mls), mim_crack(mls, getfem::mesh_im_level_set::INTEGRATE_BOUNDARY), mf_pre_u(mesh),
			mfls_u(mls, mf_pre_u), mf_sing_u(mesh),
			mf_partition_of_unity(mesh),
			mf_product(mf_partition_of_unity, mf_sing_u),
			mf_u_sum(mesh),
			mf_rhs(mesh), mf_coef(mesh),  ls(mesh, 1, true) {}
};

/* Read parameters from the .param file, build the mesh, set finite element
 * and integration methods and selects the boundaries.
 */
void crack_problem::init(void) {
  const char *MESH_TYPE = PARAM.string_value("MESH_TYPE","Mesh type ");
  const char *FEM_TYPE  = PARAM.string_value("FEM_TYPE","FEM name");
  const char *INTEGRATION = PARAM.string_value("INTEGRATION",
					       "Name of integration method");
  const char *SIMPLEX_INTEGRATION = PARAM.string_value("SIMPLEX_INTEGRATION",
					 "Name of simplex integration method");

  enrichment_option = PARAM.int_value("ENRICHMENT_OPTION",
				      "Enrichment option");
  cout << "MESH_TYPE=" << MESH_TYPE << "\n";
  cout << "FEM_TYPE="  << FEM_TYPE << "\n";
  cout << "INTEGRATION=" << INTEGRATION << "\n";
  
  /* First step : build the mesh */
  bgeot::pgeometric_trans pgt = 
    bgeot::geometric_trans_descriptor(MESH_TYPE);
  size_type N = pgt->dim();
  std::vector<size_type> nsubdiv;
  nsubdiv.resize(N-1, PARAM.int_value("NL", "Number of horizontal space steps "));
  nsubdiv.push_back(PARAM.int_value("NH", "Number of vertical space steps "));
  getfem::regular_unit_mesh(mesh, nsubdiv, pgt,
			    PARAM.int_value("MESH_NOISED") != 0);
  base_small_vector tt(N); tt.fill(-0.5);
  mesh.translation(tt);
  
  bgeot::base_matrix M(N,N);
  for (size_type i=0; i < N; ++i)
    M(i,i) = (i<N-1) ? PARAM.real_value("L", "L") :  PARAM.real_value("H","H");
  mesh.transformation(M);

  datafilename = PARAM.string_value("ROOTFILENAME","Base name of data files.");
  residue = PARAM.real_value("RESIDUE"); if (residue == 0.) residue = 1e-10;
  enr_area_radius = PARAM.real_value("RADIUS_ENR_AREA",
				     "radius of the enrichment area");

  mu = PARAM.real_value("MU", "Lam� coefficient mu");
  lambda = PARAM.real_value("LAMBDA", "Lam� coefficient lambda");
  neumann_force = PARAM.real_value("NEUMANN_FORCE", "Neumann force");

  mf_u().set_qdim(N);

  /* set the finite element on the mf_u */
  getfem::pfem pf_u = 
    getfem::fem_descriptor(FEM_TYPE);
  getfem::pintegration_method ppi = 
    getfem::int_method_descriptor(INTEGRATION);
  getfem::pintegration_method sppi = 
    getfem::int_method_descriptor(SIMPLEX_INTEGRATION);
  

  mim.set_integration_method(mesh.convex_index(), ppi);
  mim_crack.set_integration_method(mesh.convex_index(), ppi);
  mls.add_level_set(ls);
  mim.set_simplex_im(sppi);
  mim_crack.set_simplex_im(sppi);
  mf_pre_u.set_finite_element(mesh.convex_index(), pf_u);
  mf_partition_of_unity.set_classical_finite_element(1);

  dir_with_mult = (PARAM.int_value("DIRICHLET_WITH_MULTIPLIERS") != 0);

  /* set the finite element on mf_rhs (same as mf_u is DATA_FEM_TYPE is
     not used in the .param file */
  const char *data_fem_name = PARAM.string_value("DATA_FEM_TYPE");
  if (data_fem_name == 0) {
    if (!pf_u->is_lagrange()) {
      DAL_THROW(dal::failure_error, "You are using a non-lagrange FEM. "
		<< "In that case you need to set "
		<< "DATA_FEM_TYPE in the .param file");
    }
    mf_rhs.set_finite_element(mesh.convex_index(), pf_u);
  } else {
    mf_rhs.set_finite_element(mesh.convex_index(), 
			      getfem::fem_descriptor(data_fem_name));
  }
  
  /* set the finite element on mf_coef. Here we use a very simple element
   *  since the only function that need to be interpolated on the mesh_fem 
   * is f(x)=1 ... */
  mf_coef.set_finite_element(mesh.convex_index(),
			     getfem::classical_fem(pgt,0));


  /* set boundary conditions
   * (Neuman on the upper face, Dirichlet elsewhere) */
  cout << "Selecting Neumann and Dirichlet boundaries\n";
  getfem::mesh_region border_faces;
  getfem::outer_faces_of_mesh(mesh, border_faces);
  for (getfem::mr_visitor i(border_faces); !i.finished(); ++i) {
    base_node un = mesh.normal_of_face_of_convex(i.cv(), i.f());
    un /= gmm::vect_norm2(un);
    if (gmm::abs(un[0] + 1.0) >= 1.0E-7) { // new Neumann face
      mesh.region(NEUMANN_BOUNDARY_NUM).add(i.cv(), i.f());
    } else {
      mesh.region(DIRICHLET_BOUNDARY_NUM).add(i.cv(), i.f());
    }
  }

}

/**************************************************************************/
/*  Model.                                                                */
/**************************************************************************/

bool crack_problem::solve(plain_vector &U) {
  size_type nb_dof_rhs = mf_rhs.nb_dof();
  size_type N = mesh.dim();

  ls.reinit();  
  cout << "ls.get_mesh_fem().nb_dof() = " << ls.get_mesh_fem().nb_dof() << "\n";
  for (size_type d = 0; d < ls.get_mesh_fem().nb_dof(); ++d) {
    const base_node P = ls.get_mesh_fem().point_of_dof(d);
    ls.values(0)[d] = P[N-1];
    ls.values(1)[d] = -P[0] + .2 - P[1]; //+ 0.3 - gmm::sqr((ls.get_mesh_fem().point_of_dof(d))[1])*2.0;
  }
  ls.touch();

  mls.adapt();
  mim.adapt();
  mim_crack.adapt();

  cout << "testing mims..\n";
  getfem::mesh_fem mf(mim.linked_mesh()); mf.set_classical_finite_element(0);
  scalar_type vol1 = gmm::sqr(getfem::asm_L2_norm(mim, mf, std::vector<double>(mf.nb_dof(), 1.0)));
  scalar_type surf1 = gmm::sqr(getfem::asm_L2_norm(mim, mf, std::vector<double>(mf.nb_dof(), 1.0), NEUMANN_BOUNDARY_NUM));
  scalar_type surfcrack =  gmm::sqr(getfem::asm_L2_norm(mim_crack, mf, std::vector<double>(mf.nb_dof(), 1.0)));

  cout << "surface of the crack : " << surfcrack << endl;
  cout << "volume of mesh is " << vol1 << " surf = " << surf1 << "\n";
  getfem::mesh_im mim2(mim.linked_mesh()); mim2.set_integration_method(mim.linked_mesh().convex_index(), 6);
  scalar_type vol2 = gmm::sqr(getfem::asm_L2_norm(mim2, mf, std::vector<double>(mf.nb_dof(), 1.0)));
  scalar_type surf2 = gmm::sqr(getfem::asm_L2_norm(mim2, mf, std::vector<double>(mf.nb_dof(), 1.0), NEUMANN_BOUNDARY_NUM));
  cout << "volume of mesh is " << vol2 << " surf = " << surf2 << "\n";
  assert(gmm::abs(vol1-vol2) < 1e-5);
  assert(gmm::abs(surf1-surf2) < 1e-5);

  
  cerr << "CV INDEX: " << mim_crack.convex_index() << "\n";
  for (dal::bv_visitor i(mim_crack.convex_index()); !i.finished(); ++i)
    cerr << "     !!!!!!!!!!!!!!!!!!! " << mim_crack.int_method_of_element(i) << "\n";

  mfls_u.adapt();
  std::vector<getfem::pglobal_function> vfunc(4);
  for (size_type i = 0; i < 4; ++i)
    vfunc[i] = isotropic_crack_singular_2D(i, ls, 0.0);
  
  mf_sing_u.set_functions(vfunc);


  switch (enrichment_option) {
  case 1 :
    {
      dal::bit_vector enriched_dofs;
      plain_vector X(mf_partition_of_unity.nb_dof());
      plain_vector Y(mf_partition_of_unity.nb_dof());
      getfem::interpolation(ls.get_mesh_fem(), mf_partition_of_unity,
			    ls.values(1), X);
      getfem::interpolation(ls.get_mesh_fem(), mf_partition_of_unity,
			    ls.values(0), Y);
      for (size_type j = 0; j < mf_partition_of_unity.nb_dof(); ++j) {
	if (gmm::sqr(X[j]) + gmm::sqr(Y[j]) <= gmm::sqr(enr_area_radius))
	  enriched_dofs.add(j);
      }
      if (enriched_dofs.card() < 3)
	DAL_WARNING(0, "There is " << enriched_dofs.card() <<
		    " enriched dofs for the crack tip");
      mf_product.set_enrichment(enriched_dofs);
      mf_u_sum.set_mesh_fems(mf_product, mfls_u);
    }
    break;
    default : mf_u_sum.set_mesh_fems(mfls_u); break;
  }

  U.resize(mf_u().nb_dof());

  cout << "Number of dof for u: " << mf_u().nb_dof() << endl;

  // Linearized elasticity brick.
  getfem::mdbrick_isotropic_linearized_elasticity<>
    ELAS(mim, mf_u(), mf_coef, lambda, mu);

  // Defining the volumic source term.
  plain_vector F(nb_dof_rhs * N); 
  // Volumic source term brick.
  getfem::mdbrick_source_term<> VOL_F(ELAS, mf_rhs, F);

  // Defining the Neumann condition right hand side.
  gmm::clear(F);
  for (size_type i = 0; i < nb_dof_rhs; ++i)
    F[i*N+N-1] = (mf_rhs.point_of_dof(i))[N-1];
  gmm::scale(F, neumann_force);
 
  // Neumann condition brick.
  getfem::mdbrick_source_term<> NEUMANN(VOL_F, mf_rhs, F,NEUMANN_BOUNDARY_NUM);
  
  gmm::clear(F);
  // Dirichlet condition brick.
  getfem::mdbrick_Dirichlet<> final_model(NEUMANN, mf_rhs, F,
					  DIRICHLET_BOUNDARY_NUM, 0,
					  dir_with_mult);

  // Generic solve.
  cout << "Total number of variables : " << final_model.nb_dof() << endl;
  getfem::standard_model_state MS(final_model);
  gmm::iteration iter(residue, 1, 40000);
  getfem::standard_solve(MS, final_model, iter);

  // Solution extraction
  gmm::copy(ELAS.get_solution(MS), U);

  return (iter.converged());
}
  
/**************************************************************************/
/*  main program.                                                         */
/**************************************************************************/

int main(int argc, char *argv[]) {
  dal::exception_callback_debug cb;
  dal::exception_callback::set_exception_callback(&cb); // to debug ...

#ifdef GETFEM_HAVE_FEENABLEEXCEPT /* trap SIGFPE */
  feenableexcept(FE_DIVBYZERO | FE_INVALID);
#endif

  //getfem::getfem_mesh_level_set_noisy();


  try {
    crack_problem p;
    p.PARAM.read_command_line(argc, argv);
    p.init();
    p.mesh.write_to_file(p.datafilename + ".mesh");
    plain_vector U(p.mf_u().nb_dof());
    if (!p.solve(U)) DAL_THROW(dal::failure_error,"Solve has failed");

    {
      cout << "Post processing...\n";
      getfem::getfem_mesh mcut;
      p.mls.global_cut_mesh(mcut);
//       getfem::mesh_fem mf(mcut, p.mf_u().get_qdim());
//       mf.set_classical_discontinuous_finite_element(2, 0.001);

//       plain_vector V(mf.nb_dof());
//       getfem::interpolation(p.mf_u(), mf, U, V);

      getfem::stored_mesh_slice sl;
      getfem::getfem_mesh mcut_refined;
      sl.build(mcut, /*getfem::slicer_boundary(mcut), */getfem::slicer_build_mesh(mcut_refined), 2);

      getfem::mesh_fem mf_refined(mcut_refined, p.mf_u().get_qdim());
      mf_refined.set_classical_discontinuous_finite_element(1, 0.001);
      plain_vector W(mf_refined.nb_dof());
      getfem::interpolation(p.mf_u(), mf_refined, U, W);

      if (p.PARAM.int_value("VTK_EXPORT")) {
	cout << "export to " << p.datafilename + ".vtk" << "..\n";
	getfem::vtk_export exp(p.datafilename + ".vtk",
			       p.PARAM.int_value("VTK_EXPORT")==1);
	exp.exporting(mf_refined); 
	exp.write_point_data(mf_refined, W, "elastostatic_displacement");
	cout << "export done, you can view the data file with (for example)\n"
	  "mayavi -d " << p.datafilename << ".vtk -f ExtractVectorNorm -f "
	  "WarpVector -m BandedSurfaceMap -m Outline\n";
      }

    }

  }
  DAL_STANDARD_CATCH_ERROR;

  return 0; 
}
