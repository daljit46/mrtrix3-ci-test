# A function that adds a bash test for each line in a given file
function(add_bash_tests)
    set(singleValueArgs FILE_PATH PREFIX WORKING_DIRECTORY)
    set(multiValueArgs EXEC_DIRECTORIES)
    cmake_parse_arguments(
        ARG
        ""
        "${singleValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    set(file_path ${ARG_FILE_PATH})
    set(prefix ${ARG_PREFIX})
    set(working_directory ${ARG_WORKING_DIRECTORY})
    set(exec_directories ${ARG_EXEC_DIRECTORIES})

    # In a MINGW environment, the paths used in a bash command must be prefixed with /c
    if(MINGW)
        string(REPLACE REGEX "C:" "/c" exec_directories "${exec_directories}")
    endif()

    get_filename_component(file_name ${file_path} NAME_WE)

    # Add a custom target for IDEs to pickup the test script
    add_custom_target(test_${file_name} SOURCES ${file_path})

    # Add test that cleans up temporary files
    add_test(
        NAME ${prefix}_${file_name}_cleanup
        COMMAND ${BASH} -c "rm -rf ${working_directory}/tmp* ${working_directory}/*-tmp-*"
        WORKING_DIRECTORY ${working_directory}
    )
    set_tests_properties(${prefix}_${file_name}_cleanup PROPERTIES FIXTURES_SETUP ${file_name}_cleanup)

    file(STRINGS ${file_path} FILE_CONTENTS)
    list(LENGTH FILE_CONTENTS FILE_CONTENTS_LENGTH)
    math(EXPR MAX_LINE_NUM "${FILE_CONTENTS_LENGTH} - 1")


    set(EXEC_DIR_PATHS "${exec_directories}")
    string(REPLACE ";" ":" EXEC_DIR_PATHS "${EXEC_DIR_PATHS}")
    message(STATUS "Add bash tests for ${file_name} with exec directories: ${EXEC_DIR_PATHS}")

    foreach(line_num RANGE ${MAX_LINE_NUM})
        list(GET FILE_CONTENTS ${line_num} line)

        set(test_name ${file_name}) 
        if(${FILE_CONTENTS_LENGTH} GREATER 1)
            math(EXPR suffix "${line_num} + 1")
            set(test_name "${file_name}_${suffix}")
        endif()

        add_test(
            NAME ${prefix}_${test_name}
            COMMAND ${BASH} -c "export PATH=${EXEC_DIR_PATHS}:$PATH;${line}"
            WORKING_DIRECTORY ${working_directory}
        )
        set_tests_properties(${prefix}_${test_name} 
            PROPERTIES FIXTURES_REQUIRED ${file_name}_cleanup
        )
        message(VERBOSE "Add bash tests commands for ${file_name}: ${line}")
    endforeach()
endfunction()