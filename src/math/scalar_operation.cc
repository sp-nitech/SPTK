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

#include "SPTK/math/scalar_operation.h"

#include <cmath>  // std::atan, std::atanh, std::ceil, std::cos, std::exp, std::fabs, std::floor, std::fmod, std::log, std::pow, std::round, std::sin, std::sqrt, std::tan, std::tanh, std::trunc

namespace {

class OperationInterface {
 public:
  virtual ~OperationInterface() {
  }

  virtual bool Run(double* number) const = 0;
};

/**
 * This class is inherited by the operation class that does not handle magic
 * number.
 */
class OperationPerformer : public sptk::ScalarOperation::ModuleInterface {
 public:
  explicit OperationPerformer(OperationInterface* operation)
      : operation_(operation) {
  }

  virtual ~OperationPerformer() {
    delete operation_;
  }

  virtual bool Run(double* number, bool* is_magic_number) const {
    if (*is_magic_number) {
      return true;
    }
    return operation_->Run(number);
  }

 private:
  const OperationInterface* operation_;
  DISALLOW_COPY_AND_ASSIGN(OperationPerformer);
};

class Addition : public OperationInterface {
 public:
  explicit Addition(double addend) : addend_(addend) {
  }

  virtual bool Run(double* number) const {
    *number += addend_;
    return true;
  }

 private:
  const double addend_;
  DISALLOW_COPY_AND_ASSIGN(Addition);
};

class Subtraction : public OperationInterface {
 public:
  explicit Subtraction(double subtrahend) : subtrahend_(subtrahend) {
  }

  virtual bool Run(double* number) const {
    *number -= subtrahend_;
    return true;
  }

 private:
  const double subtrahend_;
  DISALLOW_COPY_AND_ASSIGN(Subtraction);
};

class Multiplication : public OperationInterface {
 public:
  explicit Multiplication(double multiplier) : multiplier_(multiplier) {
  }

  virtual bool Run(double* number) const {
    *number *= multiplier_;
    return true;
  }

 private:
  const double multiplier_;
  DISALLOW_COPY_AND_ASSIGN(Multiplication);
};

class Division : public OperationInterface {
 public:
  explicit Division(double divisor) : multiplier_(1.0 / divisor) {
  }

  virtual bool Run(double* number) const {
    *number *= multiplier_;
    return true;
  }

 private:
  const double multiplier_;
  DISALLOW_COPY_AND_ASSIGN(Division);
};

class Modulo : public OperationInterface {
 public:
  explicit Modulo(double divisor) : divisor_(divisor) {
  }

  virtual bool Run(double* number) const {
    *number = std::fmod(*number, divisor_);
    return true;
  }

 private:
  const double divisor_;
  DISALLOW_COPY_AND_ASSIGN(Modulo);
};

class Power : public OperationInterface {
 public:
  explicit Power(double exponent) : exponent_(exponent) {
  }

  virtual bool Run(double* number) const {
    *number = std::pow(*number, exponent_);
    return true;
  }

 private:
  const double exponent_;
  DISALLOW_COPY_AND_ASSIGN(Power);
};

class LowerBounding : public OperationInterface {
 public:
  explicit LowerBounding(double lower_bound) : lower_bound_(lower_bound) {
  }

  virtual bool Run(double* number) const {
    if (*number < lower_bound_) *number = lower_bound_;
    return true;
  }

 private:
  const double lower_bound_;
  DISALLOW_COPY_AND_ASSIGN(LowerBounding);
};

class UpperBounding : public OperationInterface {
 public:
  explicit UpperBounding(double upper_bound) : upper_bound_(upper_bound) {
  }

  virtual bool Run(double* number) const {
    if (upper_bound_ < *number) *number = upper_bound_;
    return true;
  }

 private:
  const double upper_bound_;
  DISALLOW_COPY_AND_ASSIGN(UpperBounding);
};

class Absolute : public OperationInterface {
 public:
  Absolute() {
  }

  virtual bool Run(double* number) const {
    *number = std::fabs(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Absolute);
};

class Reciprocal : public OperationInterface {
 public:
  Reciprocal() {
  }

  virtual bool Run(double* number) const {
    *number = 1.0 / *number;
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Reciprocal);
};

class Square : public OperationInterface {
 public:
  Square() {
  }

  virtual bool Run(double* number) const {
    *number *= *number;
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Square);
};

class SquareRoot : public OperationInterface {
 public:
  SquareRoot() {
  }

