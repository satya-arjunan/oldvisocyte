if (FALSE) # XXX(kitware): Visocyte handles versions.
cmake_minimum_required(VERSION 2.8)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  if (${CMAKE_MAJOR_VERSION} GREATER 2 OR ${CMAKE_MINOR_VERSION} GREATER 7)
    cmake_policy(SET CMP0015 OLD)
  endif (${CMAKE_MAJOR_VERSION} GREATER 2 OR ${CMAKE_MINOR_VERSION} GREATER 7)
endif(COMMAND cmake_policy)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING
  "one of: Release, Debug, RelWithDebInfo or MinSizeRel")
if (${CMAKE_MAJOR_VERSION} GREATER 2 OR ${CMAKE_MINOR_VERSION} GREATER 7)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release;Debug;RelWithDebInfo;MinSizeRel")
endif (${CMAKE_MAJOR_VERSION} GREATER 2 OR ${CMAKE_MINOR_VERSION} GREATER 7)
endif ()

project("cgns" C)
set(CGNS_VERSION "3.3")

if (FALSE) # XXX(kitware): Visocyte doesn't need CPack here.
# Allow for building a package
set(CPACK_PACKAGE_VERSION "${CGNS_VERSION}-develop")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "cgns-${CGNS_VERSION}")
set(CPACK_GENERATOR "STGZ;TGZ;ZIP")
set(CPACK_SOURCE_GENERATOR "STGZ;TGZ;ZIP")
include(CPack)

CONFIGURE_FILE(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
	"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif ()

if (FALSE) # XXX(kitware): CMake no longer does this.
# if this is cygwin, turn off win32 flag
if (CYGWIN)
  set(WIN32 "NO")
endif (CYGWIN)
endif ()

##############
# build mode #
##############

# get some type sizes
include(CheckTypeSize)
CHECK_TYPE_SIZE(long SIZE_OF_LONG)
if (FALSE) # XXX(kitware): Hardcoded settings.
CHECK_TYPE_SIZE(off_t SIZE_OF_OFF_T)

if ("${SIZE_OF_OFF_T}" LESS 8)
  option(CGNS_ENABLE_LFS "Enable or disable large file support" "OFF")
endif ("${SIZE_OF_OFF_T}" LESS 8)

if ("${CMAKE_SIZEOF_VOID_P}" GREATER 4)
  option(CGNS_ENABLE_64BIT "Enable or disable 64-bit code building" "OFF")
else ("${CMAKE_SIZEOF_VOID_P}" GREATER 4)
  set(CGNS_ENABLE_64BIT "OFF")
endif ("${CMAKE_SIZEOF_VOID_P}" GREATER 4)

option(CGNS_ENABLE_LEGACY "Enable or disable building legacy code (3.0 compatible)" "OFF")
option(CGNS_ENABLE_SCOPING "Enable or disable scoping of enumeration values" "OFF")
option(CGNS_ENABLE_BASE_SCOPE "Enable or disable base scoped families or connectivities" "OFF")
option(CGNS_ENABLE_MEM_DEBUG "Enable or disable memory debugging" "OFF")
else ()
set(CGNS_ENABLE_LFS ON)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CGNS_ENABLE_64BIT ON)
else ()
  set(CGNS_ENABLE_64BIT OFF)
endif ()
set(CGNS_ENABLE_LEGACY OFF)
set(CGNS_ENABLE_SCOPING ON)
set(CGNS_ENABLE_BASE_SCOPE OFF)
set(CGNS_ENABLE_MEM_DEBUG OFF)
endif ()

if (CGNS_ENABLE_LEGACY)
  set(CGNS_ENABLE_64BIT "OFF")
  mark_as_advanced(FORCE CGNS_ENABLE_64BIT)
else (CGNS_ENABLE_LEGACY)
  mark_as_advanced(CLEAR CGNS_ENABLE_64BIT)
endif (CGNS_ENABLE_LEGACY)

if (CGNS_ENABLE_64BIT)
  set(CGNS_ENABLE_LEGACY "OFF")
  mark_as_advanced(FORCE CGNS_ENABLE_LEGACY)
else (CGNS_ENABLE_64BIT)
  mark_as_advanced(CLEAR CGNS_ENABLE_LEGACY)
endif (CGNS_ENABLE_64BIT)

