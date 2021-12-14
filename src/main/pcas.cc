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

#include <fstream>   // std::ifstream
#include <iomanip>   // std::setw
#include <iostream>  // std::cerr, std::cin, std::cout, std::endl, etc.
#include <sstream>   // std::ostringstream

#include "Getopt/getoptwin.h"
#include "SPTK/math/matrix.h"
#include "SPTK/utils/sptk_utils.h"

namespace {

const int kDefaultVectorLength(25);
const int kDefaultNumPrincipalComponent(2);

void PrintUsage(std::ostream* stream) {
  // clang-format off
  *stream << std::endl;
  *stream << " pcas - compute principal component score" << std::endl;
  *stream << std::endl;
  *stream << "  usage:" << std::endl;
  *stream << "       pcas [ options ] evfile [ infile ] > stdout" << std::endl;
  *stream << "  options:" << std::endl;
  *stream << "       -l l  : length of vector               (   int)[" << std::setw(5) << std::right << kDefaultVectorLength          << "][ 1 <= l <=   ]" << std::endl;  // NOLINT
  *stream << "       -m m  : order of vectors               (   int)[" << std::setw(5) << std::right << "l-1"                         << "][ 0 <= m <=   ]" << std::endl;  // NOLINT
  *stream << "       -n n  : number of principal components (   int)[" << std::setw(5) << std::right << kDefaultNumPrincipalComponent << "][ 1 <= n <= l ]" << std::endl;  // NOLINT
  *stream << "       -h    : print this message" << std::endl;
  *stream << "  evfile:" << std::endl;
  *stream << "       mean vector and eigenvectors           (double)" << std::endl;  // NOLINT
  *stream << "  infile:" << std::endl;
  *stream << "       vector sequence                        (double)[stdin]" << std::endl;  // NOLINT
  *stream << "  stdout:" << std::endl;
  *stream << "       principal component scores             (double)" << std::endl;  // NOLINT
  *stream << std::endl;
  *stream << " SPTK: version " << sptk::kVersion << std::endl;
  *stream << std::endl;
  // clang-format on
}

}  // namespace