  virtual bool Run(double* number) const {
    *number = std::sqrt(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(SquareRoot);
};

class NaturalLogarithm : public OperationInterface {
 public:
  NaturalLogarithm() {
  }

  virtual bool Run(double* number) const {
    *number = std::log(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(NaturalLogarithm);
};

class Logarithm : public OperationInterface {
 public:
  explicit Logarithm(double base) : multiplier_(1.0 / std::log(base)) {
  }

  virtual bool Run(double* number) const {
    *number = std::log(*number) * multiplier_;
    return true;
  }

 private:
  const double multiplier_;
  DISALLOW_COPY_AND_ASSIGN(Logarithm);
};

class NaturalExponential : public OperationInterface {
 public:
  NaturalExponential() {
  }

  virtual bool Run(double* number) const {
    *number = std::exp(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(NaturalExponential);
};

class Exponential : public OperationInterface {
 public:
  explicit Exponential(double base) : base_(base) {
  }

  virtual bool Run(double* number) const {
    *number = std::pow(base_, *number);
    return true;
  }

 private:
  const double base_;
  DISALLOW_COPY_AND_ASSIGN(Exponential);
};

class Flooring : public OperationInterface {
 public:
  Flooring() {
  }

  virtual bool Run(double* number) const {
    *number = std::floor(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Flooring);
};

class Ceiling : public OperationInterface {
 public:
  Ceiling() {
  }

  virtual bool Run(double* number) const {
    *number = std::ceil(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Ceiling);
};

class Rounding : public OperationInterface {
 public:
  Rounding() {
  }

  virtual bool Run(double* number) const {
    *number = std::round(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Rounding);
};

class RoundingUp : public OperationInterface {
 public:
  RoundingUp() {
  }

  virtual bool Run(double* number) const {
    *number = (*number < 0.0) ? std::floor(*number) : std::ceil(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(RoundingUp);
};

class RoundingDown : public OperationInterface {
 public:
  RoundingDown() {
  }

  virtual bool Run(double* number) const {
    *number = std::trunc(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(RoundingDown);
};

class UnitStep : public OperationInterface {
 public:
  UnitStep() {
  }

  virtual bool Run(double* number) const {
    *number = (*number < 0.0) ? 0.0 : 1.0;
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(UnitStep);
};

class Sign : public OperationInterface {
 public:
  Sign() {
  }

  virtual bool Run(double* number) const {
    *number = sptk::ExtractSign(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Sign);
};

class Sine : public OperationInterface {
 public:
  Sine() {
  }

  virtual bool Run(double* number) const {
    *number = std::sin(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Sine);
};

class Cosine : public OperationInterface {
 public:
  Cosine() {
  }

  virtual bool Run(double* number) const {
    *number = std::cos(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Cosine);
};

class Tangent : public OperationInterface {
 public:
  Tangent() {
  }

  virtual bool Run(double* number) const {
    *number = std::tan(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Tangent);
};

class Arctangent : public OperationInterface {
 public:
  Arctangent() {
  }

  virtual bool Run(double* number) const {
    *number = std::atan(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(Arctangent);
};

class HyperbolicTangent : public OperationInterface {
 public:
  HyperbolicTangent() {
  }

  virtual bool Run(double* number) const {
    *number = std::tanh(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(HyperbolicTangent);
};

class HyperbolicArctangent : public OperationInterface {
 public:
  HyperbolicArctangent() {
  }

  virtual bool Run(double* number) const {
    *number = std::atanh(*number);
    return true;
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(HyperbolicArctangent);
};

class MagicNumberRemover : public sptk::ScalarOperation::ModuleInterface {
 public:
  explicit MagicNumberRemover(double magic_number)
      : magic_number_(magic_number) {
  }

  virtual bool Run(double* number, bool* is_magic_number) const {
    if (*is_magic_number) {
      return false;
    }
    if (magic_number_ == *number) {
      *is_magic_number = true;
    }
    return true;
  }

 private:
  const double magic_number_;
  DISALLOW_COPY_AND_ASSIGN(MagicNumberRemover);
};

class MagicNumberReplacer : public sptk::ScalarOperation::ModuleInterface {
 public:
  explicit MagicNumberReplacer(double replacement_number)
      : replacement_number_(replacement_number) {
  }

  virtual bool Run(double* number, bool* is_magic_number) const {
    if (*is_magic_number) {
      *number = replacement_number_;
      *is_magic_number = false;
    }
    return true;
  }

 private:
  const double replacement_number_;
  DISALLOW_COPY_AND_ASSIGN(MagicNumberReplacer);
};

}  // namespace

namespace sptk {

bool ScalarOperation::AddAdditionOperation(double addend) {
  modules_.push_back(new OperationPerformer(new Addition(addend)));
  return true;
}

bool ScalarOperation::AddSubtractionOperation(double subtrahend) {
  modules_.push_back(new OperationPerformer(new Subtraction(subtrahend)));
  return true;
}

bool ScalarOperation::AddMultiplicationOperation(double multiplier) {
  modules_.push_back(new OperationPerformer(new Multiplication(multiplier)));
  return true;
}

bool ScalarOperation::AddDivisionOperation(double divisor) {
  if (0.0 == divisor) return false;
  modules_.push_back(new OperationPerformer(new Division(divisor)));
  return true;
}

bool ScalarOperation::AddModuloOperation(double divisor) {
  if (0 == divisor) return false;
  modules_.push_back(new OperationPerformer(new Modulo(divisor)));
  return true;
}

bool ScalarOperation::AddPowerOperation(double exponent) {
  modules_.push_back(new OperationPerformer(new Power(exponent)));
  return true;
}

bool ScalarOperation::AddLowerBoundingOperation(double lower_bound) {
  modules_.push_back(new OperationPerformer(new LowerBounding(lower_bound)));
  return true;
}

bool ScalarOperation::AddUpperBoundingOperation(double upper_bound) {
  modules_.push_back(new OperationPerformer(new UpperBounding(upper_bound)));
  return true;
}

bool ScalarOperation::AddAbsoluteOperation() {
  modules_.push_back(new OperationPerformer(new Absolute()));
  return true;
}

bool ScalarOperation::AddReciprocalOperation() {
  modules_.push_back(new OperationPerformer(new Reciprocal()));
  return true;
}

bool ScalarOperation::AddSquareOperation() {
  modules_.push_back(new OperationPerformer(new Square()));
  return true;
}

bool ScalarOperation::AddSquareRootOperation() {
  modules_.push_back(new OperationPerformer(new SquareRoot()));
  return true;
}

bool ScalarOperation::AddNaturalLogarithmOperation() {
  modules_.push_back(new OperationPerformer(new NaturalLogarithm()));
  return true;
}

bool ScalarOperation::AddLogarithmOperation(double base) {
  if (base <= 0.0) return false;
  modules_.push_back(new OperationPerformer(new Logarithm(base)));
  return true;
}

bool ScalarOperation::AddNaturalExponentialOperation() {
  modules_.push_back(new OperationPerformer(new NaturalExponential()));
  return true;
}

bool ScalarOperation::AddExponentialOperation(double base) {
  modules_.push_back(new OperationPerformer(new Exponential(base)));
  return true;
}

bool ScalarOperation::AddFlooringOperation() {
  modules_.push_back(new OperationPerformer(new Flooring()));
  return true;
}

bool ScalarOperation::AddCeilingOperation() {
  modules_.push_back(new OperationPerformer(new Ceiling()));
  return true;
}

bool ScalarOperation::AddRoundingOperation() {
  modules_.push_back(new OperationPerformer(new Rounding()));
  return true;
}

bool ScalarOperation::AddRoundingUpOperation() {
  modules_.push_back(new OperationPerformer(new RoundingUp()));
  return true;
}

bool ScalarOperation::AddRoundingDownOperation() {
  modules_.push_back(new OperationPerformer(new RoundingDown()));
  return true;
}

bool ScalarOperation::AddUnitStepOperation() {
  modules_.push_back(new OperationPerformer(new UnitStep()));
  return true;
}

bool ScalarOperation::AddSignOperation() {
  modules_.push_back(new OperationPerformer(new Sign()));
  return true;
}

bool ScalarOperation::AddSineOperation() {
  modules_.push_back(new OperationPerformer(new Sine()));
  return true;
}

bool ScalarOperation::AddCosineOperation() {
  modules_.push_back(new OperationPerformer(new Cosine()));
  return true;
}

bool ScalarOperation::AddTangentOperation() {
  modules_.push_back(new OperationPerformer(new Tangent()));
  return true;
}

bool ScalarOperation::AddArctangentOperation() {
  modules_.push_back(new OperationPerformer(new Arctangent()));
  return true;
}

bool ScalarOperation::AddHyperbolicTangentOperation() {
  modules_.push_back(new OperationPerformer(new HyperbolicTangent()));
  return true;
}

bool ScalarOperation::AddHyperbolicArctangentOperation() {
  modules_.push_back(new OperationPerformer(new HyperbolicArctangent()));
  return true;
}

bool ScalarOperation::AddMagicNumberRemover(double magic_number) {
  if (use_magic_number_) return false;
  modules_.push_back(new MagicNumberRemover(magic_number));
  use_magic_number_ = true;
  return true;
}

bool ScalarOperation::AddMagicNumberReplacer(double replacement_number) {
  if (!use_magic_number_) return false;
  modules_.push_back(new MagicNumberReplacer(replacement_number));
  use_magic_number_ = false;
  return true;
}

bool ScalarOperation::Run(double* number, bool* is_magic_number) const {
  if (NULL == number || NULL == is_magic_number) {
    return false;
  }

  *is_magic_number = false;
  for (std::vector<ScalarOperation::ModuleInterface*>::const_iterator itr(
           modules_.begin());
       itr != modules_.end(); ++itr) {
    if (!(*itr)->Run(number, is_magic_number)) {
      return false;
    }
  }

  return true;
}

}  // namespace sptk
