#
# Copyright or © or Copr. Université de Bretagne-Sud, Lab-STICC, Bordeaux-INP, IMS Contributor(s) :
# Camille Monière (2022)
#
# camille.moniere@univ-ubs.fr, camille.moniere@ims-bordeaux.fr
#
# This software is a computer program whose purpose is to simulate and implement C-type to bytes
# FPGA IPs.
#
# This software is governed by the CeCILL-B license under French law and abiding by the rules of
# distribution of free software.  You can  use, modify and/ or redistribute the software under the
# terms of the CeCILL-B license as circulated by CEA, CNRS and INRIA at the following URL
# "http://www.cecill.info".
#
# As a counterpart to the access to the source code and  rights to copy, modify and redistribute
# granted by the license, users are provided only with a limited warranty  and the software's
# author,  the holder of the economic rights,  and the successive licensors  have only  limited
# liability.
#
# In this respect, the user's attention is drawn to the risks associated with loading,  using,
# modifying and/or developing or reproducing the software by the user in light of its specific
# status of free software, that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced professionals having
# in-depth computer knowledge. Users are therefore encouraged to load and test the software's
# suitability as regards their requirements in conditions enabling the security of their systems
# and/or data to be ensured and,  more generally, to use and operate it in the same conditions as
# regards security.
#
# The fact that you are presently reading this means that you have had knowledge of the CeCILL-B
# license and that you accept its terms.
#

cmake_minimum_required (VERSION 3.18.0 FATAL_ERROR)
# setting this is required

project (
  UartWrapperHLS
  LANGUAGES CXX
  VERSION 0.1
)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# ##################################################################################################
# Options
# ##################################################################################################

option (EXPORT_COMMANDS "export compile commands, for use with clangd for example." ON)

option (XILINX_COSIM "use TCL Vivado/Vitis HLS script to co-simulate the design." OFF)
if (XILINX_COSIM)
  option (XILINX_IP "use TCL Vivado/Vitis HLS script to generate an IP." OFF)
  if (XILINX_IP)
    option (XILINX_IMPL "use TCL Vivado/Vitis HLS script to run design implementation." OFF)
  endif ()
endif ()

option (ENABLE_TESTING "use CTest as a test suite." ON)

option (PEDANTIC "use -Wall and -pedantic." ON)

# ##################################################################################################
# Parameters
# ##################################################################################################

set (
  PRM_ITF_BLK
  "ap_ctrl_chain"
  CACHE STRING "Block-level interface protocol."
)
set_property (
  CACHE PRM_ITF_BLK
  PROPERTY STRINGS
           "ap_ctrl_none"
           "ap_ctrl_hs"
           "ap_ctrl_chain"
)
set (ITF_BLK ${PRM_ITF_BLK})

set (
  PRM_ITF_IN
  "axis"
  CACHE STRING "Input stream interface protocol."
)
set_property (
  CACHE PRM_ITF_IN
  PROPERTY STRINGS
           "ap_hs"
           "ap_fifo"
           "axis"
)
set (ITF_IN ${PRM_ITF_IN})

set (
  PRM_ITF_OUT
  "axis"
  CACHE STRING "Output stream interface protocol."
)
set_property (
  CACHE PRM_ITF_OUT
  PROPERTY STRINGS
           "ap_hs"
           "ap_fifo"
           "axis"
)
set (ITF_OUT ${PRM_ITF_OUT})

set (
  PRM_TYPE
  "uint32_t"
  CACHE STRING "Type to convert to/from."
)
set (TYPE ${PRM_TYPE})
set (
  PRM_SIGNED
  OFF
  CACHE BOOL "If Type is signed or not."
)
if (PRM_SIGNED)
  set (SIGNED "true")
else ()
  set (SIGNED "false")
endif ()

set (
  PRM_STRMLEN
  "1"
  CACHE STRING "Size of the TYPE stream."
)
set (STRMLEN ${PRM_STRMLEN})

set (
  PRM_ENDIAN
  "LE"
  CACHE STRING "Byte stream endianess."
)
set (ENDIAN ${PRM_ENDIAN})

# ##################################################################################################

if (PEDANTIC)
  add_compile_options (-Wall -pedantic)
endif ()
add_compile_options (-Wno-unused-label -Wno-unknown-pragmas)

if (EXPORT_COMMANDS)
  set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif ()

set (
  XILINX_HOME
  /opt/Xilinx
  CACHE PATH "path to Xilinx root folder."
)
set (
  XILINX_VER
  "2019.1"
  CACHE STRING "Xilinx software version to use."
)

if (XILINX_VER VERSION_GREATER_EQUAL "2020.1")
  set (
    AP_INCLUDE_DIR
    ${XILINX_HOME}/Vitis_HLS/${XILINX_VER}/include
    CACHE INTERNAL "Path to Xilinx includes" FORCE
  )
  find_program (
    XILINX_HLS vitis_hls HINTS ${XILINX_HOME}/Vitis_HLS/${XILINX_VER}/bin NO_CACHE REQUIRED
  )