/**
 * @a pcas [ @e option ] @e evfile [ @e infile ]
 *
 * - @b -l @e int
 *   - length of vector @f$(1 \le L)@f$
 * - @b -m @e int
 *   - order of vector @f$(0 \le M)@f$
 * - @b -n @e int
 *   - number of principal components @f$(1 \le N \le L)@f$
 * - @b evfile @e str
 *   - double-type mean vector and eigenvectors
 * - @b infile @e str
 *   - double-type vector sequence
 * - @b stdout
 *   - double-type principal component score
 *
 * The input of this command is the @f$M@f$-th order vectors:
 * @f[
 *   \begin{array}{cccc}
 *     \boldsymbol{x}(0), & \boldsymbol{x}(1), & \boldsymbol{x}(0), & \ldots,
 *   \end{array}
 * @f]
 * the mean vector @f$\boldsymbol{m}@f$, and @f$N@f$ eigenvectors
 * @f[
 *   \boldsymbol{A} = \left[ \begin{array}{cccc}
 *     \boldsymbol{e}(0) & \boldsymbol{e}(1) & \cdots & \boldsymbol{e}(N-1)
 *   \end{array} \right].
 * @f]
 * The pricipal component score of @f$\boldsymbol{x}(t)@f$ is calculated as
 * @f[
 *   \boldsymbol{z}(t) = \boldsymbol{A}^{\mathsf{T}}
 *     (\boldsymbol{x}(t) - \boldsymbol{m}).
 * @f]
 *
 * In the below example, the principal component scores of @c data.d are
 * calculated using the mean vector and the eigen vectors in @c eigvec.dat.
 *
 * @code{.sh}
 *   pcas -l 3 -n 2 eigvec.dat < data.d > score.dat
 * @endcode
 *
 * @param[in] argc Number of arguments.
 * @param[in] argv Argument vector.
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
  int vector_length(kDefaultVectorLength);
  int num_principal_component(kDefaultNumPrincipalComponent);

  for (;;) {
    const int option_char(getopt_long(argc, argv, "l:m:n:h", NULL, NULL));
    if (-1 == option_char) break;

    switch (option_char) {
      case 'l': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -l option must be a positive integer";
          sptk::PrintErrorMessage("pcas", error_message);
          return 1;
        }
        break;
      }
      case 'm': {
        if (!sptk::ConvertStringToInteger(optarg, &vector_length) ||
            vector_length < 0) {
          std::ostringstream error_message;
          error_message << "The argument for the -m option must be a "
                        << "non-negative integer";
          sptk::PrintErrorMessage("pcas", error_message);
          return 1;
        }
        ++vector_length;
        break;
      }
      case 'n': {
        if (!sptk::ConvertStringToInteger(optarg, &num_principal_component) ||
            num_principal_component <= 0) {
          std::ostringstream error_message;
          error_message
              << "The argument for the -n option must be a positive integer";
          sptk::PrintErrorMessage("pcas", error_message);
          return 1;
        }
        break;
      }
      case 'h': {
        PrintUsage(&std::cout);
        return 0;
      }
      default: {
        PrintUsage(&std::cerr);
        return 1;
      }
    }
  }

  if (vector_length < num_principal_component) {
    std::ostringstream error_message;
    error_message << "Number of principal components must be equal to or "
                  << "less than length of input vector";
    sptk::PrintErrorMessage("pcas", error_message);
    return 1;
  }

  const char* eigenvectors_file;
  const char* input_file;
  const int num_input_files(argc - optind);
  if (2 == num_input_files) {
    eigenvectors_file = argv[argc - 2];
    input_file = argv[argc - 1];
  } else if (1 == num_input_files) {
    eigenvectors_file = argv[argc - 1];
    input_file = NULL;
  } else {
    std::ostringstream error_message;
    error_message << "Just two input files, evfile and infile, are required";
    sptk::PrintErrorMessage("pcas", error_message);
    return 1;
  }

  // Read mean vector and eigenvector matrix.
  sptk::Matrix mean_vector(vector_length, 1);
  sptk::Matrix eigenvector_matrix(num_principal_component, vector_length);
  {
    std::ifstream ifs;
    ifs.open(eigenvectors_file, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
      std::ostringstream error_message;
      error_message << "Cannot open file " << eigenvectors_file;
      sptk::PrintErrorMessage("pcas", error_message);
      return 1;
    }

    if (!ReadStream(&mean_vector, &ifs)) {
      std::ostringstream error_message;
      error_message << "Cannot read mean vector";
      sptk::PrintErrorMessage("pcas", error_message);
      return 1;
    }

    if (!ReadStream(&eigenvector_matrix, &ifs)) {
      std::ostringstream error_message;
      error_message << "Cannot read eigenvectors";
      sptk::PrintErrorMessage("pcas", error_message);
      return 1;
    }
  }

  std::ifstream ifs;
  ifs.open(input_file, std::ios::in | std::ios::binary);
  if (ifs.fail() && NULL != input_file) {
    std::ostringstream error_message;
    error_message << "Cannot open file " << input_file;
    sptk::PrintErrorMessage("pcas", error_message);
    return 1;
  }
  std::istream& input_stream(ifs.fail() ? std::cin : ifs);

  sptk::Matrix input_vector(vector_length, 1);
  while (sptk::ReadStream(&input_vector, &input_stream)) {
    if (!sptk::WriteStream(eigenvector_matrix * (input_vector - mean_vector),
                           &std::cout)) {
      std::ostringstream error_message;
      error_message << "Failed to write principal component score";
      sptk::PrintErrorMessage("pcas", error_message);
      return 1;
    }
  }

  return 0;
}
