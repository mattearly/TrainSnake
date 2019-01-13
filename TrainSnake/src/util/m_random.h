#pragma once

/// to export documentation, use doxygen

#include <random>
#include <chrono>

namespace mearly {

  ///
  /// \brief ZTOR Zero-To-One-Random uses MersenneTwister seeded on a high precision
  /// timestamp seed for a true feeling of randomness
  /// \return double between 0.0 and 1.0
  ///
  double ZTOR();

  ///
  /// \brief ZOOR Zero-Or-One-Random
  /// \return 0 or 1
  ///
  int ZOOR();

  ///
  /// \brief NTKR N-To-K-Random
  /// \return an random integer in the range N to K
  ///
  int NTKR(int n, int k);

  ///
  /// \brief NTKR N-To-K-Random
  /// \return an random float in the range N to K
  ///
  float NTRK(float n, float k);

}  //end namespace mearly