else ()
  set (
    AP_INCLUDE_DIR
    ${XILINX_HOME}/Vivado/${XILINX_VER}/include
    CACHE INTERNAL "Path to Xilinx includes" FORCE
  )
  find_program (
    XILINX_HLS vivado_hls HINTS ${XILINX_HOME}/Vivado/${XILINX_VER}/bin NO_CACHE REQUIRED
  )
endif ()
message (STATUS "AP headers must lie under ${AP_INCLUDE_DIR}")

if ((NOT EXISTS ${AP_INCLUDE_DIR}/ap_int.h) OR (NOT EXISTS ${AP_INCLUDE_DIR}/ap_fixed.h))
  message (FATAL_ERROR "Arbitrary precision headers not found in ${AP_INCLUDE_DIR}.\n"
                       "Please provide a suitable path to the headers."
  )
endif ()

# ##################################################################################################

add_library (
  hls_uart_converters sources/CConverter/CConverterToBytes.cpp
                      sources/CConverter/CConverterFromBytes.cpp
)
target_include_directories (hls_uart_converters PUBLIC sources)
target_include_directories (hls_uart_converters SYSTEM PUBLIC ${AP_INCLUDE_DIR})

# ##################################################################################################
# TO BYTES    ######################################################################################
# ##################################################################################################

set (CFGIN_TO_TOP_CPP "sources/top_converters/type_strmlen_endian_to_bytes.cpp.in")
set (CFGOUT_TO_TOP_CPP
     "${CMAKE_CURRENT_SOURCE_DIR}/sources/top_converters/${TYPE}_${STRMLEN}_${ENDIAN}_to_bytes.cpp"
)
configure_file (${CFGIN_TO_TOP_CPP} ${CFGOUT_TO_TOP_CPP} @ONLY)

set (CFGIN_TO_TOP_HPP "sources/top_converters/type_strmlen_endian_to_bytes.hpp.in")
set (CFGOUT_TO_TOP_HPP
     "${CMAKE_CURRENT_SOURCE_DIR}/sources/top_converters/${TYPE}_${STRMLEN}_${ENDIAN}_to_bytes.hpp"
)
configure_file (${CFGIN_TO_TOP_HPP} ${CFGOUT_TO_TOP_HPP} @ONLY)

set (CFGIN_TO_TB_CPP "sources/top_converters/type_strmlen_endian_to_bytes_tb.cpp.in")
set (
  CFGOUT_TO_TB_CPP
  "${CMAKE_CURRENT_SOURCE_DIR}/sources/top_converters/${TYPE}_${STRMLEN}_${ENDIAN}_to_bytes_tb.cpp"
)
configure_file (${CFGIN_TO_TB_CPP} ${CFGOUT_TO_TB_CPP} @ONLY)

set (CFGIN_TO_TCL "hls_files/templates/script_type_strmlen_endian_to_bytes.tcl.in")
set (
  CFGOUT_TO_TCL
  "${CMAKE_CURRENT_SOURCE_DIR}/hls_files/generated/script_${TYPE}_${STRMLEN}_${ENDIAN}_to_bytes.tcl"
)
configure_file (${CFGIN_TO_TCL} ${CFGOUT_TO_TCL} @ONLY)

add_executable (
  hls_uart_${TYPE}_${STRMLEN}_${ENDIAN}_to_bytes_tb ${CFGOUT_TO_TOP_CPP} ${CFGOUT_TO_TB_CPP}
)
target_link_libraries (hls_uart_${TYPE}_${STRMLEN}_${ENDIAN}_to_bytes_tb hls_uart_converters)

# ##################################################################################################
# FROM BYTES  ######################################################################################
# ##################################################################################################

set (CFGIN_FROM_TOP_CPP "sources/top_converters/type_strmlen_endian_from_bytes.cpp.in")
set (
  CFGOUT_FROM_TOP_CPP
  "${CMAKE_CURRENT_SOURCE_DIR}/sources/top_converters/${TYPE}_${STRMLEN}_${ENDIAN}_from_bytes.cpp"
)
configure_file (${CFGIN_FROM_TOP_CPP} ${CFGOUT_FROM_TOP_CPP} @ONLY)

set (CFGIN_FROM_TOP_HPP "sources/top_converters/type_strmlen_endian_from_bytes.hpp.in")
set (
  CFGOUT_FROM_TOP_HPP
  "${CMAKE_CURRENT_SOURCE_DIR}/sources/top_converters/${TYPE}_${STRMLEN}_${ENDIAN}_from_bytes.hpp"
)
configure_file (${CFGIN_FROM_TOP_HPP} ${CFGOUT_FROM_TOP_HPP} @ONLY)

set (CFGIN_FROM_TB_CPP "sources/top_converters/type_strmlen_endian_from_bytes_tb.cpp.in")
set (
  CFGOUT_FROM_TB_CPP
  "${CMAKE_CURRENT_SOURCE_DIR}/sources/top_converters/${TYPE}_${STRMLEN}_${ENDIAN}_from_bytes_tb.cpp"
)
configure_file (${CFGIN_FROM_TB_CPP} ${CFGOUT_FROM_TB_CPP} @ONLY)

