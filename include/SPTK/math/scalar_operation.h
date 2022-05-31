// ------------------------------------------------------------------------ //
// Copyright 2021 SPTK Working Group                                        //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ------------------------------------------------------------------------ //

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
  bool AddModuloOperation(double divisor);

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