if (CGNS_ENABLE_LFS)
  if (WIN32)
    # Are these functions always available on windows that we don't need to
    # check?
    set(HAVE_OPEN64 0)
    set(HAVE_LSEEK64 1)
  else (WIN32)
    include(CheckSymbolExists)
    check_symbol_exists(open64 "sys/types.h;sys/stat.h;unistd.h" HAVE_OPEN64)
    check_symbol_exists(lseek64 "sys/types.h;unistd.h" HAVE_LSEEK64)
  endif (WIN32)
  if (HAVE_OPEN64)
    add_definitions(-DHAVE_OPEN64)
  endif (HAVE_OPEN64)
  if (HAVE_LSEEK64)
    add_definitions(-DHAVE_LSEEK64)
  endif (HAVE_LSEEK64)
else (CGNS_ENABLE_LFS)
  remove_definitions(-DHAVE_OPEN64 -DHAVE_LSEEK64)
endif (CGNS_ENABLE_LFS)

# XXX(kitware): Hardcoded for Visocyte.
if (FALSE)
option(CGNS_ENABLE_TESTS "Enable test programs" "OFF")
else ()
set(CGNS_ENABLE_TESTS OFF)
endif ()
if (CGNS_ENABLE_TESTS)
  enable_testing()
endif (CGNS_ENABLE_TESTS)

###########
# Fortran #
###########

if (FALSE) # XXX(kitware): Hardcoded settings.
# Allow fortran support to be toggled
option(CGNS_ENABLE_FORTRAN "Enable or disable the use of Fortran" "OFF")
if (NOT DEFINED FORTRAN_NAMING)
  set(FORTRAN_NAMING "UNKNOWN")
endif (NOT DEFINED FORTRAN_NAMING)
else ()
set(CGNS_ENABLE_FORTRAN OFF)
endif ()

# If we are using fortran, enable it and configure the C-Fortran interface
# It would be nice to automatically detect this setting

if(CGNS_ENABLE_FORTRAN)

    enable_language("Fortran" OPTIONAL)

