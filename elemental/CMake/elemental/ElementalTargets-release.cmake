#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "pmrrr" for configuration "Release"
set_property(TARGET pmrrr APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(pmrrr PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "/home/alchemist/install/openmpi/lib/libmpi.so;/usr/lib/libopenblas.so;/usr/lib/gcc/x86_64-redhat-linux/4.8.5/libgfortran.so;-lpthread;/usr/lib64/libm.so"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libpmrrr.so.88-dev"
  IMPORTED_SONAME_RELEASE "libpmrrr.so.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS pmrrr )
list(APPEND _IMPORT_CHECK_FILES_FOR_pmrrr "${_IMPORT_PREFIX}/lib64/libpmrrr.so.88-dev" )

# Import target "ElSuiteSparse" for configuration "Release"
set_property(TARGET ElSuiteSparse APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ElSuiteSparse PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libElSuiteSparse.so.88-dev"
  IMPORTED_SONAME_RELEASE "libElSuiteSparse.so.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS ElSuiteSparse )
list(APPEND _IMPORT_CHECK_FILES_FOR_ElSuiteSparse "${_IMPORT_PREFIX}/lib64/libElSuiteSparse.so.88-dev" )

# Import target "El" for configuration "Release"
set_property(TARGET El APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(El PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "pmrrr;ElSuiteSparse;/home/alchemist/install/elemental/lib/libparmetis.so;/home/alchemist/install/elemental/lib/libmetis.so;/usr/lib/libopenblas.so;/usr/lib/gcc/x86_64-redhat-linux/4.8.5/libgfortran.so;-lpthread;/usr/lib64/libm.so;/home/alchemist/install/openmpi/lib/libmpi.so"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib64/libEl.so.88-dev"
  IMPORTED_SONAME_RELEASE "libEl.so.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS El )
list(APPEND _IMPORT_CHECK_FILES_FOR_El "${_IMPORT_PREFIX}/lib64/libEl.so.88-dev" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
