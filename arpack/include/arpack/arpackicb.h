! arpackicb.h must be included only by F77/F90, not by C/C++.

#define INTERFACE64 0

! i_int stands for iso_c_binding int
#if INTERFACE64
#define i_int c_int64_t
#else
#define i_int c_int
#endif
