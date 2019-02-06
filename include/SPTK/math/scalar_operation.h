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

#ifndef SPTK_MATH_SCALAR_OPERATION_H_
#define SPTK_MATH_SCALAR_OPERATION_H_

#include <vector>  // std::vector

#include "SPTK/utils/sptk_utils.h"

namespace sptk {

class ScalarOperation {
 public:
  //
  class ModuleInterface {
   public:
    virtual ~ModuleInterface() {
    }

    virtual bool Run(double* number, bool* is_magic_number) const = 0;
  };

  //
  ScalarOperation() : use_magic_number_(false) {
  }

  //
  virtual ~ScalarOperation() {
    for (std::vector<ScalarOperation::ModuleInterface*>::iterator itr(
             modules_.begin());
         itr != modules_.end(); ++itr) {
      delete (*itr);
    }
  }

  //
  bool AddAdditionOperation(double addend);

  //
  bool AddSubtractionOperation(double subtrahend);

  //
  bool AddMultiplicationOperation(double multiplier);

  //
  bool AddDivisionOperation(double divisor);

  //
  bool AddModuloOperation(int divisor);

  //
  bool AddPowerOperation(double exponent);

  //
  bool AddLowerBoundingOperation(double lower_bound);

  //
  bool AddUpperBoundingOperation(double upper_bound);

  //
  bool AddAbsoluteOperation();

  //
  bool AddReciprocalOperation();

  //
  bool AddSquareOperation();

  //
  bool AddSquareRootOperation();

  //
  bool AddNaturalLogarithmOperation();

  //
  bool AddLogarithmOperation(double base);

  //
  bool AddNaturalExponentialOperation();

  //
  bool AddExponentialOperation(double base);

  //
  bool AddFlooringOperation();

  //
  bool AddCeilingOperation();

  //
  bool AddRoundingOperation();

  //
  bool AddRoundingUpOperation();

  //
  bool AddRoundingDownOperation();

  //
  bool AddUnitStepOperation();

  //
  bool AddSignOperation();

  //
  bool AddSineOperation();

  //
  bool AddCosineOperation();

  //
  bool AddTangentOperation();

  //
  bool AddArctangentOperation();

  //
  bool AddMagicNumberRemover(double magic_number);

  //
  bool AddMagicNumberReplacer(double replacement_number);

  //
  bool Run(double* number, bool* is_magic_number) const;

 private:
  //
  bool use_magic_number_;

  //
  std::vector<ScalarOperation::ModuleInterface*> modules_;

  //
  DISALLOW_COPY_AND_ASSIGN(ScalarOperation);
};

}  // namespace sptk

#endif  // SPTK_MATH_SCALAR_OPERATION_H_
