/* cmakeconfig.h.in (can be manually modified)  */
/* Define to dummy `main' function (if any) required to link to the Fortran
   libraries. */
#cmakedefine GETFEM_FC_DUMMY_MAIN @GETFEM_FC_DUMMY_MAIN@

/* Define if F77 and FC dummy `main' functions are identical. */
#cmakedefine GETFEM_FC_DUMMY_MAIN_EQ_F77 @GETFEM_FC_DUMMY_MAIN_EQ_F77@

/* enable openblas to be multithreaded */
#cmakedefine GETFEM_FORCE_SINGLE_THREAD_BLAS @GETFEM_FORCE_SINGLE_THREAD_BLAS@

/* glibc backtrace function */
#cmakedefine GETFEM_HAVE_BACKTRACE @GETFEM_HAVE_BACKTRACE@

/* the use of the blas call for basic algebra routines. */
#cmakedefine GETFEM_HAVE_BLAS_INTERFACE @GETFEM_HAVE_BLAS_INTERFACE@

/* Define to 1 if you have the <cmumps_c.h> header file. */
#cmakedefine GETFEM_HAVE_CMUMPS_C_H @GETFEM_HAVE_CMUMPS_C_H@

/* Define to 1 if you have the <cxxabi.h> header file. */
#cmakedefine GETFEM_HAVE_CXXABI_H @GETFEM_HAVE_CXXABI_H@

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine GETFEM_HAVE_DLFCN_H @GETFEM_HAVE_DLFCN_H@

/* Define to 1 if you have the <dmumps_c.h> header file. */
#cmakedefine GETFEM_HAVE_DMUMPS_C_H @GETFEM_HAVE_DMUMPS_C_H@

/* glibc floating point exceptions control */
#cmakedefine GETFEM_HAVE_FEENABLEEXCEPT @GETFEM_HAVE_FEENABLEEXCEPT@

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine GETFEM_HAVE_INTTYPES_H @GETFEM_HAVE_INTTYPES_H@

/* Define to 1 if you have the `mpi_cxx' library (-lmpi_cxx). */
#cmakedefine GETFEM_HAVE_LIBMPI_CXX @GETFEM_HAVE_LIBMPI_CXX@

/* Define to 1 if you have the <libqhull_r/qhull_ra.h> header file. */
#cmakedefine GETFEM_HAVE_LIBQHULL_R_QHULL_RA_H @GETFEM_HAVE_LIBQHULL_R_QHULL_RA_H@

/* Define to 1 if you have the `superlu' library (-lsuperlu). */
#cmakedefine GETFEM_HAVE_LIBSUPERLU @GETFEM_HAVE_LIBSUPERLU@

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine GETFEM_HAVE_MEMORY_H @GETFEM_HAVE_MEMORY_H@

/* defined if the Metis library was found and is working */
#cmakedefine GETFEM_HAVE_METIS @GETFEM_HAVE_METIS@

/* Define to 1 if you have the <metis.h> header file. */
#cmakedefine GETFEM_HAVE_METIS_H @GETFEM_HAVE_METIS_H@

/* defined if the Metis library found is older than version 4 */
#cmakedefine GETFEM_HAVE_METIS_OLD_API @GETFEM_HAVE_METIS_OLD_API@

/* Define if you have the MPI library. */
#cmakedefine GETFEM_HAVE_MPI @GETFEM_HAVE_MPI@

/* gcc style __PRETTY_FUNCTION__ macro */
#cmakedefine GETFEM_HAVE_PRETTY_FUNCTION @GETFEM_HAVE_PRETTY_FUNCTION@

/* If available, contains the Python version number currently in use. */
#cmakedefine GETFEM_HAVE_PYTHON @GETFEM_HAVE_PYTHON@

/* defined if the qd library was found and is working */
#cmakedefine GETFEM_HAVE_QDLIB @GETFEM_HAVE_QDLIB@

/* Defined to 1 if Scilab is present on the system */
#cmakedefine GETFEM_HAVE_SCILAB @GETFEM_HAVE_SCILAB@

/* Define to 1 if you have the <smumps_c.h> header file. */
#cmakedefine GETFEM_HAVE_SMUMPS_C_H @GETFEM_HAVE_SMUMPS_C_H@

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine GETFEM_HAVE_STDINT_H @GETFEM_HAVE_STDINT_H@

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine GETFEM_HAVE_STDLIB_H @GETFEM_HAVE_STDLIB_H@

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine GETFEM_HAVE_STRINGS_H @GETFEM_HAVE_STRINGS_H@

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine GETFEM_HAVE_STRING_H @GETFEM_HAVE_STRING_H@

/* Define to 1 to on/off SuperLU support. */
#cmakedefine GETFEM_HAVE_SUPERLU @GETFEM_HAVE_SUPERLU@

