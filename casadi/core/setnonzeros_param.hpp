/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010-2014 Joel Andersson, Joris Gillis, Moritz Diehl,
 *                            K.U. Leuven. All rights reserved.
 *    Copyright (C) 2011-2014 Greg Horn
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef CASADI_SETNONZEROS_PARAM_HPP
#define CASADI_SETNONZEROS_PARAM_HPP

#include "mx_node.hpp"
#include <map>
#include <stack>

/// \cond INTERNAL

namespace casadi {

  /** \brief Assign or add entries to a matrix, parametrically
      \author Joris Gillis
      \date 2019
  */
  template<bool Add>
  class CASADI_EXPORT SetNonzerosParam : public MXNode {
  public:
    ///@{
    /** \brief Create functions
    *   returns y with y[nz]+=x
    */
    ///@{
    static MX create(const MX& y, const MX& x, const MX& nz);
    static MX create(const MX& y, const MX& x, const Slice& s, const MX& nz_offset);
    static MX create(const MX& y, const MX& x, const Slice& inner, const Slice& outer, const MX& nz_offset);
    ///@}

    /// Constructor
    SetNonzerosParam(const MX& y, const MX& x, const MX& nz);

    /// Destructor
    ~SetNonzerosParam() override = 0;

    /** \brief  Evaluate symbolically (MX) */
    void eval_mx(const std::vector<MX>& arg, std::vector<MX>& res) const override;

    /** \brief Calculate forward mode directional derivatives */
    void ad_forward(const std::vector<std::vector<MX> >& fseed,
                         std::vector<std::vector<MX> >& fsens) const override;

    /** \brief Calculate reverse mode directional derivatives */
    void ad_reverse(const std::vector<std::vector<MX> >& aseed,
                         std::vector<std::vector<MX> >& asens) const override;

    /** \brief Get the operation */
    casadi_int op() const override { return Add ? OP_ADDNONZEROS_PARAM : OP_SETNONZEROS_PARAM;}

    /// Can the operation be performed inplace (i.e. overwrite the result)
    casadi_int n_inplace() const override { return 1;}

    /** \brief Deserialize with type disambiguation */
    static MXNode* deserialize(DeserializingStream& s);

  protected:
    /** \brief Deserializing constructor */
    explicit SetNonzerosParam(DeserializingStream& s) : MXNode(s) {}
  };


    /** \brief Add the nonzeros of a matrix to another matrix, parametrically
      \author Joris Gillis
      \date 2019
  */
  template<bool Add>
  class CASADI_EXPORT SetNonzerosParamVector : public SetNonzerosParam<Add>{
  public:

    /// Constructor
    SetNonzerosParamVector(const MX& y, const MX& x, const MX& nz);

    /// Destructor
    ~SetNonzerosParamVector() override {}

    /// Evaluate the function numerically
    int eval(const double** arg, double** res, casadi_int* iw, double* w) const override;

    /** \brief  Propagate sparsity forward */
    int sp_forward(const bvec_t** arg, bvec_t** res, casadi_int* iw, bvec_t* w) const override;

    /** \brief  Propagate sparsity backwards */
    int sp_reverse(bvec_t** arg, bvec_t** res, casadi_int* iw, bvec_t* w) const override;

    /** \brief  Print expression */
    std::string disp(const std::vector<std::string>& arg) const override;

    /** \brief Generate code for the operation */
    void generate(CodeGenerator& g,
                  const std::vector<casadi_int>& arg,
                  const std::vector<casadi_int>& res) const override;

    /** \brief Serialize an object without type information */
    void serialize_body(SerializingStream& s) const override;
    /** \brief Serialize type information */
    void serialize_type(SerializingStream& s) const override;

    /** \brief Deserializing constructor */
    explicit SetNonzerosParamVector(DeserializingStream& s);
  };

  // Specialization of the above when nz_ is a Slice
  template<bool Add>
  class CASADI_EXPORT SetNonzerosParamSlice : public SetNonzerosParam<Add>{
  public:

    /// Constructor
    SetNonzerosParamSlice(const MX& y, const MX& x, const Slice& s, const MX& nz_offset) :
      SetNonzerosParam<Add>(y, x, nz_offset), s_(s) {}

    /// Destructor
    ~SetNonzerosParamSlice() override {}

    /** \brief  Propagate sparsity forward */
    int sp_forward(const bvec_t** arg, bvec_t** res, casadi_int* iw, bvec_t* w) const override;

    /** \brief  Propagate sparsity backwards */
    int sp_reverse(bvec_t** arg, bvec_t** res, casadi_int* iw, bvec_t* w) const override;

    /// Evaluate the function numerically
    int eval(const double** arg, double** res, casadi_int* iw, double* w) const override;

    /** \brief  Print expression */
    std::string disp(const std::vector<std::string>& arg) const override;

    /** \brief Generate code for the operation */
    void generate(CodeGenerator& g,
                  const std::vector<casadi_int>& arg,
                  const std::vector<casadi_int>& res) const override;

    // Data member
    Slice s_;

    /** \brief Serialize an object without type information */
    void serialize_body(SerializingStream& s) const override;
    /** \brief Serialize type information */
    void serialize_type(SerializingStream& s) const override;

    /** \brief Deserializing constructor */
    explicit SetNonzerosParamSlice(DeserializingStream& s);
  };

  // Specialization of the above when nz_ is a nested Slice
  template<bool Add>
  class CASADI_EXPORT SetNonzerosParamSlice2 : public SetNonzerosParam<Add>{
  public:

    /// Constructor
    SetNonzerosParamSlice2(const MX& y, const MX& x, const Slice& inner, const Slice& outer, const MX& nz_offset) :
        SetNonzerosParam<Add>(y, x, nz_offset), inner_(inner), outer_(outer) {}

    /// Destructor
    ~SetNonzerosParamSlice2() override {}

    /** \brief  Propagate sparsity forward */
    int sp_forward(const bvec_t** arg, bvec_t** res, casadi_int* iw, bvec_t* w) const override;

    /** \brief  Propagate sparsity backwards */
    int sp_reverse(bvec_t** arg, bvec_t** res, casadi_int* iw, bvec_t* w) const override;

    /// Evaluate the function numerically
    int eval(const double** arg, double** res, casadi_int* iw, double* w) const override;

    /** \brief  Print expression */
    std::string disp(const std::vector<std::string>& arg) const override;

    /** \brief Generate code for the operation */
    void generate(CodeGenerator& g,
                  const std::vector<casadi_int>& arg,
                  const std::vector<casadi_int>& res) const override;

    // Data members
    Slice inner_, outer_;

    /** \brief Serialize an object without type information */
    void serialize_body(SerializingStream& s) const override;
    /** \brief Serialize type information */
    void serialize_type(SerializingStream& s) const override;

    /** \brief Deserializing constructor */
    explicit SetNonzerosParamSlice2(DeserializingStream& s);
  };

} // namespace casadi
/// \endcond

#endif // CASADI_SETNONZEROS_PARAM_HPP
