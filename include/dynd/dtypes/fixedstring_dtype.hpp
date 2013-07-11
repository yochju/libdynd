//
// Copyright (C) 2011-13 Mark Wiebe, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//
// The fixedstring dtype represents a string with
// a particular encoding, stored in a fixed-size
// buffer.
//
#ifndef _DYND__FIXEDSTRING_TYPE_HPP_
#define _DYND__FIXEDSTRING_TYPE_HPP_

#include <dynd/type.hpp>
#include <dynd/dtype_assign.hpp>
#include <dynd/dtypes/view_dtype.hpp>
#include <dynd/string_encodings.hpp>

namespace dynd {

class fixedstring_dtype : public base_string_dtype {
    intptr_t m_stringsize;
    string_encoding_t m_encoding;

public:
    fixedstring_dtype(intptr_t stringsize, string_encoding_t encoding);

    virtual ~fixedstring_dtype();

    string_encoding_t get_encoding() const {
        return m_encoding;
    }

    void get_string_range(const char **out_begin, const char**out_end, const char *metadata, const char *data) const;
    void set_utf8_string(const char *metadata, char *data, assign_error_mode errmode,
                    const char* utf8_begin, const char *utf8_end) const;

    void print_data(std::ostream& o, const char *metadata, const char *data) const;

    void print_dtype(std::ostream& o) const;

    ndt::type get_canonical_type() const;

    bool is_lossless_assignment(const ndt::type& dst_dt, const ndt::type& src_dt) const;

    bool operator==(const base_type& rhs) const;

    void metadata_default_construct(char *DYND_UNUSED(metadata), size_t DYND_UNUSED(ndim), const intptr_t* DYND_UNUSED(shape)) const {
    }
    void metadata_copy_construct(char *DYND_UNUSED(dst_metadata), const char *DYND_UNUSED(src_metadata), memory_block_data *DYND_UNUSED(embedded_reference)) const {
    }
    void metadata_destruct(char *DYND_UNUSED(metadata)) const {
    }
    void metadata_debug_print(const char *DYND_UNUSED(metadata), std::ostream& DYND_UNUSED(o), const std::string& DYND_UNUSED(indent)) const {
    }

    size_t make_assignment_kernel(
                    hierarchical_kernel *out, size_t offset_out,
                    const ndt::type& dst_dt, const char *dst_metadata,
                    const ndt::type& src_dt, const char *src_metadata,
                    kernel_request_t kernreq, assign_error_mode errmode,
                    const eval::eval_context *ectx) const;

    size_t make_comparison_kernel(
                    hierarchical_kernel *out, size_t offset_out,
                    const ndt::type& src0_dt, const char *src0_metadata,
                    const ndt::type& src1_dt, const char *src1_metadata,
                    comparison_type_t comptype,
                    const eval::eval_context *ectx) const;
};

inline ndt::type make_fixedstring_dtype(intptr_t stringsize,
                string_encoding_t encoding = string_encoding_utf_8) {
    return ndt::type(new fixedstring_dtype(stringsize, encoding), false);
}

} // namespace dynd

#endif // _DYND__FIXEDSTRING_TYPE_HPP_
