function(assert_variable ARG_VARIABLE ARG_MESSAGE)
  if(NOT DEFINED ${ARG_VARIABLE})
    message(FATAL_ERROR ${ARG_MESSAGE})
  endif()
endfunction()

function(nuget_download ARG_VERSION ARG_DOWNLOAD_PATH)
  set(URL "https://dist.nuget.org/win-x86-commandline/${ARG_VERSION}/nuget.exe")
  file(DOWNLOAD ${URL} "${ARG_DOWNLOAD_PATH}/nuget.exe")
  message(STATUS "[ NuGet ] Downloaded ${URL} to ${ARG_DOWNLOAD_PATH}")
endfunction()

function(nuget_init)
  set(flagArgs SKIP_DOWNLOAD)
  set(oneValueArgs VERSION DOWNLOAD_PATH)
  set(multiValueArgs)
  cmake_parse_arguments(
      NUGET
      "${flagArgs}"
      "${oneValueArgs}"
      "${multiValueArgs}"
      ${ARGN})

  # Use the latest version if no NUGET_VERSION is specified
  if(NOT DEFINED NUGET_VERSION)
    set(NUGET_VERSION "latest")
  endif()

  assert_variable(NUGET_VERSION "You must specify the NUGET_VERSION")
  assert_variable(
      NUGET_DOWNLOAD_PATH "You must specify the NUGET_DOWNLOAD_PATH")
  set(ENV{NUGET_DOWNLOAD_PATH} ${NUGET_DOWNLOAD_PATH})
  set(ENV{NUGET_BINARY_PATH} ${NUGET_DOWNLOAD_PATH}/nuget.exe)

  if (NUGET_SKIP_DOWNLOAD)
    message(STATUS "[ NuGet ] Skipping the NuGet.exe download.")
  elseif(NOT EXISTS $ENV{NUGET_BINARY_PATH})
    nuget_download(${NUGET_VERSION} ${NUGET_DOWNLOAD_PATH})
  endif()
endfunction()


function(nuget_sources_add
    ARG_NUGET_SOURCE_NAME
    ARG_NUGET_PUBLISH_URL)
  execute_process(COMMAND "nuget.exe" sources list
    WORKING_DIRECTORY $ENV{NUGET_DOWNLOAD_PATH}
    OUTPUT_VARIABLE NUGET_SOURCES_LIST)
  string(FIND "${NUGET_SOURCES_LIST}" "${ARG_NUGET_SOURCE_NAME}" found_index)
  if (NOT ${found_index} EQUAL -1)
    execute_process(COMMAND
        "nuget.exe" sources remove -name ${ARG_NUGET_SOURCE_NAME}
        WORKING_DIRECTORY $ENV{NUGET_DOWNLOAD_PATH}
        OUTPUT_VARIABLE NUGET_REMOVE_SOURCE)
  endif()
  execute_process(COMMAND
      nuget.exe sources Add
      -Name ${ARG_NUGET_SOURCE_NAME}
      -Source ${ARG_NUGET_PUBLISH_URL}
      WORKING_DIRECTORY $ENV{NUGET_DOWNLOAD_PATH}
      OUTPUT_VARIABLE NUGET_ADD_SOURCE)
  message(STATUS
      "[ NuGet ] Added NuGet Source ${ARG_NUGET_SOURCE_NAME}@${ARG_NUGET_PUBLISH_URL}")
endfunction()


macro(set_parent_variable NAME VALUE)
  set(${ARGV0} ${ARGV1} PARENT_SCOPE)
endmacro()


function(nuget)
  list(APPEND NUGET_SUPPORTED_SUBCOMMANDS "install")
  set(flagArgs SKIP_PRERELEASE)
  set(oneValueArgs
      SUBCOMMAND
      COPY_RESOURCES
      DOWNLOAD_PATH
      SOURCE_NAME
      PUBLISH_URL
      INCLUDE_DIR
      PACKAGE
      PACKAGE_VERSION
      POST_BUILD
      POST_INSTALL)
  set(multiValueArgs)
  cmake_parse_arguments(
      NUGET
      "${flagArgs}"
      "${oneValueArgs}"
      "${multiValueArgs}"
      ${ARGN})
  if (NOT NUGET_SUBCOMMAND IN_LIST NUGET_SUPPORTED_SUBCOMMANDS)
    message(FATAL_ERROR "[ NuGet ] Unsupported ${NUGET_SUBCOMMAND} sub-command")
  endif()
  nuget_init(DOWNLOAD_PATH ${NUGET_DOWNLOAD_PATH})
  nuget_sources_add(
      ${NUGET_SOURCE_NAME} ${NUGET_PUBLISH_URL})

  set(NUGET_BINARY_PATH $ENV{NUGET_BINARY_PATH})
  execute_process(COMMAND
      "${NUGET_BINARY_PATH}" install ${NUGET_PACKAGE}
      -Source ${NUGET_SOURCE_NAME}
      -PreRelease
      -Version ${NUGET_PACKAGE_VERSION}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      RESULT_VARIABLE ret_code
      ERROR_VARIABLE NUGET_INSTALL_ERROR_OUTPUT
      OUTPUT_VARIABLE NUGET_INSTALL_OUTPUT)
  set(NUGET_PACKAGE_INSTALL_ROOT
      ${CMAKE_BINARY_DIR}/${NUGET_PACKAGE}.${NUGET_PACKAGE_VERSION})
  set(NUGET_PACKAGE_INSTALL_ROOT ${NUGET_PACKAGE_INSTALL_ROOT} PARENT_SCOPE)

  string(REPLACE "\n" ";" LINES ${NUGET_INSTALL_OUTPUT})
  foreach(LINE ${LINES})
    string(FIND "${LINE}" "Successfully installed" installed_index)
    string(FIND "${LINE}" "already installed" already_installed_index)
    if(NOT ${installed_index} EQUAL -1 OR NOT ${already_installed_index} EQUAL -1)
      message(STATUS "[ NuGet ] ${LINE}")
    endif()
  endforeach()

  if (DEFINED NUGET_POST_INSTALL)
    set(POST_INSTALL_SCRIPT ${NUGET_PACKAGE_INSTALL_ROOT}/${NUGET_POST_INSTALL})
    execute_process(COMMAND
        "cmd.exe" /c ${POST_INSTALL_SCRIPT}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE POST_INSTALL_SCRIPT_OUTPUT)
    message(STATUS "[ NuGet ] Finished running post install script: ${POST_INSTALL_SCRIPT}")
  endif()

  if (DEFINED NUGET_POST_BUILD)
    set(POST_BUILD_SCRIPT ${NUGET_PACKAGE_INSTALL_ROOT}/${NUGET_POST_BUILD})
    set_parent_variable(${NUGET_COPY_RESOURCES} ${POST_BUILD_SCRIPT})
  endif()
endfunction()