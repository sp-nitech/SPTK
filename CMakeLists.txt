# ------------------------------------------------------------------------ #
# Copyright 2021 SPTK Working Group                                        #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ------------------------------------------------------------------------ #

cmake_minimum_required(VERSION 3.1)

project(SPTK)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.5.0)
    message(FATAL_ERROR "require clang >= 3.5.0")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8.5)
    message(FATAL_ERROR "require gcc >= 4.8.5")
  endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0)
    message(FATAL_ERROR "require msvc >= Visual Studio 2015")
  endif()
endif()

set(CMAKE_CXX_STANDARD 11)
if (MSVC)
  set(CMAKE_CXX_FLAGS "/O2 /W4 /wd4244 /wd4305 /wd4996 /EHsc")
else()
  set(CMAKE_CXX_FLAGS "-O2 -Wall -Wno-deprecated-register")
endif()

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)

set(CC_SOURCES
  ${THIRD_PARTY_DIR}/Getopt/getoptwin.cc
  ${THIRD_PARTY_DIR}/REAPER/core/file_resource.cc
  ${THIRD_PARTY_DIR}/REAPER/core/float_matrix.cc
  ${THIRD_PARTY_DIR}/REAPER/core/track.cc
  ${THIRD_PARTY_DIR}/REAPER/epoch_tracker/epoch_tracker.cc
  ${THIRD_PARTY_DIR}/REAPER/epoch_tracker/fd_filter.cc
  ${THIRD_PARTY_DIR}/REAPER/epoch_tracker/fft_reaper.cc
  ${THIRD_PARTY_DIR}/REAPER/epoch_tracker/lpc_analyzer.cc
  ${THIRD_PARTY_DIR}/REAPER/wave/codec_riff.cc
  ${THIRD_PARTY_DIR}/REAPER/wave/wave.cc
  ${THIRD_PARTY_DIR}/REAPER/wave/wave_io.cc
  ${THIRD_PARTY_DIR}/SWIPE/swipe.cc
  ${THIRD_PARTY_DIR}/SWIPE/vector.cc
  ${THIRD_PARTY_DIR}/Snack/jkGetF0.cc
  ${THIRD_PARTY_DIR}/Snack/sigproc.cc
  ${THIRD_PARTY_DIR}/WORLD/common.cc
  ${THIRD_PARTY_DIR}/WORLD/dio.cc
  ${THIRD_PARTY_DIR}/WORLD/fft_world.cc
  ${THIRD_PARTY_DIR}/WORLD/matlabfunctions.cc
  ${SOURCE_DIR}/analysis/adaptive_generalized_cepstral_analysis.cc
  ${SOURCE_DIR}/analysis/adaptive_mel_cepstral_analysis.cc
  ${SOURCE_DIR}/analysis/adaptive_mel_generalized_cepstral_analysis.cc
  ${SOURCE_DIR}/analysis/autocorrelation_analysis.cc
  ${SOURCE_DIR}/analysis/fast_fourier_transform_cepstral_analysis.cc
  ${SOURCE_DIR}/analysis/mel_cepstral_analysis.cc
  ${SOURCE_DIR}/analysis/mel_filter_bank_analysis.cc
  ${SOURCE_DIR}/analysis/mel_frequency_cepstral_coefficients_analysis.cc
  ${SOURCE_DIR}/analysis/mel_generalized_cepstral_analysis.cc
  ${SOURCE_DIR}/analysis/pitch_extraction.cc
  ${SOURCE_DIR}/analysis/pitch_extraction_by_rapt.cc
  ${SOURCE_DIR}/analysis/pitch_extraction_by_reaper.cc
  ${SOURCE_DIR}/analysis/pitch_extraction_by_swipe.cc
  ${SOURCE_DIR}/analysis/pitch_extraction_by_world.cc
  ${SOURCE_DIR}/analysis/second_order_all_pass_mel_cepstral_analysis.cc
  ${SOURCE_DIR}/analysis/zero_crossing_analysis.cc
  ${SOURCE_DIR}/check/line_spectral_pairs_stability_check.cc
  ${SOURCE_DIR}/check/linear_predictive_coefficients_stability_check.cc
  ${SOURCE_DIR}/check/mlsa_digital_filter_stability_check.cc
  ${SOURCE_DIR}/compression/huffman_coding.cc
  ${SOURCE_DIR}/compression/huffman_decoding.cc
  ${SOURCE_DIR}/compression/huffman_encoding.cc
  ${SOURCE_DIR}/compression/inverse_multistage_vector_quantization.cc
  ${SOURCE_DIR}/compression/inverse_uniform_quantization.cc
  ${SOURCE_DIR}/compression/inverse_vector_quantization.cc
  ${SOURCE_DIR}/compression/linde_buzo_gray_algorithm.cc
  ${SOURCE_DIR}/compression/mu_law_compression.cc
  ${SOURCE_DIR}/compression/mu_law_expansion.cc
  ${SOURCE_DIR}/compression/multistage_vector_quantization.cc
  ${SOURCE_DIR}/compression/uniform_quantization.cc
  ${SOURCE_DIR}/compression/vector_quantization.cc
  ${SOURCE_DIR}/conversion/all_pole_to_all_zero_digital_filter_coefficients.cc
  ${SOURCE_DIR}/conversion/autocorrelation_to_composite_sinusoidal_modeling.cc
  ${SOURCE_DIR}/conversion/cepstrum_to_autocorrelation.cc
  ${SOURCE_DIR}/conversion/cepstrum_to_minimum_phase_impulse_response.cc
  ${SOURCE_DIR}/conversion/cepstrum_to_negative_derivative_of_phase_spectrum.cc
  ${SOURCE_DIR}/conversion/composite_sinusoidal_modeling_to_autocorrelation.cc
  ${SOURCE_DIR}/conversion/filter_coefficients_to_group_delay.cc
  ${SOURCE_DIR}/conversion/filter_coefficients_to_phase_spectrum.cc
  ${SOURCE_DIR}/conversion/filter_coefficients_to_spectrum.cc
  ${SOURCE_DIR}/conversion/generalized_cepstrum_gain_normalization.cc
  ${SOURCE_DIR}/conversion/generalized_cepstrum_inverse_gain_normalization.cc
  ${SOURCE_DIR}/conversion/line_spectral_pairs_to_linear_predictive_coefficients.cc
  ${SOURCE_DIR}/conversion/linear_predictive_coefficients_to_cepstrum.cc
  ${SOURCE_DIR}/conversion/linear_predictive_coefficients_to_line_spectral_pairs.cc
  ${SOURCE_DIR}/conversion/linear_predictive_coefficients_to_parcor_coefficients.cc
  ${SOURCE_DIR}/conversion/log_area_ratio_to_parcor_coefficients.cc
  ${SOURCE_DIR}/conversion/mel_cepstrum_to_mlsa_digital_filter_coefficients.cc
  ${SOURCE_DIR}/conversion/mel_generalized_cepstrum_to_mel_generalized_cepstrum.cc
  ${SOURCE_DIR}/conversion/mel_generalized_cepstrum_to_spectrum.cc
  ${SOURCE_DIR}/conversion/mel_generalized_line_spectral_pairs_to_spectrum.cc
  ${SOURCE_DIR}/conversion/minimum_phase_impulse_response_to_cepstrum.cc
  ${SOURCE_DIR}/conversion/mlsa_digital_filter_coefficients_to_mel_cepstrum.cc
  ${SOURCE_DIR}/conversion/negative_derivative_of_phase_spectrum_to_cepstrum.cc
  ${SOURCE_DIR}/conversion/parcor_coefficients_to_linear_predictive_coefficients.cc
  ${SOURCE_DIR}/conversion/parcor_coefficients_to_log_area_ratio.cc
  ${SOURCE_DIR}/conversion/spectrum_to_autocorrelation.cc
  ${SOURCE_DIR}/conversion/spectrum_to_spectrum.cc
  ${SOURCE_DIR}/conversion/waveform_to_autocorrelation.cc
  ${SOURCE_DIR}/conversion/waveform_to_spectrum.cc
  ${SOURCE_DIR}/filter/all_pole_digital_filter.cc
  ${SOURCE_DIR}/filter/all_pole_lattice_digital_filter.cc
  ${SOURCE_DIR}/filter/all_zero_digital_filter.cc
  ${SOURCE_DIR}/filter/infinite_impulse_response_digital_filter.cc
  ${SOURCE_DIR}/filter/inverse_mglsa_digital_filter.cc
  ${SOURCE_DIR}/filter/inverse_pseudo_quadrature_mirror_filter_banks.cc
  ${SOURCE_DIR}/filter/line_spectral_pairs_digital_filter.cc
  ${SOURCE_DIR}/filter/median_filter.cc
  ${SOURCE_DIR}/filter/mglsa_digital_filter.cc
  ${SOURCE_DIR}/filter/mlsa_digital_filter.cc
  ${SOURCE_DIR}/filter/pseudo_quadrature_mirror_filter_banks.cc
  ${SOURCE_DIR}/filter/second_order_digital_filter.cc
  ${SOURCE_DIR}/generation/delta_calculation.cc
  ${SOURCE_DIR}/generation/excitation_generation.cc
  ${SOURCE_DIR}/generation/m_sequence_generation.cc
  ${SOURCE_DIR}/generation/nonrecursive_maximum_likelihood_parameter_generation.cc
  ${SOURCE_DIR}/generation/normal_distributed_random_value_generation.cc
  ${SOURCE_DIR}/generation/recursive_maximum_likelihood_parameter_generation.cc
  ${SOURCE_DIR}/input/input_source_delay.cc
  ${SOURCE_DIR}/input/input_source_filling_magic_number.cc
  ${SOURCE_DIR}/input/input_source_from_array.cc
  ${SOURCE_DIR}/input/input_source_from_matrix.cc
  ${SOURCE_DIR}/input/input_source_from_stream.cc
  ${SOURCE_DIR}/input/input_source_from_vector.cc
  ${SOURCE_DIR}/input/input_source_interpolation.cc
  ${SOURCE_DIR}/input/input_source_interpolation_with_magic_number.cc
  ${SOURCE_DIR}/input/input_source_preprocessing_for_filter_gain.cc
  ${SOURCE_DIR}/math/discrete_cosine_transform.cc
  ${SOURCE_DIR}/math/discrete_fourier_transform.cc
  ${SOURCE_DIR}/math/distance_calculation.cc
  ${SOURCE_DIR}/math/durand_kerner_method.cc
  ${SOURCE_DIR}/math/dynamic_time_warping.cc
  ${SOURCE_DIR}/math/entropy_calculation.cc
  ${SOURCE_DIR}/math/fast_fourier_transform.cc
  ${SOURCE_DIR}/math/fourier_transform.cc
  ${SOURCE_DIR}/math/frequency_transform.cc
  ${SOURCE_DIR}/math/gaussian_mixture_model_based_conversion.cc
  ${SOURCE_DIR}/math/gaussian_mixture_modeling.cc
  ${SOURCE_DIR}/math/histogram_calculation.cc
  ${SOURCE_DIR}/math/inverse_discrete_cosine_transform.cc
  ${SOURCE_DIR}/math/inverse_fast_fourier_transform.cc
  ${SOURCE_DIR}/math/levinson_durbin_recursion.cc
  ${SOURCE_DIR}/math/matrix.cc
  ${SOURCE_DIR}/math/matrix2d.cc
  ${SOURCE_DIR}/math/minmax_accumulation.cc
  ${SOURCE_DIR}/math/principal_component_analysis.cc
  ${SOURCE_DIR}/math/real_valued_fast_fourier_transform.cc
  ${SOURCE_DIR}/math/real_valued_inverse_fast_fourier_transform.cc
  ${SOURCE_DIR}/math/reverse_levinson_durbin_recursion.cc
  ${SOURCE_DIR}/math/scalar_operation.cc
  ${SOURCE_DIR}/math/second_order_all_pass_frequency_transform.cc
  ${SOURCE_DIR}/math/second_order_all_pass_inverse_frequency_transform.cc
  ${SOURCE_DIR}/math/statistics_accumulation.cc
  ${SOURCE_DIR}/math/symmetric_matrix.cc
  ${SOURCE_DIR}/math/symmetric_system_solver.cc
  ${SOURCE_DIR}/math/toeplitz_plus_hankel_system_solver.cc
  ${SOURCE_DIR}/math/two_dimensional_fast_fourier_transform.cc
  ${SOURCE_DIR}/math/two_dimensional_inverse_fast_fourier_transform.cc
  ${SOURCE_DIR}/math/two_dimensional_real_valued_fast_fourier_transform.cc
  ${SOURCE_DIR}/math/vandermonde_system_solver.cc
  ${SOURCE_DIR}/utils/data_symmetrizing.cc
  ${SOURCE_DIR}/utils/mel_cepstrum_postfiltering.cc
  ${SOURCE_DIR}/utils/misc_utils.cc
  ${SOURCE_DIR}/utils/sptk_utils.cc
  ${SOURCE_DIR}/window/chebyshev_window.cc
  ${SOURCE_DIR}/window/cosine_window.cc
  ${SOURCE_DIR}/window/data_windowing.cc
  ${SOURCE_DIR}/window/kaiser_window.cc
  ${SOURCE_DIR}/window/standard_window.cc
)

