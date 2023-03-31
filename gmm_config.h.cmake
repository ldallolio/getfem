/* cmakeconfig.h.in (can be manually modified)  */
/* Define to dummy `main' function (if any) required to link to the Fortran
   libraries. */
#cmakedefine GMM_FC_DUMMY_MAIN @GMM_FC_DUMMY_MAIN@

/* Define if F77 and FC dummy `main' functions are identical. */
#cmakedefine GMM_FC_DUMMY_MAIN_EQ_F77 @GMM_FC_DUMMY_MAIN_EQ_F77@

/* enable openblas to be multithreaded */
#cmakedefine GMM_FORCE_SINGLE_THREAD_BLAS @GMM_FORCE_SINGLE_THREAD_BLAS@

/* glibc backtrace function */
#cmakedefine GMM_HAVE_BACKTRACE @GMM_HAVE_BACKTRACE@

/* the use of the blas call for basic algebra routines. */
#cmakedefine GMM_HAVE_BLAS_INTERFACE @GMM_HAVE_BLAS_INTERFACE@

/* Define to 1 if you have the <cmumps_c.h> header file. */
#cmakedefine GMM_HAVE_CMUMPS_C_H @GMM_HAVE_CMUMPS_C_H@

/* Define to 1 if you have the <cxxabi.h> header file. */
#cmakedefine GMM_HAVE_CXXABI_H @GMM_HAVE_CXXABI_H@

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine GMM_HAVE_DLFCN_H @GMM_HAVE_DLFCN_H@

/* Define to 1 if you have the <dmumps_c.h> header file. */
#cmakedefine GMM_HAVE_DMUMPS_C_H @GMM_HAVE_DMUMPS_C_H@

/* glibc floating point exceptions control */
#cmakedefine GMM_HAVE_FEENABLEEXCEPT @GMM_HAVE_FEENABLEEXCEPT@

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine GMM_HAVE_INTTYPES_H @GMM_HAVE_INTTYPES_H@

/* Define to 1 if you have the `mpi_cxx' library (-lmpi_cxx). */
#cmakedefine GMM_HAVE_LIBMPI_CXX @GMM_HAVE_LIBMPI_CXX@

/* Define to 1 if you have the <libqhull_r/qhull_ra.h> header file. */
#cmakedefine GMM_HAVE_LIBQHULL_R_QHULL_RA_H @GMM_HAVE_LIBQHULL_R_QHULL_RA_H@

/* Define to 1 if you have the `superlu' library (-lsuperlu). */
#cmakedefine GMM_HAVE_LIBSUPERLU @GMM_HAVE_LIBSUPERLU@

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine GMM_HAVE_MEMORY_H @GMM_HAVE_MEMORY_H@

/* defined if the Metis library was found and is working */
#cmakedefine GMM_HAVE_METIS @GMM_HAVE_METIS@

/* Define to 1 if you have the <metis.h> header file. */
#cmakedefine GMM_HAVE_METIS_H @GMM_HAVE_METIS_H@

/* defined if the Metis library found is older than version 4 */
#cmakedefine GMM_HAVE_METIS_OLD_API @GMM_HAVE_METIS_OLD_API@

/* Define if you have the MPI library. */
#cmakedefine GMM_HAVE_MPI @GMM_HAVE_MPI@

/* gcc style __PRETTY_FUNCTION__ macro */
#cmakedefine GMM_HAVE_PRETTY_FUNCTION @GMM_HAVE_PRETTY_FUNCTION@

/* If available, contains the Python version number currently in use. */
#cmakedefine GMM_HAVE_PYTHON @GMM_HAVE_PYTHON@

/* defined if the qd library was found and is working */
#cmakedefine GMM_HAVE_QDLIB @GMM_HAVE_QDLIB@

/* Defined to 1 if Scilab is present on the system */
#cmakedefine GMM_HAVE_SCILAB @GMM_HAVE_SCILAB@

/* Define to 1 if you have the <smumps_c.h> header file. */
#cmakedefine GMM_HAVE_SMUMPS_C_H @GMM_HAVE_SMUMPS_C_H@

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine GMM_HAVE_STDINT_H @GMM_HAVE_STDINT_H@

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine GMM_HAVE_STDLIB_H @GMM_HAVE_STDLIB_H@

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine GMM_HAVE_STRINGS_H @GMM_HAVE_STRINGS_H@

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine GMM_HAVE_STRING_H @GMM_HAVE_STRING_H@

/* Define to 1 to on/off SuperLU support. */
#cmakedefine GMM_HAVE_SUPERLU @GMM_HAVE_SUPERLU@

