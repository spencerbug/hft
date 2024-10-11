cmake_minimum_required(VERSION 3.13)

# Set the project name
project(HFT_Application
    VERSION 1.0
    DESCRIPTION "High-Frequency Trading Applicationg using CME MDP3.0 SBE, Globex, NASDAQ ITCH and OUCH protocols for arbitrage"
    LANGUAGES CXX
)

if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Release types: Debug, Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ===============================================
# Add Simple Binary Encoding Subdirectory
# To call CMakeLists.txt in simple-binary-encoding
# ===============================================
add_subdirectory(simple-binary-encoding)

# ===============================================
# Directory Path Definitions
# Define paths for SBE XML templates and generated C++ code
# ===============================================
set(SBE_TEMPLATES_DIR "${CMAKE_SOURCE_DIR}/sbetemplates")
set(GENERATED_DIR "${CMAKE_SOURCE_DIR}/generated")
set(SBE_HOME_DIR "${CMAKE_SOURCE_DIR}/simple-binary-encoding")


# ===============================================
# Retrieve SBE Version
# ===============================================
# Read the version from simple-bindary-encoding's version.txt
file(STRINGS "${SBE_HOME_DIR}/version.txt" SBE_VERSION_TXT LIMIT_COUNT 1 REGEX "^[0-9]+(\\.[0-9]+)+")


# Define a path to the SBE JAR built by simple-binary-encoding
set(SBE_JAR "${SBE_HOME_DIR}/sbe-all/build/libs/sbe-all-${SBE_VERSION_TXT}.jar")


# ===============================================
# Find Java Executable
# ===============================================
find_package(Java REQUIRED)
if (NOT Java_FOUND)
    message(FATAL_ERROR "Java not found")
endif()
# Ensure Java version is 17 or higher
execute_process(
    COMMAND ${Java_JAVA_EXECUTABLE} -version
    OUTPUT_VARIABLE JAVA_VERSION_OUTPUT
    ERROR_VARIABLE JAVA_VERSION_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" JAVA_VERSION_MATCH "${JAVA_VERSION_OUTPUT}")
if (JAVA_VERSION_MATCH)
    string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\1" JAVA_VERSION_MAJOR "${JAVA_VERSION_MATCH}")
    if (JAVA_VERSION_MAJOR LESS 17)
        message(FATAL_ERROR "Java version 17 or higher is required. Found version: ${JAVA_VERSION_OUTPUT}")
    endif()
else()
    message(FATAL_ERROR "Failed to determine Java version from output: ${JAVA_VERSION_OUTPUT}")
endif()


# ===============================================
# Function to Find Immediate Subdirectories (Namespaces)
# ===============================================
function(find_immediate_subdirs result_var parent_dir)
    file(GLOB children RELATIVE ${parent_dir} ${parent_dir}/*)
    set(subdirs "")
    foreach(child ${children})
        if(IS_DIRECTORY "${parent_dir}/${child}")
            list(APPEND subdirs "${child}")
        endif()
    endforeach()
    set(${result_var} "${subdirs}" PARENT_SCOPE)
endfunction()

# ===============================================
# Find All Namespace Directories
# ===============================================
find_immediate_subdirs(NAMESPACE_DIRS "${SBE_TEMPLATES_DIR}")

# Check if any namespaces are found
if(NOT NAMESPACE_DIRS)
    message(FATAL_ERROR "No namespace directories found in ${SBE_TEMPLATES_DIR}")
endif()


# Initialize lists to hold generated sources
set(ALL_GENERATED_SRC)
set(ALL_GENERATED_HDR)

# ===============================================
# Build the SBE JAR with Gradlew
# ===============================================
# Check if we've run gradle to build the SBE JAR
if(NOT EXISTS ${SBE_JAR})
    message(STATUS "SBE JAR not found at ${SBE_JAR}. Running gradle in simple-binary-encoding to build the JAR...")
    execute_process(
        COMMAND ./gradlew
        WORKING_DIRECTORY ${SBE_HOME_DIR}
        RESULT_VARIABLE RESULT
        ERROR_VARIABLE ERROR_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR "Gradlew failed: ${ERROR_OUTPUT}")
    endif()
endif()

# ===============================================
# Iterate Over Each Namespace and Generate Code
# ===============================================
# Ensure the generated directory exists
file(MAKE_DIRECTORY ${GENERATED_DIR})
foreach(NAMESPACE ${NAMESPACE_DIRS})
    message(STATUS "Processing namespace: ${NAMESPACE}")
    # Define paths
    set (NAMESPACE_TEMPLATE_DIR "${SBE_TEMPLATES_DIR}/${NAMESPACE}")
    set (NAMESPACE_GENERATED_DIR "${GENERATED_DIR}/${NAMESPACE}")

    # Find all XML templates in the current namespace
    file(GLOB NAMESPACE_XML_FILES "${NAMESPACE_TEMPLATE_DIR}/*.xml")

    # Check if any XML files are found
    if(NOT NAMESPACE_XML_FILES)
        message(WARNING "No XML templates found in ${NAMESPACE_TEMPLATE_DIR}. Skipping namespace ${NAMESPACE}.")
        continue()
    endif()

    # For each XML file found, run the SBE compiler
    foreach(XML_FILE ${NAMESPACE_XML_FILES})
        message(STATUS "Generating code for: ${XML_FILE}")
        execute_process(
            COMMAND ${Java_JAVA_EXECUTABLE} -Dsbe.generated.ir=true -Dsbe.target.language=Cpp -Dsbe.target.namespace=${NAMESPACE} -Dsbe.output.dir=${GENERATED_DIR} -Dsbe.errorLog=yes -jar ${SBE_JAR} ${XML_FILE}
            RESULT_VARIABLE RESULT
            OUTPUT_VARIABLE OUTPUT
            ERROR_VARIABLE ERROR_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )

        if(NOT RESULT EQUAL 0)
            message(FATAL_ERROR "Code generation failed for ${XML_FILE} with error: ${ERROR_OUTPUT}")
        endif()
    endforeach()
endforeach()

# Now collect generated source and headers
file(GLOB_RECURSE ALL_GENERATED_SRC "${NAMESPACE_GENERATED_DIR}/*.cpp")
file(GLOB_RECURSE ALL_GENERATED_HDR "${NAMESPACE_GENERATED_DIR}/*.h")


# Create the library
add_library(sbe_generated STATIC ${ALL_GENERATED_SRC} ${ALL_GENERATED_HDR})
set_target_properties(sbe_generated PROPERTIES LINKER_LANGUAGE CXX)
# Set the output directory for the build
set_target_properties(sbe_generated PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Include directories for generated code and Simple Binary Encoding
target_include_directories(sbe_generated PUBLIC
    ${GENERATED_DIR}
)


# ===============================================
# Add Source Directories
# ===============================================
add_subdirectory(source/sbe_client)
add_subdirectory(source/sbe_server)


# ===============================================
# Add Test Directory
# ===============================================
enable_testing()
add_subdirectory(test)