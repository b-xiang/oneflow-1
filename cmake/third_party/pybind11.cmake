include (ExternalProject)

set(PYTHON_EXECUTABLE /bin/python)
set(PYTHON_INCLUDE_DIRS /usr/include/python2.7)
set(NUMPY_INCLUDE_DIRS /usr/lib64/python2.7/site-packages/numpy/core/include)
set(PYTHON_LIBDIR /lib64/python2.7/config/libpython2.7.so)

set(PYBIND11_INCLUDE_DIR ${THIRD_PARTY_DIR}/pybind11/include)
set(PYBIND11_INSTALL_DIR ${THIRD_PARTY_DIR}/pybind11)
set(pybind11_URL https://github.com/pybind/pybind11.git)
set(pybind11_TAG 8edc147d67ca85a93ed1f53628004528dc36a04d)

add_definitions(-DEIGEN_NO_AUTOMATIC_RESIZING -DEIGEN_NO_MALLOC -DEIGEN_USE_GPU)

if (BUILD_THIRD_PARTY)

ExternalProject_Add(pybind11
    PREFIX pybind11
    GIT_REPOSITORY ${pybind11_URL}
    GIT_TAG ${pybind11_TAG}
    UPDATE_COMMAND ""
    INSTALL_DIR "${PYBIND11_INSTALL_DIR}"
    CMAKE_CACHE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
        -DCMAKE_INSTALL_PREFIX:STRING=${PYBIND11_INSTALL_DIR}
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
        -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
        -DBUILD_TESTING:BOOL=OFF
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:STRING=${PYBIND11_INSTALL_DIR}
        -DPYTHON_EXECUTABLE:STRING=${PYTHON_EXECUTABLE}
)

endif(BUILD_THIRD_PARTY)