/* Define to 1 if you have the <superlu/colamd.h> header file. */
#cmakedefine GMM_HAVE_SUPERLU_COLAMD_H @GMM_HAVE_SUPERLU_COLAMD_H@

/* Define to 1 if you have the <superlu/slu_cdefs.h> header file. */
#cmakedefine GMM_HAVE_SUPERLU_SLU_CDEFS_H @GMM_HAVE_SUPERLU_SLU_CDEFS_H@

/* Define to 1 if you have the <superlu/slu_Cnames.h> header file. */
#cmakedefine GMM_HAVE_SUPERLU_SLU_CNAMES_H @GMM_HAVE_SUPERLU_SLU_CNAMES_H@

/* Define to 1 if you have the <superlu/slu_dcomplex.h> header file. */
#cmakedefine GMM_HAVE_SUPERLU_SLU_DCOMPLEX_H @GMM_HAVE_SUPERLU_SLU_DCOMPLEX_H@

/* Define to 1 if you have the <superlu/slu_ddefs.h> header file. */
#cmakedefine GMM_HAVE_SUPERLU_SLU_DDEFS_H @GMM_HAVE_SUPERLU_SLU_DDEFS_H@

/* Define to 1 if you have the <superlu/slu_scomplex.h> header file. */
#cmakedefine GMM_HAVE_SUPERLU_SLU_SCOMPLEX_H @GMM_HAVE_SUPERLU_SLU_SCOMPLEX_H@

/* Define to 1 if you have the <superlu/slu_sdefs.h> header file. */
#cmakedefine GMM_HAVE_SUPERLU_SLU_SDEFS_H @GMM_HAVE_SUPERLU_SLU_SDEFS_H@

/* Define to 1 if you have the <superlu/slu_zdefs.h> header file. */
#cmakedefine GMM_HAVE_SUPERLU_SLU_ZDEFS_H @GMM_HAVE_SUPERLU_SLU_ZDEFS_H@

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine GMM_HAVE_SYS_STAT_H @GMM_HAVE_SYS_STAT_H@

/* Define to 1 if you have the <sys/times.h> header file. */
#cmakedefine GMM_HAVE_SYS_TIMES_H @GMM_HAVE_SYS_TIMES_H@

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine GMM_HAVE_SYS_TYPES_H @GMM_HAVE_SYS_TYPES_H@

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine GMM_HAVE_UNISTD_H @GMM_HAVE_UNISTD_H@

/* Define to 1 if you have the <zmumps_c.h> header file. */
#cmakedefine GMM_HAVE_ZMUMPS_C_H @GMM_HAVE_ZMUMPS_C_H@

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#cmakedefine GMM_LT_OBJDIR @GMM_LT_OBJDIR@

/* Major version number */
#cmakedefine GMM_MAJOR_VERSION @GMM_MAJOR_VERSION@

/* Minor version number */
#cmakedefine GMM_MINOR_VERSION @GMM_MINOR_VERSION@

/* Name of package */
#cmakedefine GMM_PACKAGE @GMM_PACKAGE@

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine GMM_PACKAGE_BUGREPORT @GMM_PACKAGE_BUGREPORT@

/* Define to the full name of this package. */
#cmakedefine GMM_PACKAGE_NAME @GMM_PACKAGE_NAME@

/* Define to the full name and version of this package. */
#cmakedefine GMM_PACKAGE_STRING @GMM_PACKAGE_STRING@

/* Define to the one symbol short name of this package. */
#cmakedefine GMM_PACKAGE_TARNAME @GMM_PACKAGE_TARNAME@

/* Define to the home page for this package. */
#cmakedefine GMM_PACKAGE_URL @GMM_PACKAGE_URL@

/* Define to the version of this package. */
#cmakedefine GMM_PACKAGE_VERSION @GMM_PACKAGE_VERSION@

/* Patch version number */
#cmakedefine GMM_PATCH_VERSION @GMM_PATCH_VERSION@

/* defined if quad-doubles are to be used instead of double-double */
#cmakedefine GMM_QDLIB_USE_QUAD @GMM_QDLIB_USE_QUAD@

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine GMM_STDC_HEADERS @GMM_STDC_HEADERS@

/* Use blas with 64 bits integers */
#cmakedefine GMM_USE_BLAS64_INTERFACE @GMM_USE_BLAS64_INTERFACE@

/* Use rpc for getfem communication with matlab */
#cmakedefine GMM_USE_RPC @GMM_USE_RPC@

/* Version number of package */
#cmakedefine GMM_VERSION "@GMM_VERSION@"
