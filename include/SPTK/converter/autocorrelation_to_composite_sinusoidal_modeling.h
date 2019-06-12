// ----------------------------------------------------------------- //
//             The Speech Signal Processing Toolkit (SPTK)           //
//             developed by SPTK Working Group                       //
//             http://sp-tk.sourceforge.net/                         //
// ----------------------------------------------------------------- //
//                                                                   //
//  Copyright (c) 1984-2007  Tokyo Institute of Technology           //
//                           Interdisciplinary Graduate School of    //
//                           Science and Engineering                 //
//                                                                   //
//                1996-2019  Nagoya Institute of Technology          //
//                           Department of Computer Science          //
//                                                                   //
// All rights reserved.                                              //
//                                                                   //
// Redistribution and use in source and binary forms, with or        //
// without modification, are permitted provided that the following   //
// conditions are met:                                               //
//                                                                   //
// - Redistributions of source code must retain the above copyright  //
//   notice, this list of conditions and the following disclaimer.   //
// - Redistributions in binary form must reproduce the above         //
//   copyright notice, this list of conditions and the following     //
//   disclaimer in the documentation and/or other materials provided //
//   with the distribution.                                          //
// - Neither the name of the SPTK working group nor the names of its //
//   contributors may be used to endorse or promote products derived //
//   from this software without specific prior written permission.   //
//                                                                   //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            //
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       //
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          //
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          //
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS //
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          //
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   //
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     //
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON //
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   //
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    //
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           //
// POSSIBILITY OF SUCH DAMAGE.                                       //
// ----------------------------------------------------------------- //

#ifndef SPTK_CONVERTER_AUTOCORRELATION_TO_COMPOSITE_SINUSOIDAL_MODELING_H_
#define SPTK_CONVERTER_AUTOCORRELATION_TO_COMPOSITE_SINUSOIDAL_MODELING_H_

#include <complex>  // std::complex
#include <vector>   // std::vector

#include "SPTK/math/durand_kerner_method.h"
#include "SPTK/math/symmetric_system_solver.h"
#include "SPTK/math/vandermonde_system_solver.h"
#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class AutocorrelationToCompositeSinusoidalModeling {
 public:
  class Buffer {
   public:
    Buffer() {
    }
    virtual ~Buffer() {
    }

   private:
    std::vector<double> u_;
    std::vector<double> u_first_half_;
    std::vector<double> u_second_half_;
    SymmetricMatrix u_symmetric_matrix_;
    std::vector<std::complex<double> > x_;
    std::vector<double> x_real_part_;
    std::vector<double> p_;
    std::vector<double> intensities_;
    SymmetricSystemSolver::Buffer symmetric_system_solver_buffer_;
    VandermondeSystemSolver::Buffer vandermonde_system_solver_buffer_;

    friend class AutocorrelationToCompositeSinusoidalModeling;
    DISALLOW_COPY_AND_ASSIGN(Buffer);
  };

  //
  AutocorrelationToCompositeSinusoidalModeling(int num_order, int num_iteration,
                                               double convergence_threshold);

  //
  virtual ~AutocorrelationToCompositeSinusoidalModeling() {
  }

  //
  int GetNumOrder() const {
    return num_order_;
  }

  //
  int GetNumSineWave() const {
    return num_sine_wave_;
  }

  //
  int GetNumIteration() const {
    return durand_kerner_method_.GetNumIteration();
  }

  //
  double GetConvergenceThreshold() const {
    return durand_kerner_method_.GetConvergenceThreshold();
  }

  //
  bool IsValid() const {
    return is_valid_;
  }

  //
  bool Run(const std::vector<double>& autocorrelation,
           std::vector<double>* composite_sinusoidal_modeling,
           AutocorrelationToCompositeSinusoidalModeling::Buffer* buffer) const;

 private:
  //
  const int num_order_;

  //
  const int num_sine_wave_;

  //
  const SymmetricSystemSolver symmetric_system_solver_;

  //
  const DurandKernerMethod durand_kerner_method_;

  //
  const VandermondeSystemSolver vandermonde_system_solver_;

  //
  bool is_valid_;

  //
  DISALLOW_COPY_AND_ASSIGN(AutocorrelationToCompositeSinusoidalModeling);
};

}  // namespace sptk

#endif  // SPTK_CONVERTER_AUTOCORRELATION_TO_COMPOSITE_SINUSOIDAL_MODELING_H_
