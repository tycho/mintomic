cmake_minimum_required(VERSION 2.8.5)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE INTERNAL "" FORCE)
project(MintomicTests)
option(MINT_HAS_EXCHANGE "Include tests for atomic exchange" OFF)

macro(GetFilesWithSourceGroups GLOB_TYPE VARIABLE RELATIVE_TO)
    file(${GLOB_TYPE} files ${ARGN})
    foreach(file ${files})
        file(RELATIVE_PATH relFile ${RELATIVE_TO} ${file})
        get_filename_component(folder ${relFile} PATH)
        string(REPLACE / \\ folder "${folder}")
        source_group("${folder}" FILES ${file})
    endforeach()
    list(APPEND ${VARIABLE} ${files})
endmacro()

#------------------------------------------
# Define all the tests
#------------------------------------------
function(AddMintomicTests template)
    set(INT_SIZES 32)
    set(STATUSES "_pass")
    set(THREAD_COUNTS 1)
    set(EXTRA_SUFFIX "")

    foreach(param IN ITEMS ${ARGN})
        string(FIND ${param} "=" sepIndex)
        if(NOT ${sepIndex} LESS 0)
            math(EXPR valueIndex "${sepIndex} + 1")
            string(SUBSTRING ${param} 0 ${sepIndex} key)
            string(SUBSTRING ${param} ${valueIndex} -1 value)
            set(TEST_${key} ${value})
            set(EXTRA_SUFFIX ${EXTRA_SUFFIX}_${value})
        elseif(param STREQUAL DUAL_THREAD)
            set(THREAD_COUNTS 2)
        elseif(param STREQUAL VAR_THREAD)
            set(THREAD_COUNTS 1 2 3 4)
        elseif(param STREQUAL VAR_SIZE)
            set(INT_SIZES 32 64)
        elseif(param STREQUAL ALLOW_FAIL)
            set(STATUSES _pass _fail)
        endif()
    endforeach()

    foreach(TEST_INT_BITSIZE IN ITEMS ${INT_SIZES})
        if(${TEST_INT_BITSIZE} EQUAL 32)
            set(TEST_INT_BITSHIFT 5)
        elseif(${TEST_INT_BITSIZE} EQUAL 64)
            set(TEST_INT_BITSHIFT 6)
        endif()
        foreach(status IN ITEMS ${STATUSES})
            if(${status} STREQUAL "_pass")
                set(status "")
                set(TEST_FORCE_FAIL false)
            else()
                set(TEST_FORCE_FAIL true)
            endif()

            set(TEST_FUNC test_${template}_${TEST_INT_BITSIZE}${status}${EXTRA_SUFFIX})
            configure_file(template.test_${template}.cpp generated/${TEST_FUNC}.cpp)

            set(TEST_ENTRY_FUNCS "${TEST_ENTRY_FUNCS}TestEntryFunc ${TEST_FUNC};\n")
            foreach(numThreads IN ITEMS ${THREAD_COUNTS})
                set(TEST_INFOS "${TEST_INFOS}    { \"${TEST_FUNC} x ${numThreads}\", ${TEST_FUNC}, ${numThreads}, ${TEST_FORCE_FAIL} },\n")
            endforeach()
        endforeach()
    endforeach()

    set(TEST_ENTRY_FUNCS ${TEST_ENTRY_FUNCS} PARENT_SCOPE)
    set(TEST_INFOS ${TEST_INFOS} PARENT_SCOPE)
endfunction()

set(TEST_ENTRY_FUNCS "")
set(TEST_INFOS "")
if (MINT_HAS_EXCHANGE)
    AddMintomicTests(exchange       VAR_THREAD VAR_SIZE ALLOW_FAIL)
endif()
AddMintomicTests(load_store     DUAL_THREAD ALLOW_FAIL VAR_SIZE)
AddMintomicTests(return_values  VAR_SIZE)
AddMintomicTests(increment      VAR_THREAD VAR_SIZE)
AddMintomicTests(add_triangle   VAR_THREAD VAR_SIZE)
AddMintomicTests(bitarray       VAR_THREAD VAR_SIZE DATA_SIZE=128)
AddMintomicTests(bitarray       VAR_THREAD VAR_SIZE DATA_SIZE=32768)
AddMintomicTests(mutex_acq_rel  VAR_THREAD VAR_SIZE ALLOW_FAIL)
if (NOT MSVC)
    AddMintomicTests(pthread_mutex  VAR_THREAD VAR_SIZE ALLOW_FAIL)
endif()
AddMintomicTests(storeload      DUAL_THREAD ALLOW_FAIL)
AddMintomicTests(linkedlist     VAR_THREAD VAR_SIZE)
configure_file(template.main.cpp generated/main.cpp)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "generated")

#------------------------------------------
# Configure the executable and all its dependencies
#------------------------------------------
GetFilesWithSourceGroups(GLOB_RECURSE PROJECT_FILES ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/generated/*.cpp ${CMAKE_CURRENT_BINARY_DIR}/generated/*.h)
GetFilesWithSourceGroups(GLOB TEMPLATE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR} *.cpp.in)
list(APPEND PROJECT_FILES ${TEMPLATE_SOURCES})

# Define executable target.
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.mycompany.\${PRODUCT_NAME:identifier}")
add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${PROJECT_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")

include(../cmake/BuildSettings.cmake)

# Add subdirectories with dependencies before dependents so that each subdirectory
# inherits the include paths gathered from the dependencies before it.
add_subdirectory(../src/mintomic mintomic)
add_subdirectory(../src/mintsystem mintsystem)
add_subdirectory(../src/mintpack mintpack)
include_directories(../include)

# Add libraries with dependencies after dependents to satisfy ld linker.
target_link_libraries(${PROJECT_NAME} MintPack)
target_link_libraries(${PROJECT_NAME} MintSystem)
get_target_property(TARGET_TYPE Mintomic TYPE)
if(${TARGET_TYPE} STREQUAL "UTILITY")
    # Nothing to link with in the Mintomic subdirectory
else()
    target_link_libraries(${PROJECT_NAME} Mintomic)
endif()