#-----------------------------------------------------------------------------
# Detect name mangling convention used between Fortran and C
#-----------------------------------------------------------------------------

  set(F2CLIST  "LOWERCASE" "LOWERCASE_" "LOWERCASE__" "UPPERCASE" "UPPERCASE_" "UPPERCASE__")
  set(FORTRAN_NAMING_HELP
  "Configures how to link the Fortran components into the C library.
  Options are:
  	  LOWERCASE
	  LOWERCASE_
	  LOWERCASE__
	  UPPERCASE
	  UPPERRCASE_
	  UPPERRCASE__
  LOWERCASE_ is known to work with gfortran.  If this setting
  does not work with your compiler, try the others until one does.
  ")

  include(FortranCInterface)
  set(FortranCInterface_GLOBAL_SYMBOLS mysub mysub_ mysub__ MYSUB MYSUB_ MYSUB__)
  FortranCInterface_HEADER(${CMAKE_BINARY_DIR}/FCMangle.h 
  			   MACRO_NAMESPACE "CGNS_FC_"
  			   SYMBOL_NAMESPACE "CGNS_FC_")

  file (STRINGS ${CMAKE_BINARY_DIR}/FCMangle.h CONTENTS REGEX "CGNS_FC_GLOBAL\\(.*,.*\\) +(.*)")
  string (REGEX MATCH "CGNS_FC_GLOBAL\\(.*,.*\\) +(.*)" RESULT ${CONTENTS})
  set (CGNS_FC_FUNC "${CMAKE_MATCH_1}")

  file(REMOVE ${CMAKE_BINARY_DIR}/FCMangle.h)

  if(CGNS_FC_FUNC MATCHES "^NAME(.*)")
     set (FORTRAN_NAMING "UPPERCASE")
  elseif(CGNS_FC_FUNC MATCHES "^name(.*)")
     set (FORTRAN_NAMING "LOWERCASE")
  endif()
  if(CGNS_FC_FUNC MATCHES "(.*)__$")
     set (FORTRAN_NAMING "${FORTRAN_NAMING}__")
  elseif(CGNS_FC_FUNC MATCHES "(.*)_$")
     set (FORTRAN_NAMING "${FORTRAN_NAMING}_")
  endif()

  message(STATUS "Fortran name mangling convention: ${FORTRAN_NAMING}")

# check that FORTRAN_NAMING is valid
  if (FORTRAN_NAMING STREQUAL "UNKNOWN")
    if (WIN32)
      set(FORTRAN_NAMING "UPPERCASE" CACHE STRING ${FORTRAN_NAMING_HELP})
    else (WIN32)
      set(FORTRAN_NAMING "LOWERCASE_" CACHE STRING ${FORTRAN_NAMING_HELP})
    endif (WIN32)
    if (${CMAKE_MAJOR_VERSION} GREATER 2 OR ${CMAKE_MINOR_VERSION} GREATER 7)
      set_property(CACHE FORTRAN_NAMING PROPERTY STRINGS ${F2CLIST})
    endif (${CMAKE_MAJOR_VERSION} GREATER 2 OR ${CMAKE_MINOR_VERSION} GREATER 7)
  else (FORTRAN_NAMING STREQUAL "UNKNOWN")
    list(FIND F2CLIST "${FORTRAN_NAMING}" ListIndex)
    if (ListIndex LESS 0)
      message(SEND_ERROR "invalid FORTRAN_NAMING value. Must be empty or one of:
        LOWERCASE
        LOWERCASE_
        LOWERCASE__
        UPPERCASE
        UPPERRCASE_
        UPPERRCASE__")
    endif (ListIndex LESS 0)
  endif (FORTRAN_NAMING STREQUAL "UNKNOWN")
endif (CGNS_ENABLE_FORTRAN)

# this forces cmake to set up the required Fortran variables
if (CGNS_ENABLE_FORTRAN AND HAS_FORTRAN)
  enable_language("Fortran")
endif (CGNS_ENABLE_FORTRAN AND HAS_FORTRAN)

########
# HDF5 #
########

if (FALSE) # XXX(kitware): Hardcoded settings.
option(CGNS_ENABLE_HDF5 "Enable or disable HDF5 interface" "OFF")
else ()
set(CGNS_ENABLE_HDF5 ON)
endif ()

if (FALSE) # XXX(kitware): Adapted for Visocyte.
if (CGNS_ENABLE_HDF5)

  if (CGNS_BUILD_SHARED)
    set (FIND_HDF_COMPONENTS C shared)
  else (CGNS_BUILD_SHARED)
    set (FIND_HDF_COMPONENTS C static)
  endif (CGNS_BUILD_SHARED)
  message (STATUS "HDF5 find comps: ${FIND_HDF_COMPONENTS}")

  #set (SEARCH_PACKAGE_NAME ${HDF5_PACKAGE_NAME})
  set (SEARCH_PACKAGE_NAME "hdf5")

  find_package (HDF5 NAMES ${SEARCH_PACKAGE_NAME} COMPONENTS ${FIND_HDF_COMPONENTS})
  message (STATUS "HDF5 C libs:${HDF5_FOUND} static:${HDF5_static_C_FOUND} and shared:${HDF5_shared_C_FOUND}")
  if (HDF5_FOUND)
    add_executable (h5dump IMPORTED)
    if (NOT HDF5_static_C_FOUND AND NOT HDF5_shared_C_FOUND)
      set (FIND_HDF_COMPONENTS C)

      find_package (HDF5 NAMES ${SEARCH_PACKAGE_NAME} COMPONENTS ${FIND_HDF_COMPONENTS})
      message (STATUS "HDF5 libs:${HDF5_FOUND} C:${HDF5_C_FOUND}")
      set (LINK_LIBS ${LINK_LIBS} ${HDF5_LIBRARIES})
      if (HDF5_BUILD_SHARED_LIBS)
        add_definitions (-DH5_BUILT_AS_DYNAMIC_LIB)
      else (HDF5_BUILD_SHARED_LIBS)
        add_definitions (-DH5_BUILT_AS_STATIC_LIB)
      endif (HDF5_BUILD_SHARED_LIBS)
      if (BUILD_SHARED_LIBS AND WIN32)
        set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dumpdll")
      else (BUILD_SHARED_LIBS AND WIN32)
        set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump")
      endif (BUILD_SHARED_LIBS AND WIN32)
    else (NOT HDF5_static_C_FOUND AND NOT HDF5_shared_C_FOUND)
      if (BUILD_SHARED_LIBS AND HDF5_shared_C_FOUND)
        set (LINK_LIBS ${LINK_LIBS} ${HDF5_C_SHARED_LIBRARY})
      else (HDF5_static_C_FOUND)
        set (LINK_LIBS ${LINK_LIBS} ${HDF5_C_STATIC_LIBRARY})
      endif (BUILD_SHARED_LIBS AND HDF5_shared_C_FOUND)
      set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump")
    endif (NOT HDF5_static_C_FOUND AND NOT HDF5_shared_C_FOUND)
    set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:h5dump>)

    set (HDF5_HAVE_H5PUBCONF_H 1)
    set (HDF5_HAVE_HDF5 1)
    set (HDF5_HDF5_HEADER "h5pubconf.h")
    set (HDF5_INCLUDE_DIR_GEN ${HDF5_INCLUDE_DIR})
    message (STATUS "HDF5-${HDF5_VERSION_STRING} found: INC=${HDF5_INCLUDE_DIR} TOOLS=${HDF5_TOOLS_DIR}")
  else (HDF5_FOUND)
    find_package (HDF5) # Legacy find
#Legacy find_package does not set HDF5_TOOLS_DIR, so we set it here
    set(HDF5_TOOLS_DIR ${HDF5_LIBRARY_DIRS}/../bin)
#Legacy find_package does not set HDF5_BUILD_SHARED_LIBS, so we set it here
    if (CGNS_BUILD_SHARED AND EXISTS "${HDF5_LIBRARY_DIRS}/libhdf5${CMAKE_SHARED_LIBRARY_SUFFIX}")
      set (HDF5_BUILD_SHARED_LIBS 1)
      add_definitions (-DH5_BUILT_AS_DYNAMIC_LIB)
    else ()
      set (HDF5_BUILD_SHARED_LIBS 0)
      add_definitions (-DH5_BUILT_AS_STATIC_LIB)
    endif ()
    set (LINK_LIBS ${LINK_LIBS} ${HDF5_LIBRARIES})

    add_executable (h5dump IMPORTED)
    set_property (TARGET h5dump PROPERTY IMPORTED_LOCATION "${HDF5_TOOLS_DIR}/h5dump")
    set (HDF5_DUMP_EXECUTABLE $<TARGET_FILE:h5dump>)
  endif (HDF5_FOUND)
  set (HDF5_PACKAGE_NAME ${SEARCH_PACKAGE_NAME})

  if (HDF5_FOUND)
    set (HDF5_HAVE_H5PUBCONF_H 1)
    set (HDF5_HAVE_HDF5 1)
    set (HDF5_HDF5_HEADER "h5pubconf.h")
    set (HDF5_INCLUDE_DIR_GEN ${HDF5_INCLUDE_DIR})
    message (STATUS "HDF5-${HDF5_VERSION_STRING} found: INC=${HDF5_INCLUDE_DIR} TOOLS=${HDF5_TOOLS_DIR} HDF5_BUILD_SHARED_LIBS=${HDF5_BUILD_SHARED_LIBS}")
    INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIR})
    set (HDF5_LIBRARY ${LINK_LIBS})
    message (STATUS "HDF5 link libs: ${LINK_LIBS}")
  else (HDF5_FOUND)
    message (FATAL_ERROR " HDF5 was specified but was not found")
  endif (HDF5_FOUND)

  set(HDF5_NEED_ZLIB "OFF" CACHE BOOL "Does the HDF5 library require linking to zlib?")
  if(HDF5_NEED_ZLIB)
    find_library(ZLIB_LIBRARY z)
    mark_as_advanced(CLEAR ZLIB_LIBRARY)
  else (HDF5_NEED_ZLIB)
    mark_as_advanced(FORCE ZLIB_LIBRARY)
  endif(HDF5_NEED_ZLIB)

  set(HDF5_NEED_SZIP "OFF" CACHE BOOL "Does the HDF5 library require linking to zlib?")
  if (HDF5_NEED_SZIP)
    find_library(SZIP_LIBRARY szip)
    mark_as_advanced(CLEAR SZIP_LIBRARY)
  else (HDF5_NEED_SZIP)
    mark_as_advanced(FORCE SZIP_LIBRARY)
  endif (HDF5_NEED_SZIP)

  set(HDF5_NEED_MPI "OFF" CACHE BOOL "Does the HDF5 library require linking to mpi? (Only true if using parallel HDF5)")
  set(MPI_INC)
  set(MPI_LIBS)
  if (HDF5_NEED_MPI)
    if (NOT MPI_FOUND)
      include(FindMPI)
    endif (NOT MPI_FOUND)
    # MPI variable names changed with cmake 2.8.5
    if (DEFINED MPI_C_COMPILER)
      mark_as_advanced(CLEAR MPI_C_COMPILER MPI_C_INCLUDE_PATH MPI_C_LIBRARIES)
      mark_as_advanced(FORCE MPI_LIBRARY MPI_EXTRA_LIBRARY)
      set(MPI_INC ${MPI_C_INCLUDE_PATH})
      set(MPI_LIBS ${MPI_C_LIBRARIES})
    else (DEFINED MPI_C_COMPILER)
      mark_as_advanced(CLEAR MPI_COMPILER MPI_INCLUDE_PATH MPI_LIBRARY MPI_EXTRA_LIBRARY)
      set(MPI_INC ${MPI_INCLUDE_PATH})
      if (MPI_EXTRA_LIBRARY)
        set(MPI_LIBS ${MPI_LIBRARY} ${MPI_EXTRA_LIBRARY})
      else (MPI_EXTRA_LIBRARY)
        set(MPI_LIBS ${MPI_LIBRARY})
      endif (MPI_EXTRA_LIBRARY)
    endif (DEFINED MPI_C_COMPILER)
    mark_as_advanced(CLEAR MPIEXEC)
  else (HDF5_NEED_MPI)
    mark_as_advanced(FORCE MPI_C_COMPILER MPI_C_INCLUDE_PATH MPI_C_LIBRARIES)
    mark_as_advanced(FORCE MPI_COMPILER MPI_INCLUDE_PATH MPI_LIBRARY MPI_EXTRA_LIBRARY MPIEXEC)
  endif (HDF5_NEED_MPI)
  mark_as_advanced(CLEAR HDF5_NEED_ZLIB HDF5_NEED_SZIP HDF5_NEED_MPI)