/* Define to 1 if you have the <superlu/colamd.h> header file. */
#cmakedefine GETFEM_HAVE_SUPERLU_COLAMD_H @GETFEM_HAVE_SUPERLU_COLAMD_H@

/* Define to 1 if you have the <superlu/slu_cdefs.h> header file. */
#cmakedefine GETFEM_HAVE_SUPERLU_SLU_CDEFS_H @GETFEM_HAVE_SUPERLU_SLU_CDEFS_H@

/* Define to 1 if you have the <superlu/slu_Cnames.h> header file. */
#cmakedefine GETFEM_HAVE_SUPERLU_SLU_CNAMES_H @GETFEM_HAVE_SUPERLU_SLU_CNAMES_H@

/* Define to 1 if you have the <superlu/slu_dcomplex.h> header file. */
#cmakedefine GETFEM_HAVE_SUPERLU_SLU_DCOMPLEX_H @GETFEM_HAVE_SUPERLU_SLU_DCOMPLEX_H@

/* Define to 1 if you have the <superlu/slu_ddefs.h> header file. */
#cmakedefine GETFEM_HAVE_SUPERLU_SLU_DDEFS_H @GETFEM_HAVE_SUPERLU_SLU_DDEFS_H@

/* Define to 1 if you have the <superlu/slu_scomplex.h> header file. */
#cmakedefine GETFEM_HAVE_SUPERLU_SLU_SCOMPLEX_H @GETFEM_HAVE_SUPERLU_SLU_SCOMPLEX_H@

/* Define to 1 if you have the <superlu/slu_sdefs.h> header file. */
#cmakedefine GETFEM_HAVE_SUPERLU_SLU_SDEFS_H @GETFEM_HAVE_SUPERLU_SLU_SDEFS_H@

/* Define to 1 if you have the <superlu/slu_zdefs.h> header file. */
#cmakedefine GETFEM_HAVE_SUPERLU_SLU_ZDEFS_H @GETFEM_HAVE_SUPERLU_SLU_ZDEFS_H@

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine GETFEM_HAVE_SYS_STAT_H @GETFEM_HAVE_SYS_STAT_H@

/* Define to 1 if you have the <sys/times.h> header file. */
#cmakedefine GETFEM_HAVE_SYS_TIMES_H @GETFEM_HAVE_SYS_TIMES_H@

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine GETFEM_HAVE_SYS_TYPES_H @GETFEM_HAVE_SYS_TYPES_H@

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine GETFEM_HAVE_UNISTD_H @GETFEM_HAVE_UNISTD_H@

/* Define to 1 if you have the <zmumps_c.h> header file. */
#cmakedefine GETFEM_HAVE_ZMUMPS_C_H @GETFEM_HAVE_ZMUMPS_C_H@

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#cmakedefine GETFEM_LT_OBJDIR @GETFEM_LT_OBJDIR@

/* Major version number */
#cmakedefine GETFEM_MAJOR_VERSION @GETFEM_MAJOR_VERSION@

/* Minor version number */
#cmakedefine GETFEM_MINOR_VERSION @GETFEM_MINOR_VERSION@

/* Name of package */
#cmakedefine GETFEM_PACKAGE @GETFEM_PACKAGE@

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine GETFEM_PACKAGE_BUGREPORT @GETFEM_PACKAGE_BUGREPORT@

/* Define to the full name of this package. */
#cmakedefine GETFEM_PACKAGE_NAME @GETFEM_PACKAGE_NAME@

/* Define to the full name and version of this package. */
#cmakedefine GETFEM_PACKAGE_STRING @GETFEM_PACKAGE_STRING@

/* Define to the one symbol short name of this package. */
#cmakedefine GETFEM_PACKAGE_TARNAME @GETFEM_PACKAGE_TARNAME@

/* Define to the home page for this package. */
#cmakedefine GETFEM_PACKAGE_URL @GETFEM_PACKAGE_URL@

/* Define to the version of this package. */
#cmakedefine GETFEM_PACKAGE_VERSION @GETFEM_PACKAGE_VERSION@

/* Patch version number */
#cmakedefine GETFEM_PATCH_VERSION @GETFEM_PATCH_VERSION@

/* defined if quad-doubles are to be used instead of double-double */
#cmakedefine GETFEM_QDLIB_USE_QUAD @GETFEM_QDLIB_USE_QUAD@

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine GETFEM_STDC_HEADERS @GETFEM_STDC_HEADERS@

/* Use blas with 64 bits integers */
#cmakedefine GETFEM_USE_BLAS64_INTERFACE @GETFEM_USE_BLAS64_INTERFACE@

/* Use rpc for getfem communication with matlab */
#cmakedefine GETFEM_USE_RPC @GETFEM_USE_RPC@

/* Version number of package */
#cmakedefine GETFEM_VERSION "@GETFEM_VERSION@"
