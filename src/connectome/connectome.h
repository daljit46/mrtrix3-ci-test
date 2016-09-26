/*
 * Copyright (c) 2008-2016 the MRtrix3 contributors
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/
 * 
 * MRtrix is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * For more details, see www.mrtrix.org
 * 
 */



#ifndef __connectome_connectome_h__
#define __connectome_connectome_h__


#include <Eigen/Dense>

#include "app.h"
#include "exception.h"
#include "mrtrix.h"
#include "types.h"


namespace MR {
  namespace Connectome {



    typedef uint32_t node_t;

    typedef default_type value_type;
    typedef Eigen::Array<value_type, Eigen::Dynamic, Eigen::Dynamic> matrix_type;
    typedef Eigen::Array<value_type, Eigen::Dynamic, 1> vector_type;
    typedef Eigen::Array<bool, Eigen::Dynamic, Eigen::Dynamic> mask_type;



    extern const App::OptionGroup MatrixOutputOptions;



    template <class MatrixType>
    void check (const MatrixType& in, const node_t num_nodes)
    {
      if (in.rows() != in.cols())
        throw Exception ("Connectome matrix is not square (" + str(in.rows()) + " x " + str(in.cols()) + ")");
      if (in.rows() != num_nodes)
        throw Exception ("Connectome matrix contains " + str(in.rows()) + " nodes; expected " + str(num_nodes));
    }



    template <class MatrixType>
    bool is_directed (MatrixType& in)
    {
      if (in.rows() != in.cols())
        throw Exception ("Connectome matrix is not square (" + str(in.rows()) + " x " + str(in.cols()) + ")");

      for (node_t row = 0; row != in.rows(); ++row) {
        for (node_t col = row+1; col != in.cols(); ++col) {

          const typename MatrixType::Scalar lower_value = in (col, row);
          const typename MatrixType::Scalar upper_value = in (row, col);

          if (upper_value && lower_value && (upper_value != lower_value))
            return true;

      } }

      return false;
    }



    template <class MatrixType>
    void to_symmetric (MatrixType& in)
    {
      if (is_directed (in))
        throw Exception ("Cannot convert a non-symmetric directed matrix to be symmetric");

      for (node_t row = 0; row != in.rows(); ++row) {
        for (node_t col = row+1; col != in.cols(); ++col) {

          const typename MatrixType::Scalar lower_value = in (col, row);
          const typename MatrixType::Scalar upper_value = in (row, col);

          if (upper_value && !lower_value)
            in (row, col) = in (col, row) = upper_value;
          else if (lower_value && !upper_value)
            in (row, col) = in (col, row) = lower_value;

      } }
    }



    template <class MatrixType>
    void to_upper (MatrixType& in)
    {
      if (is_directed (in))
        throw Exception ("Cannot convert a non-symmetric directed matrix to upper triangular");

      for (node_t row = 0; row != in.rows(); ++row) {
        for (node_t col = row+1; col != in.cols(); ++col) {

          const typename MatrixType::Scalar lower_value = in (col, row);
          const typename MatrixType::Scalar upper_value = in (row, col);

          if (!upper_value && lower_value)
            in (row, col) = lower_value;
          in (col, row) = typename MatrixType::Scalar(0);

      } }
    }



  }
}


#endif