else (CGNS_ENABLE_HDF5)
  mark_as_advanced(FORCE HDF5_NEED_ZLIB HDF5_NEED_SZIP HDF5_NEED_MPI)
  mark_as_advanced(FORCE ZLIB_LIBRARY SZIP_LIBRARY)
  mark_as_advanced(FORCE MPI_C_COMPILER MPI_C_INCLUDE_PATH MPI_C_LIBRARIES)
  mark_as_advanced(FORCE MPI_COMPILER MPI_INCLUDE_PATH MPI_LIBRARY MPI_EXTRA_LIBRARY MPIEXEC)
endif (CGNS_ENABLE_HDF5)
else ()
if (VTK_MODULE_USE_EXTERNAL_VTK_hdf5)
  # TODO: Implement.
  set(H5_HAVE_PARALLEL OFF)
else ()
  set(H5_HAVE_PARALLEL OFF)
endif ()
endif ()

#################
# parallel cgns #
#################

if (FALSE) # XXX(kitware): Adapted for Visocyte.
if (CGNS_ENABLE_HDF5 AND HDF5_NEED_MPI)
  set(CGNS_ENABLE_PARALLEL "OFF" CACHE BOOL "Enable or disable parallel interface ?")
  mark_as_advanced(CLEAR CGNS_ENABLE_PARALLEL)
  # Check that HDF5 has parallel support
  if (NOT (HDF5_IS_PARALLEL OR HDF5_ENABLE_PARALLEL))
      message(FATAL_ERROR "HDF5 has been found, but is missing parallel support.")
  endif()
