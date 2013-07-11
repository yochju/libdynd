//
// Copyright (C) 2011-13 Mark Wiebe, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//
// The conversion dtype represents one dtype viewed
// as another buffering based on the casting mechanism.
//
// This dtype takes on the characteristics of its storage dtype
// through the dtype interface, except for the "kind" which
// is expression_kind to signal that the value_type must be examined.
//
#ifndef _DYND__CONVERT_TYPE_HPP_
#define _DYND__CONVERT_TYPE_HPP_

#include <dynd/type.hpp>
#include <dynd/dtype_assign.hpp>

namespace dynd {

class convert_dtype : public base_expression_dtype {
    ndt::type m_value_type, m_operand_type;
    assign_error_mode m_errmode;
    // These error modes may be set to assign_error_none if the
    // assignment is lossless in that direction.
    assign_error_mode m_errmode_to_value, m_errmode_to_operand;
public:
    convert_dtype(const ndt::type& value_type, const ndt::type& operand_type, assign_error_mode errmode);

    virtual ~convert_dtype();

    const ndt::type& get_value_type() const {
        return m_value_type;
    }
    const ndt::type& get_operand_type() const {
        return m_operand_type;
    }
    void print_data(std::ostream& o, const char *metadata, const char *data) const;

    void print_dtype(std::ostream& o) const;

    void get_shape(size_t ndim, size_t i, intptr_t *out_shape, const char *metadata) const;

    bool is_lossless_assignment(const ndt::type& dst_dt, const ndt::type& src_dt) const;

    bool operator==(const base_type& rhs) const;

    ndt::type with_replaced_storage_type(const ndt::type& replacement_type) const;

    size_t make_operand_to_value_assignment_kernel(
                    hierarchical_kernel *out, size_t offset_out,
                    const char *dst_metadata, const char *src_metadata,
                    kernel_request_t kernreq, const eval::eval_context *ectx) const;
    size_t make_value_to_operand_assignment_kernel(
                    hierarchical_kernel *out, size_t offset_out,
                    const char *dst_metadata, const char *src_metadata,
                    kernel_request_t kernreq, const eval::eval_context *ectx) const;

    // Propagate properties and functions from the value dtype
    void get_dynamic_array_properties(
                    const std::pair<std::string, gfunc::callable> **out_properties,
                    size_t *out_count) const
    {
        if (!m_value_type.is_builtin()) {
            m_value_type.extended()->get_dynamic_array_properties(out_properties, out_count);
        }
    }
    void get_dynamic_array_functions(
                    const std::pair<std::string, gfunc::callable> **out_functions,
                    size_t *out_count) const
    {
        if (!m_value_type.is_builtin()) {
            m_value_type.extended()->get_dynamic_array_functions(out_functions, out_count);
        }
    }
};

/**
 * Makes a conversion dtype to convert from the operand_type to the value_type.
 * If the value_type has expression_kind, it chains operand_type.value_type()
 * into value_type.storage_type().
 */
inline ndt::type make_convert_dtype(const ndt::type& value_type, const ndt::type& operand_type,
                assign_error_mode errmode = assign_error_default) {
    if (operand_type.value_type() != value_type) {
        if (value_type.get_kind() != expression_kind) {
            // Create a conversion dtype when the value kind is different
            return ndt::type(new convert_dtype(value_type, operand_type, errmode), false);
        } else if (value_type.storage_type() == operand_type.value_type()) {
            // No conversion required at the connection
            return static_cast<const base_expression_dtype *>(
                            value_type.extended())->with_replaced_storage_type(operand_type);
        } else {
            // A conversion required at the connection
            return static_cast<const base_expression_dtype *>(
                            value_type.extended())->with_replaced_storage_type(
                                ndt::type(new convert_dtype(
                                    value_type.storage_type(), operand_type, errmode), false));
        }
    } else {
        return operand_type;
    }
}

template<typename Tvalue, typename Tstorage>
ndt::type make_convert_dtype(assign_error_mode errmode = assign_error_default) {
    return ndt::type(new convert_dtype(ndt::make_dtype<Tvalue>(), ndt::make_dtype<Tstorage>(), errmode), false);
}

} // namespace dynd

#endif // _DYND__CONVERT_TYPE_HPP_
