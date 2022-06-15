/*
 *  Copyright or © or Copr. Université de Bretagne-Sud, Lab-STICC, Bordeaux-INP, IMS
 *  Contributor(s) : Camille Monière (2022)
 *
 *  camille.moniere@univ-ubs.fr, camille.moniere@ims-bordeaux.fr
 *
 *  This software is a computer program whose purpose is to simulate and implement C-type
 *  to bytes FPGA IPs.
 *
 *  This software is governed by the CeCILL-B license under French law and
 *  abiding by the rules of distribution of free software.  You can  use,
 *  modify and/ or redistribute the software under the terms of the CeCILL-B
 *  license as circulated by CEA, CNRS and INRIA at the following URL
 *  "http://www.cecill.info".
 *
 *  As a counterpart to the access to the source code and  rights to copy,
 *  modify and redistribute granted by the license, users are provided only
 *  with a limited warranty  and the software's author,  the holder of the
 *  economic rights,  and the successive licensors  have only  limited
 *  liability.
 *
 *  In this respect, the user's attention is drawn to the risks associated
 *  with loading,  using,  modifying and/or developing or reproducing the
 *  software by the user in light of its specific status of free software,
 *  that may mean  that it is complicated to manipulate,  and  that  also
 *  therefore means  that it is reserved for developers  and  experienced
 *  professionals having in-depth computer knowledge. Users are therefore
 *  encouraged to load and test the software's suitability as regards their
 *  requirements in conditions enabling the security of their systems and/or
 *  data to be ensured and,  more generally, to use and operate it in the
 *  same conditions as regards security.
 *
 *  The fact that you are presently reading this means that you have had
 *  knowledge of the CeCILL-B license and that you accept its terms.
 */

#ifndef _CONVERTER_TO_BYTES_HPP_
#define _CONVERTER_TO_BYTES_HPP_

#include <ap_int.h>
#include <cstdint>
#include <hls_stream.h>

template <uint32_t stream_len = 1, bool LittleEndian = true, bool use_sign = false>
class CConverterToBytes;

template <uint32_t stream_len>
class CConverterToBytes<stream_len, true, false> {
public:
    template <typename T, uint32_t sizeofT = sizeof(T)>
    static void process(hls::stream<T> & strm_in, hls::stream<uint8_t> & strm_out) {
#if !defined(XILINX_MAJOR)
        static_assert(std::numeric_limits<T>::is_integer, "T must be an integer C-type.");
        static_assert(std::is_unsigned<T>(), "T must be an unsigned type.");
#endif
    LOOP_STR_T2B_LE:
        for (uint32_t u = 0; u < stream_len; u++) {
            ap_uint<sizeofT * 8> word_buffer = strm_in.read();
        LOOP_BYTE_T2B_LE:
            for (uint32_t v = 0; v < sizeofT; v++) {
                const ap_uint<8> byte_buffer = word_buffer(7, 0);

                strm_out.write(byte_buffer);
                word_buffer >>= 8;
            }
        }
    }
};

template <uint32_t stream_len>
class CConverterToBytes<stream_len, false, false> {
public:
    template <typename T, uint32_t sizeofT = sizeof(T)>
    static void process(hls::stream<T> & strm_in, hls::stream<uint8_t> & strm_out) {
#if !defined(XILINX_MAJOR)
        static_assert(std::numeric_limits<T>::is_integer, "T must be an integer C-type.");
        static_assert(std::is_unsigned<T>(), "T must be an unsigned type.");
#endif
    LOOP_STR_T2B_BE:
        for (uint32_t u = 0; u < stream_len; u++) {
            ap_uint<sizeofT * 8> word_buffer = strm_in.read();
        LOOP_BYTE_T2B_BE:
            for (uint32_t v = 0; v < sizeofT; v++) {
                const ap_uint<8> byte_buffer = word_buffer(sizeofT * 8 - 1, (sizeofT - 1) * 8);

                strm_out.write(byte_buffer);
                word_buffer <<= 8;
            }
        }
    }
};

// SIGNED

template <uint32_t stream_len>
class CConverterToBytes<stream_len, true, true> {
public:
    template <typename T, uint32_t sizeofT = sizeof(T)>
    static void process(hls::stream<T> & strm_in, hls::stream<uint8_t> & strm_out) {
#if !defined(XILINX_MAJOR)
        static_assert(std::numeric_limits<T>::is_integer, "T must be an integer C-type.");
        static_assert(std::is_signed<T>(), "T must be an signed type.");
#endif
    LOOP_STR_T2B_LE:
        for (uint32_t u = 0; u < stream_len; u++) {
            ap_int<sizeofT * 8> word_buffer = strm_in.read();
        LOOP_BYTE_T2B_LE:
            for (uint32_t v = 0; v < sizeofT; v++) {
                const ap_uint<8> byte_buffer = word_buffer(7, 0);

                strm_out.write(byte_buffer);
                word_buffer >>= 8;
            }
        }
    }
};

template <uint32_t stream_len>
class CConverterToBytes<stream_len, false, true> {
public:
    template <typename T, uint32_t sizeofT = sizeof(T)>
    static void process(hls::stream<T> & strm_in, hls::stream<uint8_t> & strm_out) {
#if !defined(XILINX_MAJOR)
        static_assert(std::numeric_limits<T>::is_integer, "T must be an integer C-type.");
        static_assert(std::is_signed<T>(), "T must be an signed type.");
#endif
    LOOP_STR_T2B_BE:
        for (uint32_t u = 0; u < stream_len; u++) {
            ap_int<sizeofT * 8> word_buffer = strm_in.read();
        LOOP_BYTE_T2B_BE:
            for (uint32_t v = 0; v < sizeofT; v++) {
                const ap_uint<8> byte_buffer = word_buffer(sizeofT * 8 - 1, (sizeofT - 1) * 8);

                strm_out.write(byte_buffer);
                word_buffer <<= 8;
            }
        }
    }
};

#endif // _CONVERTER_TO_BYTES_HPP_
