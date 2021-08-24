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

#include "SPTK/compression/linde_buzo_gray_algorithm.h"

#include <cfloat>   // DBL_MAX
#include <cmath>    // std::fabs
#include <cstddef>  // std::size_t

#include "SPTK/generation/normal_distributed_random_value_generation.h"

namespace sptk {

LindeBuzoGrayAlgorithm::LindeBuzoGrayAlgorithm(
    int num_order, int initial_codebook_size, int target_codebook_size,
    int min_num_vector_in_cluster, int num_iteration,
    double convergence_threshold, double splitting_factor, int seed)
    : num_order_(num_order),
      initial_codebook_size_(initial_codebook_size),
      target_codebook_size_(target_codebook_size),
      min_num_vector_in_cluster_(min_num_vector_in_cluster),
      num_iteration_(num_iteration),
      convergence_threshold_(convergence_threshold),
      splitting_factor_(splitting_factor),
      seed_(seed),
      distance_calculation_(
          num_order_, DistanceCalculation::DistanceMetrics::kSquaredEuclidean),
      statistics_accumulation_(num_order_, 1),
      vector_quantization_(num_order_),
      is_valid_(true) {
  if (num_order_ < 0 || initial_codebook_size_ <= 0 ||
      target_codebook_size_ <= initial_codebook_size_ ||
      min_num_vector_in_cluster_ <= 0 || num_iteration_ <= 0 ||
      convergence_threshold_ < 0.0 || splitting_factor_ <= 0.0 ||
      !distance_calculation_.IsValid() || !statistics_accumulation_.IsValid() ||
      !vector_quantization_.IsValid()) {
    is_valid_ = false;
    return;
  }
}

bool LindeBuzoGrayAlgorithm::Run(
    const std::vector<std::vector<double> >& input_vectors,
    std::vector<std::vector<double> >* codebook_vectors,
    std::vector<int>* codebook_indices) const {
  // Check inputs.
  const int num_input_vector(static_cast<int>(input_vectors.size()));
  if (!is_valid_ ||
      num_input_vector < min_num_vector_in_cluster_ * target_codebook_size_ ||
      NULL == codebook_vectors || NULL == codebook_indices ||
      codebook_vectors->size() !=
          static_cast<std::size_t>(initial_codebook_size_)) {
    return false;
  }

  // Prepare memories.
  if (codebook_indices->size() != static_cast<std::size_t>(num_input_vector)) {
    codebook_indices->resize(num_input_vector);
  }
  std::vector<StatisticsAccumulation::Buffer> buffers(target_codebook_size_);

  // Prepare random value generator.
  NormalDistributedRandomValueGeneration random_value_generation(seed_);

  // Design codebook.
  int current_codebook_size(initial_codebook_size_);
  for (int next_codebook_size(current_codebook_size * 2);
       next_codebook_size <= target_codebook_size_; next_codebook_size *= 2) {
    // Double codebook size.
    codebook_vectors->resize(next_codebook_size);
    for (std::vector<std::vector<double> >::iterator itr(
             codebook_vectors->begin() + current_codebook_size);
         itr != codebook_vectors->end(); ++itr) {
      itr->resize(num_order_ + 1);
    }

    // Perturb codebook vectors.
    for (int i(0); i < current_codebook_size; ++i) {
      for (int m(0); m <= num_order_; ++m) {
        double random_value;
        if (!random_value_generation.Get(&random_value)) {
          return false;
        }
        const double perturbation(splitting_factor_ * random_value);
        const int j(i + current_codebook_size);
        (*codebook_vectors)[j][m] = (*codebook_vectors)[i][m] - perturbation;
        (*codebook_vectors)[i][m] = (*codebook_vectors)[i][m] + perturbation;
      }
    }

    current_codebook_size = next_codebook_size;

    double prev_total_distance(DBL_MAX);
    for (int n(0); n < num_iteration_; ++n) {
      // Initialize.
      double total_distance(0.0);
      for (int i(0); i < current_codebook_size; ++i) {
        statistics_accumulation_.Clear(&(buffers[i]));
      }

      // Accumulate statistics (E-step).
      for (int t(0); t < num_input_vector; ++t) {
        int index;
        if (!vector_quantization_.Run(input_vectors[t], *codebook_vectors,
                                      &index)) {
          return false;
        }
        (*codebook_indices)[t] = index;

        if (!statistics_accumulation_.Run(input_vectors[t],
                                          &(buffers[index]))) {
          return false;
        }

        double distance;
        if (!distance_calculation_.Run(input_vectors[t],
                                       (*codebook_vectors)[index], &distance)) {
          return false;
        }
        total_distance += distance;
      }
      total_distance /= num_input_vector;

      // Check convergence.
      const double criterion_value(
          std::fabs(prev_total_distance - total_distance) / total_distance);
      if (0.0 == total_distance || criterion_value < convergence_threshold_) {
        break;
      }
      prev_total_distance = total_distance;

      // Update codebook (M-step) and find a maximum cluster.
      int majority_index(-1);
      int max_num_vector_in_cluster(0);
      for (int i(0); i < current_codebook_size; ++i) {
        int num_vector;
        if (!statistics_accumulation_.GetNumData(buffers[i], &num_vector)) {
          return false;
        }

        if (max_num_vector_in_cluster < num_vector) {
          majority_index = i;
          max_num_vector_in_cluster = num_vector;
        }

        // Update if the cluster contains enough data.
        if (min_num_vector_in_cluster_ <= num_vector) {
          if (!statistics_accumulation_.GetMean(buffers[i],
                                                &((*codebook_vectors)[i]))) {
            return false;
          }
        }
      }

      // Update the remaining centroids.
      for (int i(0); i < current_codebook_size; ++i) {
        int num_vector;
        if (!statistics_accumulation_.GetNumData(buffers[i], &num_vector)) {
          return false;
        }

        if (num_vector < min_num_vector_in_cluster_) {
          for (int m(0); m <= num_order_; ++m) {
            double random_value;
            if (!random_value_generation.Get(&random_value)) {
              return false;
            }
            const double perturbation(splitting_factor_ * random_value);
            const int j(majority_index);
            (*codebook_vectors)[i][m] =
                (*codebook_vectors)[j][m] - perturbation;
            (*codebook_vectors)[j][m] =
                (*codebook_vectors)[j][m] + perturbation;
          }
        }
      }
    }
  }

  // Save final results.
  for (int t(0); t < num_input_vector; ++t) {
    if (!vector_quantization_.Run(input_vectors[t], *codebook_vectors,
                                  &((*codebook_indices)[t]))) {
      return false;
    }
  }

  return true;
}

}  // namespace sptk