add_library(sptk STATIC ${CC_SOURCES})
target_include_directories(sptk PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${THIRD_PARTY_DIR}
  ${THIRD_PARTY_DIR}/REAPER
  ${THIRD_PARTY_DIR}/WORLD
  )
install(TARGETS sptk
  ARCHIVE DESTINATION lib
  )
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/SPTK
  DESTINATION include
  )

set(MAIN_SOURCES
  ${SOURCE_DIR}/main/acorr.cc
  ${SOURCE_DIR}/main/acr2csm.cc
  ${SOURCE_DIR}/main/aeq.cc
  ${SOURCE_DIR}/main/amgcep.cc
  ${SOURCE_DIR}/main/average.cc
  ${SOURCE_DIR}/main/b2mc.cc
  ${SOURCE_DIR}/main/bcp.cc
  ${SOURCE_DIR}/main/bcut.cc
  ${SOURCE_DIR}/main/c2acr.cc
  ${SOURCE_DIR}/main/c2mpir.cc
  ${SOURCE_DIR}/main/c2ndps.cc
  ${SOURCE_DIR}/main/cdist.cc
  ${SOURCE_DIR}/main/clip.cc
  ${SOURCE_DIR}/main/csm2acr.cc
  ${SOURCE_DIR}/main/dct.cc
  ${SOURCE_DIR}/main/decimate.cc
  ${SOURCE_DIR}/main/delay.cc
  ${SOURCE_DIR}/main/delta.cc
  ${SOURCE_DIR}/main/dequantize.cc
  ${SOURCE_DIR}/main/df2.cc
  ${SOURCE_DIR}/main/dfs.cc
  ${SOURCE_DIR}/main/dmp.cc
  ${SOURCE_DIR}/main/dtw.cc
  ${SOURCE_DIR}/main/dtw_merge.cc
  ${SOURCE_DIR}/main/entropy.cc
  ${SOURCE_DIR}/main/excite.cc
  ${SOURCE_DIR}/main/extract.cc
  ${SOURCE_DIR}/main/fbank.cc
  ${SOURCE_DIR}/main/fd.cc
  ${SOURCE_DIR}/main/fft.cc
  ${SOURCE_DIR}/main/fft2.cc
  ${SOURCE_DIR}/main/fftcep.cc
  ${SOURCE_DIR}/main/fftr.cc
  ${SOURCE_DIR}/main/fftr2.cc
  ${SOURCE_DIR}/main/frame.cc
  ${SOURCE_DIR}/main/freqt.cc
  ${SOURCE_DIR}/main/gmm.cc
  ${SOURCE_DIR}/main/gmmp.cc
  ${SOURCE_DIR}/main/gnorm.cc
  ${SOURCE_DIR}/main/grpdelay.cc
  ${SOURCE_DIR}/main/histogram.cc
  ${SOURCE_DIR}/main/huffman.cc
  ${SOURCE_DIR}/main/huffman_decode.cc
  ${SOURCE_DIR}/main/huffman_encode.cc
  ${SOURCE_DIR}/main/idct.cc
  ${SOURCE_DIR}/main/ifft.cc
  ${SOURCE_DIR}/main/ifft2.cc
  ${SOURCE_DIR}/main/ignorm.cc
  ${SOURCE_DIR}/main/imglsadf.cc
  ${SOURCE_DIR}/main/impulse.cc
  ${SOURCE_DIR}/main/imsvq.cc
  ${SOURCE_DIR}/main/interpolate.cc
  ${SOURCE_DIR}/main/ipqmf.cc
  ${SOURCE_DIR}/main/iulaw.cc
  ${SOURCE_DIR}/main/lar2par.cc
  ${SOURCE_DIR}/main/lbg.cc
  ${SOURCE_DIR}/main/levdur.cc
  ${SOURCE_DIR}/main/linear_intpl.cc
  ${SOURCE_DIR}/main/lpc.cc
  ${SOURCE_DIR}/main/lpc2c.cc
  ${SOURCE_DIR}/main/lpc2lsp.cc
  ${SOURCE_DIR}/main/lpc2par.cc
  ${SOURCE_DIR}/main/lpccheck.cc
  ${SOURCE_DIR}/main/lsp2lpc.cc
  ${SOURCE_DIR}/main/lspcheck.cc
  ${SOURCE_DIR}/main/lspdf.cc
  ${SOURCE_DIR}/main/ltcdf.cc
  ${SOURCE_DIR}/main/magic_intpl.cc
  ${SOURCE_DIR}/main/mc2b.cc
  ${SOURCE_DIR}/main/mcpf.cc
  ${SOURCE_DIR}/main/medfilt.cc
  ${SOURCE_DIR}/main/median.cc
  ${SOURCE_DIR}/main/merge.cc
  ${SOURCE_DIR}/main/mfcc.cc
  ${SOURCE_DIR}/main/mgc2mgc.cc
  ${SOURCE_DIR}/main/mgc2sp.cc
  ${SOURCE_DIR}/main/mgcep.cc
  ${SOURCE_DIR}/main/mglsadf.cc
  ${SOURCE_DIR}/main/mglsp2sp.cc
  ${SOURCE_DIR}/main/minmax.cc
  ${SOURCE_DIR}/main/mlpg.cc
  ${SOURCE_DIR}/main/mlsacheck.cc
  ${SOURCE_DIR}/main/mpir2c.cc
  ${SOURCE_DIR}/main/mseq.cc
  ${SOURCE_DIR}/main/msvq.cc
  ${SOURCE_DIR}/main/nan.cc
  ${SOURCE_DIR}/main/ndps2c.cc
  ${SOURCE_DIR}/main/norm0.cc
  ${SOURCE_DIR}/main/nrand.cc
  ${SOURCE_DIR}/main/par2lar.cc
  ${SOURCE_DIR}/main/par2lpc.cc
  ${SOURCE_DIR}/main/pca.cc
  ${SOURCE_DIR}/main/pcas.cc
  ${SOURCE_DIR}/main/phase.cc
  ${SOURCE_DIR}/main/pitch.cc
  ${SOURCE_DIR}/main/pitch_mark.cc
  ${SOURCE_DIR}/main/poledf.cc
  ${SOURCE_DIR}/main/pqmf.cc
  ${SOURCE_DIR}/main/quantize.cc
  ${SOURCE_DIR}/main/ramp.cc
  ${SOURCE_DIR}/main/reverse.cc
  ${SOURCE_DIR}/main/rlevdur.cc
  ${SOURCE_DIR}/main/rmse.cc
  ${SOURCE_DIR}/main/root_pol.cc
  ${SOURCE_DIR}/main/sin.cc
  ${SOURCE_DIR}/main/smcep.cc
  ${SOURCE_DIR}/main/snr.cc
  ${SOURCE_DIR}/main/sopr.cc
  ${SOURCE_DIR}/main/spec.cc
  ${SOURCE_DIR}/main/step.cc
  ${SOURCE_DIR}/main/swab.cc
  ${SOURCE_DIR}/main/symmetrize.cc
  ${SOURCE_DIR}/main/train.cc
  ${SOURCE_DIR}/main/transpose.cc
  ${SOURCE_DIR}/main/ulaw.cc
  ${SOURCE_DIR}/main/vc.cc
  ${SOURCE_DIR}/main/vopr.cc
  ${SOURCE_DIR}/main/vstat.cc
  ${SOURCE_DIR}/main/vsum.cc
  ${SOURCE_DIR}/main/window.cc
  ${SOURCE_DIR}/main/x2x.cc
  ${SOURCE_DIR}/main/zcross.cc
  ${SOURCE_DIR}/main/zerodf.cc
  )

set(PYTHON_SOURCES
  ${SOURCE_DIR}/draw/fdrw.py
  ${SOURCE_DIR}/draw/glogsp.py
  ${SOURCE_DIR}/draw/gpolezero.py
  ${SOURCE_DIR}/draw/grlogsp.py
  ${SOURCE_DIR}/draw/gseries.py
  ${SOURCE_DIR}/draw/gspecgram.py
  ${SOURCE_DIR}/draw/gwave.py
  )

if(NOT WIN32)
  foreach(SOURCE ${MAIN_SOURCES})
    get_filename_component(BIN ${SOURCE} NAME_WE)
    add_executable(${BIN} ${SOURCE})
    target_link_libraries(${BIN} sptk)
    install(TARGETS ${BIN}
      RUNTIME DESTINATION bin
      )
  endforeach()

  foreach(SOURCE ${PYTHON_SOURCES})
    get_filename_component(BIN ${SOURCE} NAME_WE)
    execute_process(COMMAND ln -snf ${SOURCE} ${CMAKE_CURRENT_BINARY_DIR}/${BIN})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN}
      DESTINATION bin
      )
  endforeach()
endif()
