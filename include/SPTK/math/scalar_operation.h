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
//                1996-2020  Nagoya Institute of Technology          //
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

#ifndef SPTK_MATH_SCALAR_OPERATION_H_
#define SPTK_MATH_SCALAR_OPERATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

/**
 * Perform a sequence of scalar operations.
 */
class ScalarOperation {
 public:
  /**
   * Interface of scalar operation.
   */
  class ModuleInterface {
   public:
    virtual ~ModuleInterface() {
    }

    /**
     * @param[in,out] number Input/output number.
     * @param[in,out] is_magic_number True if output is magic number.
     */
    virtual bool Run(double* number, bool* is_magic_number) const = 0;
  };

  ScalarOperation() : use_magic_number_(false) {
  }

  virtual ~ScalarOperation() {
    for (std::vector<ScalarOperation::ModuleInterface*>::iterator itr(
             modules_.begin());
         itr != modules_.end(); ++itr) {
      delete (*itr);
    }
  }

  /**
   * @f$x + a@f$
   *
   * @param[in] addend Addend, @f$a@f$.
   * @return True on success, false on failure.
   */
  bool AddAdditionOperation(double addend);

  /**
   * @f$x - s@f$
   *
   * @param[in] subtrahend Subtrahend, @f$s@f$.
   * @return True on success, false on failure.
   */
  bool AddSubtractionOperation(double subtrahend);

  /**
   * @f$x * m@f$
   *
   * @param[in] multiplier Multiplier, @f$m@f$.
   * @return True on success, false on failure.
   */
  bool AddMultiplicationOperation(double multiplier);

  /**
   * @f$x / d@f$
   *
   * @param[in] divisor Divisor, @f$d@f$.
   * @return True on success, false on failure.
   */
  bool AddDivisionOperation(double divisor);

  /**
   * @f$x \bmod n@f$
   *
   * @param[in] divisor Divisor, @f$n@f$.
   * @return True on success, false on failure.
   */
  bool AddModuloOperation(int divisor);

  /**
   * @f$x^n@f$
   *
   * @param[in] exponent Exponent, @f$n@f$.
   * @return True on success, false on failure.
   */
  bool AddPowerOperation(double exponent);

  /**
   * @f$\max(x, l)@f$
   *
   * @param[in] lower_bound Lower bound, @f$l@f$.
   * @return True on success, false on failure.
   */
  bool AddLowerBoundingOperation(double lower_bound);

  /**
   * @f$\min(x, u)@f$
   *
   * @param[in] upper_bound Upper bound, @f$u@f$.
   * @return True on success, false on failure.
   */
  bool AddUpperBoundingOperation(double upper_bound);

  /**
   * @f$|x|@f$
   *
   * @return True on success, false on failure.
   */
  bool AddAbsoluteOperation();

  /**
   * @f$1/x@f$
   *
   * @return True on success, false on failure.
   */
  bool AddReciprocalOperation();

  /**
   * @f$x^2@f$
   *
   * @return True on success, false on failure.
   */
  bool AddSquareOperation();

  /**
   * @f$\sqrt{x}@f$
   *
   * @return True on success, false on failure.
   */
  bool AddSquareRootOperation();

  /**
   * @f$\ln{x}@f$
   *
   * @return True on success, false on failure.
   */
  bool AddNaturalLogarithmOperation();

  /**
   * @f$\log_b{x}@f$
   *
   * @param[in] base Base of logarithm function, @f$b@f$.
   * @return True on success, false on failure.
   */
  bool AddLogarithmOperation(double base);

  /**
   * @f$\exp{x}@f$
   *
   * @return True on success, false on failure.
   */
  bool AddNaturalExponentialOperation();

  /**
   * @f$b^x@f$
   *
   * @param[in] base Base of exponential function, @f$b@f$.
   * @return True on success, false on failure.
   */
  bool AddExponentialOperation(double base);

  /**
   * @f$\lfloor x \rfloor@f$
   *
   * @return True on success, false on failure.
   */
  bool AddFlooringOperation();

  /**
   * @f$\lceil x \rceil@f$
   *
   * @return True on success, false on failure.
   */
  bool AddCeilingOperation();

  /**
   * @f$\mathrm{nint}(x)@f$
   *
   * @return True on success, false on failure.
   */
  bool AddRoundingOperation();

  /**
   * @f$\mathrm{sgn}(x) \lceil |x| \rceil@f$
   *
   * @return True on success, false on failure.
   */
  bool AddRoundingUpOperation();

  /**
   * @f$\mathrm{sgn}(x) \lfloor |x| \rfloor@f$
   *
   * @return True on success, false on failure.
   */
  bool AddRoundingDownOperation();

  /**
   * @f$u(x)@f$
   *
   * @return True on success, false on failure.
   */
  bool AddUnitStepOperation();

  /**
   * @f$\mathrm{sgn}(x)@f$
   *
   * @return True on success, false on failure.
   */
  bool AddSignOperation();

  /**
   * @f$\sin x@f$
   *
   * @return True on success, false on failure.
   */
  bool AddSineOperation();

  /**
   * @f$\cos x@f$
   *
   * @return True on success, false on failure.
   */
  bool AddCosineOperation();

  /**
   * @f$\tan x@f$
   *
   * @return True on success, false on failure.
   */
  bool AddTangentOperation();

  /**
   * @f$\tan^{-1} x@f$
   *
   * @return True on success, false on failure.
   */
  bool AddArctangentOperation();

  /**
   * @f$\tanh x@f$
   *
   * @return True on success, false on failure.
   */
  bool AddHyperbolicTangentOperation();

  /**
   * @f$\tanh^{-1} x@f$
   *
   * @return True on success, false on failure.
   */
  bool AddHyperbolicArctangentOperation();

  /**
   * Remove magic number.
   *
   * @param[in] magic_number A magic number.
   * @return True on success, false on failure.
   */
  bool AddMagicNumberRemover(double magic_number);

  /**
   * Replace magic number.
   *
   * @param[in] replacement_number A replacement for magic number.
   * @return True on success, false on failure.
   */
  bool AddMagicNumberReplacer(double replacement_number);

  /**
   * @param[in,out] number Input/output number.
   * @param[out] is_magic_number True if output is magic number.
   * @return True on success, false on failure.
   */
  bool Run(double* number, bool* is_magic_number) const;

 private:
  bool use_magic_number_;

  std::vector<ScalarOperation::ModuleInterface*> modules_;

  DISALLOW_COPY_AND_ASSIGN(ScalarOperation);
};

}  // namespace sptk

#endif  // SPTK_MATH_SCALAR_OPERATION_H_