set (CFGIN_FROM_TCL "hls_files/templates/script_type_strmlen_endian_from_bytes.tcl.in")
set (
  CFGOUT_FROM_TCL
  "${CMAKE_CURRENT_SOURCE_DIR}/hls_files/generated/script_${TYPE}_${STRMLEN}_${ENDIAN}_from_bytes.tcl"
)
configure_file (${CFGIN_FROM_TCL} ${CFGOUT_FROM_TCL} @ONLY)

add_executable (
  hls_uart_${TYPE}_${STRMLEN}_${ENDIAN}_from_bytes_tb ${CFGOUT_FROM_TOP_CPP} ${CFGOUT_FROM_TB_CPP}
)
target_link_libraries (hls_uart_${TYPE}_${STRMLEN}_${ENDIAN}_from_bytes_tb hls_uart_converters)

if (ENABLE_TESTING)
  include (CTest)

  if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.19.0)
    cmake_policy (SET CMP0110 NEW)
  endif ()

  # find_file (INPUT_DAT_TB input.dat HINTS data NO_CACHE REQUIRED)
  if (CMAKE_VERSION VERSION_LESS 3.19.0)
    set (TEST_NAME_TO
         "\"Xilinx C-Simulation Testbench ${PROJECT_NAME} ${TYPE} ${STRMLEN} ${ENDIAN} TO\""
    )
  else ()
    set (TEST_NAME_TO
         "Xilinx C-Simulation Testbench ${PROJECT_NAME} ${TYPE} ${STRMLEN} ${ENDIAN} TO"
    )
  endif ()
  add_test (NAME ${TEST_NAME_TO} COMMAND hls_uart_${TYPE}_${STRMLEN}_${ENDIAN}_to_bytes_tb)

  if (CMAKE_VERSION VERSION_LESS 3.19.0)
    set (TEST_NAME_FROM
         "\"Xilinx C-Simulation Testbench ${PROJECT_NAME} ${TYPE} ${STRMLEN} ${ENDIAN} FROM\""
    )
  else ()
    set (TEST_NAME_FROM
         "Xilinx C-Simulation Testbench ${PROJECT_NAME} ${TYPE} ${STRMLEN} ${ENDIAN} FROM"
    )
  endif ()
  add_test (NAME ${TEST_NAME_FROM} COMMAND hls_uart_${TYPE}_${STRMLEN}_${ENDIAN}_from_bytes_tb)

  if (XILINX_COSIM)
    set (
      XILINX_TESTLINES
      "1000"
      CACHE STRING "Number of co-simulation passes"
    )

    if (XILINX_IP)
      set (
        EXPORT_IP
        "1"
        CACHE INTERNAL "EXPORT_IP"
      )
    else ()
      unset (EXPORT_IP CACHE)
    endif ()

    if (XILINX_IMPL)
      set (
        RUN_IMPL
        "1"
        CACHE INTERNAL "RUN_IMPL"
      )
    else ()
      unset (RUN_IMPL CACHE)
    endif ()

    add_custom_target (
      run_hls_to
      COMMAND ${XILINX_HLS} ${CFGOUT_TO_TCL} ${XILINX_TESTLINES} ${EXPORT_IP} ${RUN_IMPL}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/hls_files
      USES_TERMINAL
    )

    add_custom_target (
      run_hls_from
      COMMAND ${XILINX_HLS} ${CFGOUT_FROM_TCL} ${XILINX_TESTLINES} ${EXPORT_IP} ${RUN_IMPL}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/hls_files
      USES_TERMINAL
    )

    if (CMAKE_VERSION VERSION_LESS 3.19.0)
      set (TCL_TEST_NAME "\"Xilinx HLS TCL Flow TO\"")
    else ()
      set (TCL_TEST_NAME "Xilinx HLS TCL Flow TO")
    endif ()
    add_test (
      NAME ${TCL_TEST_NAME}
      COMMAND ${XILINX_HLS} ${CFGOUT_TO_TCL} ${XILINX_TESTLINES} 0 0
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/hls_files
    )
    set_property (TEST ${TCL_TEST_NAME} PROPERTY DISABLED TRUE)

    if (CMAKE_VERSION VERSION_LESS 3.19.0)
      set (TCL_TEST_NAME "\"Xilinx HLS TCL Flow FROM\"")
    else ()
      set (TCL_TEST_NAME "Xilinx HLS TCL Flow FROM")
    endif ()
    add_test (
      NAME ${TCL_TEST_NAME}
      COMMAND ${XILINX_HLS} ${CFGOUT_FROM_TCL} ${XILINX_TESTLINES} 0 0
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/hls_files
    )
    set_property (TEST ${TCL_TEST_NAME} PROPERTY DISABLED TRUE)
  endif ()
endif ()