else (CGNS_ENABLE_HDF5 AND HDF5_NEED_MPI)
  set(CGNS_ENABLE_PARALLEL "OFF")
  mark_as_advanced(FORCE CGNS_ENABLE_PARALLEL)
endif (CGNS_ENABLE_HDF5 AND HDF5_NEED_MPI)
else ()
set(CGNS_ENABLE_PARALLEL ${H5_HAVE_PARALLEL})
endif ()

####################
# RPATH Management #
####################

if (FALSE) # XXX(kitware): Visocyte handles rpaths.
# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# the RPATH to be used when installing
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif ()

#-----------------------------------------------------------------------------
# Dashboard and Testing Settings
#-----------------------------------------------------------------------------
if (FALSE) # XXX(kitware): Hardcoded settings.
option (CGNS_BUILD_TESTING "Build CGNS Testing" OFF)
else ()
set(CGNS_BUILD_TESTING OFF)
endif ()
if (CGNS_BUILD_TESTING)
  set (DART_TESTING_TIMEOUT 1200
      CACHE INTEGER
      "Timeout in seconds for each test (default 1200=20minutes)"
  )
  enable_testing ()
  include (CTest)
  include (${PROJECT_SOURCE_DIR}/CTestConfig.cmake)
endif (CGNS_BUILD_TESTING)



########
# CGNS #
########

# Include the src directory
add_subdirectory(src)

