set(Elemental_INCLUDE_DIRS "/home/alchemist/install/elemental/include")
set(Elemental_INCLUDE_DIRS "${Elemental_INCLUDE_DIRS};/home/alchemist/install/openmpi/include")
IF(FALSE)
   set(Elemental_INCLUDE_DIRS "${Elemental_INCLUDE_DIRS};QD_INCLUDES-NOTFOUND")
ENDIF()
IF()
   set(Elemental_INCLUDE_DIRS "${Elemental_INCLUDE_DIRS};")
   set(Elemental_INCLUDE_DIRS "${Elemental_INCLUDE_DIRS};")
   set(Elemental_INCLUDE_DIRS "${Elemental_INCLUDE_DIRS};GMP_INCLUDES-NOTFOUND")
ENDIF()
set(Elemental_INCLUDE_DIRS
  "${Elemental_INCLUDE_DIRS};")

set(Elemental_COMPILE_FLAGS "-O3 -fcx-fortran-rules -std=gnu++14 ")
set(Elemental_LINK_FLAGS "-Wl,-rpath -Wl,/home/alchemist/install/openmpi/lib -Wl,--enable-new-dtags -pthread")

set(Elemental_DEFINITIONS "")

# Our library dependencies (contains definitions for IMPORTED targets)
include("${CMAKE_CURRENT_LIST_DIR}/ElementalTargets.cmake")

set(Elemental_LIBRARIES El)
