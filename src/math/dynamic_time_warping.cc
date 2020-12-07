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

#include "SPTK/math/dynamic_time_warping.h"

#include <algorithm>  // std::reverse
#include <cfloat>     // DBL_MAX
#include <utility>    // std::make_pair

namespace {

struct Cell {
  double score;
  int horizontal_back_pointer;
  int vertical_back_pointer;
};

}  // namespace

namespace sptk {

DynamicTimeWarping::DynamicTimeWarping(
    int num_order, LocalPathConstraints local_path_constraint,
    DistanceCalculation::DistanceMetrics distance_metric)
    : num_order_(num_order),
      local_path_constraint_(local_path_constraint),
      distance_calculation_(num_order_, distance_metric),
      is_valid_(true) {
  if (num_order_ < 0 || !distance_calculation_.IsValid()) {
    is_valid_ = false;
    return;
  }

  switch (local_path_constraint_) {
    case kType1: {
      local_path_candidates_.push_back(std::make_pair(1, 0));
      local_path_candidates_.push_back(std::make_pair(0, 1));
      break;
    }
    case kType2: {
      local_path_candidates_.push_back(std::make_pair(1, 0));
      local_path_candidates_.push_back(std::make_pair(0, 1));
      local_path_candidates_.push_back(std::make_pair(1, 1));
      break;
    }
    case kType3: {
      local_path_candidates_.push_back(std::make_pair(1, 0));
      local_path_candidates_.push_back(std::make_pair(1, 1));
      break;
    }
    case kType4: {
      local_path_candidates_.push_back(std::make_pair(1, 0));
      local_path_candidates_.push_back(std::make_pair(1, 1));
      local_path_candidates_.push_back(std::make_pair(1, 2));
      break;
    }
    case kType5: {
      local_path_candidates_.push_back(std::make_pair(1, 0));
      local_path_candidates_.push_back(std::make_pair(0, 1));
      local_path_candidates_.push_back(std::make_pair(1, 1));
      break;
    }
    case kType6: {
      local_path_candidates_.push_back(std::make_pair(1, 1));
      local_path_candidates_.push_back(std::make_pair(1, 2));
      local_path_candidates_.push_back(std::make_pair(2, 1));
      break;
    }
    case kType7: {
      local_path_candidates_.push_back(std::make_pair(1, 0));
      local_path_candidates_.push_back(std::make_pair(1, 1));
      local_path_candidates_.push_back(std::make_pair(1, 2));
      break;
    }
    default: {
      is_valid_ = false;
      return;
    }
  }

  const int num_candidate(local_path_candidates_.size());
  local_path_weights_.resize(num_candidate);
  for (int k(0); k < num_candidate; ++k) {
    local_path_weights_[k] =
        local_path_candidates_[k].first + local_path_candidates_[k].second;
  }
}

bool DynamicTimeWarping::Run(
    const std::vector<std::vector<double> >& query_vector_sequence,
    const std::vector<std::vector<double> >& reference_vector_sequence,
    std::vector<std::pair<int, int> >* viterbi_path,
    double* total_score) const {
  // check inputs
  if (!is_valid_ || query_vector_sequence.empty() ||
      reference_vector_sequence.empty() || NULL == viterbi_path ||
      NULL == total_score) {
    return false;
  }

  const int num_candidate(local_path_candidates_.size());
  const int num_query_vector(query_vector_sequence.size());
  const int num_reference_vector(reference_vector_sequence.size());

  std::vector<std::vector<Cell> > cell(num_query_vector,
                                       std::vector<Cell>(num_reference_vector));
  std::vector<std::vector<Cell> > cell_for_skip_transition(
      num_query_vector, std::vector<Cell>(num_reference_vector));

  const bool includes_skip_transition(
      (kType5 == local_path_constraint_ || kType7 == local_path_constraint_)
          ? true
          : false);

  for (int i(0); i < num_query_vector; ++i) {
    for (int j(0); j < num_reference_vector; ++j) {
      double local_distance;
      if (!distance_calculation_.Run(query_vector_sequence[i],
                                     reference_vector_sequence[j],
                                     &local_distance)) {
        return false;
      }

      double best_score_of_all_paths((0 == i && 0 == j) ? local_distance
                                                        : DBL_MAX);
      int best_i_of_all_paths(-1), best_j_of_all_paths(-1);

      double best_score_of_diagonal_paths(DBL_MAX);
      int best_i_of_diagonal_paths(-1), best_j_of_diagonal_paths(-1);

      for (int k(0); k < num_candidate; ++k) {
        const int i_k(i - local_path_candidates_[k].first);
        const int j_k(j - local_path_candidates_[k].second);
        if (0 <= i_k && 0 <= j_k) {
          double score;
          if (includes_skip_transition && (i_k == i || j_k == j)) {
            score = local_path_weights_[k] * local_distance +
                    cell_for_skip_transition[i_k][j_k].score;
          } else {
            score =
                local_path_weights_[k] * local_distance + cell[i_k][j_k].score;
          }

          if (includes_skip_transition && (i_k != i && j_k != j) &&
              score < best_score_of_diagonal_paths) {
            best_score_of_diagonal_paths = score;
            best_i_of_diagonal_paths = i_k;
            best_j_of_diagonal_paths = j_k;
          }
          if (score < best_score_of_all_paths) {
            best_score_of_all_paths = score;
            best_i_of_all_paths = i_k;
            best_j_of_all_paths = j_k;
          }
        }
      }

      if (includes_skip_transition) {
        cell_for_skip_transition[i][j].score = best_score_of_diagonal_paths;
        cell_for_skip_transition[i][j].horizontal_back_pointer =
            best_i_of_diagonal_paths;
        cell_for_skip_transition[i][j].vertical_back_pointer =
            best_j_of_diagonal_paths;
      }
      cell[i][j].score = best_score_of_all_paths;
      cell[i][j].horizontal_back_pointer = best_i_of_all_paths;
      cell[i][j].vertical_back_pointer = best_j_of_all_paths;
    }
  }

  if (DBL_MAX == cell[num_query_vector - 1][num_reference_vector - 1].score) {
    return false;
  }

  *total_score = cell[num_query_vector - 1][num_reference_vector - 1].score /
                 (num_query_vector + num_reference_vector);

  {
    bool skip_transition(false);
    int i(num_query_vector - 1);
    int j(num_reference_vector - 1);
    viterbi_path->clear();
    viterbi_path->push_back(std::make_pair(i, j));
    while (0 <= i && 0 <= j) {
      const int prev_i(
          (includes_skip_transition && skip_transition)
              ? cell_for_skip_transition[i][j].horizontal_back_pointer
              : cell[i][j].horizontal_back_pointer);
      const int prev_j(
          (includes_skip_transition && skip_transition)
              ? cell_for_skip_transition[i][j].vertical_back_pointer
              : cell[i][j].vertical_back_pointer);
      if (0 <= prev_i && 0 <= prev_j) {
        viterbi_path->push_back(std::make_pair(prev_i, prev_j));
      }
      skip_transition = (prev_i == i || prev_j == j) ? true : false;
      i = prev_i;
      j = prev_j;
    }
    std::reverse(viterbi_path->begin(), viterbi_path->end());
  }

  return true;
}

}  // namespace sptk
