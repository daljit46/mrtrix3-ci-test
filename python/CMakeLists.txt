set(PYTHON_VERSION_FILE ${PROJECT_BINARY_DIR}/lib/mrtrix3/_version.py)

find_package(Git QUIET)

# We generate the version file at configure time,
# so tools like Pylint can run without building the project
execute_process(
    COMMAND ${CMAKE_COMMAND} 
    -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
    -D MRTRIX_BASE_VERSION=${MRTRIX_BASE_VERSION}
    -D DST=${CMAKE_CURRENT_SOURCE_DIR}/lib/mrtrix3/_version.py
    -D SRC=${CMAKE_CURRENT_SOURCE_DIR}/lib/mrtrix3/_version.py.in
    -P ${PROJECT_SOURCE_DIR}/cmake/FindVersion.cmake
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(copy-python-binary-scripts ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin ${PROJECT_BINARY_DIR}/bin
    COMMENT "Copying python binary scripts to build directory"
    VERBATIM
)

add_custom_target(copy-python-lib-scripts ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lib ${PROJECT_BINARY_DIR}/lib
    COMMENT "Copying python lib scripts to build directory"
    VERBATIM
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PATTERN "*.py.in" EXCLUDE # Exclude version template file
)

install(FILES ${PYTHON_VERSION_FILE}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/mrtrix3
)