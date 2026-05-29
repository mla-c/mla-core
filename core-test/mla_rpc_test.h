//
// Created by christian on 11/20/25.
//

#ifndef MLA_RPC_TEST_H
#define MLA_RPC_TEST_H

#include "../base-lib/core/rpc/mla_rpc.h"
#include "../base-lib/core-test-support/mla_test_executor.h"

struct my_rpc_test_input_t {
    mla_int32_t a;
    mla_int32_t b;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {

        const my_rpc_test_input_t* input = mla_pointer_get_data<const my_rpc_test_input_t>(obj);

        if (input == nullptr)
            return false;

        mla_serializer_write_int32(serializer, mla_string_const("a"), input->a);
        mla_serializer_write_int32(serializer, mla_string_const("b"), input->b);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {

        my_rpc_test_input_t* input = mla_pointer_get_data<my_rpc_test_input_t>(obj);

        if (input == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "a")) {
            mla_deserializer_read_int32(deserializer, input->a);
        } else if (mla_string_equals_const(property_name, "b")) {
            mla_deserializer_read_int32(deserializer, input->b);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }

    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(my_rpc_test_input_t);
        mla_reflection_struct_field_int32(meta, my_rpc_test_input_t, a);
        mla_reflection_struct_field_int32(meta, my_rpc_test_input_t, b);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

struct my_rpc_test_output_t {
    mla_int32_t sum;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {

        const my_rpc_test_output_t* output = mla_pointer_get_data<const my_rpc_test_output_t>(obj);

        if (output == nullptr)
            return false;

        mla_serializer_write_int32(serializer, mla_string_const("sum"), output->sum);
        return true;

    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {

        my_rpc_test_output_t* output = mla_pointer_get_data<my_rpc_test_output_t>(obj);

        if (output == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "sum")) {
            mla_deserializer_read_int32(deserializer, output->sum);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(my_rpc_test_output_t);
        mla_reflection_struct_field_int32(meta, my_rpc_test_output_t, sum);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

mla_bool_t my_rpc_test_procedure_handler(const my_rpc_test_input_t* input, my_rpc_test_output_t* output) {

    output->sum = input->a + input->b;
    return true;
}

#define mla_rpc_procedure_math_sum_name "math/sum"
#define mla_rpc_procedure_math_sum_signature my_rpc_test_input_t, my_rpc_test_output_t

mla_rpc_auto_register_procedure(mla_rpc_procedure_math_sum_name, my_rpc_test_input_t, my_rpc_test_output_t, my_rpc_test_procedure_handler)


/// void-like input, normal output
struct my_rpc_void_in_output_t {
    mla_int32_t value;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const my_rpc_void_in_output_t* o = mla_pointer_get_data<const my_rpc_void_in_output_t>(obj);

        if (o == nullptr)
            return false;

        mla_serializer_write_int32(serializer, mla_string_const("value"), o->value);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        my_rpc_void_in_output_t* o = mla_pointer_get_data<my_rpc_void_in_output_t>(obj);

        if (o == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "value")) {
            mla_deserializer_read_int32(deserializer, o->value);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(my_rpc_void_in_output_t);
        mla_reflection_struct_field_int32(meta, my_rpc_void_in_output_t, value);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

/// normal input, void-like output
struct my_rpc_input_void_out_t {
    mla_int32_t inputValue;

    static mla_bool_t serialize(mla_serializer_t& serializer, const mla_pointer_t& obj) {
        const my_rpc_input_void_out_t* in = mla_pointer_get_data<const my_rpc_input_void_out_t>(obj);

        if (in == nullptr)
            return false;

        mla_serializer_write_int32(serializer, mla_string_const("inputValue"), in->inputValue);
        return true;
    }

    static mla_deserializer_read_result_t deserialize(mla_deserializer_t& deserializer, mla_pointer_t& obj, const mla_string_t& property_name) {
        my_rpc_input_void_out_t* in = mla_pointer_get_data<my_rpc_input_void_out_t>(obj);

        if (in == nullptr) {
            return MLA_DESERIALIZER_READ_ERROR;
        }

        if (mla_string_equals_const(property_name, "inputValue")) {
            mla_deserializer_read_int32(deserializer, in->inputValue);
        } else {
            return MLA_DESERIALIZER_READ_SKIPPED;
        }
    }

    static mla_reflection_struct_metadata_t metadata() {
        mla_reflection_struct_metadata_t meta = mla_reflection_struct_metadata(my_rpc_input_void_out_t);
        mla_reflection_struct_field_int32(meta, my_rpc_input_void_out_t, inputValue);
        mla_reflection_struct_metadata_freeze(meta);
        return meta;
    }
};

/// handlers

inline mla_bool_t my_rpc_void_in_handler(const void* /*input*/, void* /*output*/) {
    return true;
}

inline mla_bool_t my_rpc_void_in_normal_out_handler(const void* /*input*/, my_rpc_void_in_output_t* output) {
    output->value = 42;
    return true;
}

inline mla_bool_t my_rpc_normal_in_void_out_handler(const my_rpc_input_void_out_t* input, void* /*output*/) {
    // just validate input
    return input->inputValue == 123;
}


#define mla_rpc_procedure_void_in_name "test/void_in"

#define mla_rpc_procedure_void_in_normal_out_name "test/void_in_normal_out"

#define mla_rpc_procedure_normal_in_void_out_name "test/normal_in_void_out"


mla_rpc_auto_register_procedure_void(
    mla_rpc_procedure_void_in_name,
    my_rpc_void_in_handler
)

mla_rpc_auto_register_procedure_void_input(
    mla_rpc_procedure_void_in_normal_out_name,
    my_rpc_void_in_output_t,
    my_rpc_void_in_normal_out_handler
)

mla_rpc_auto_register_procedure_void_output(
    mla_rpc_procedure_normal_in_void_out_name,
    my_rpc_input_void_out_t,
    my_rpc_normal_in_void_out_handler
)


inline void FindRpcProcedureTest() {

    mla_rpc_procedure_safe_t<mla_rpc_procedure_math_sum_signature> proc = mla_rpc_procedure_safe_invalid<mla_rpc_procedure_math_sum_signature>();

    assert_true(mla_rpc_find_procedure<mla_rpc_procedure_math_sum_signature>(mla_string_const(mla_rpc_procedure_math_sum_name), proc), "Failed to find registered RPC procedure");
    assert_false(mla_rpc_find_procedure<mla_rpc_procedure_math_sum_signature>(mla_string_const("nonexistent/procedure"), proc), "Found nonexistent RPC procedure");

}

inline void ExecuteRpcProcedureTest() {

    my_rpc_test_input_t input = { 3, 4 };
    my_rpc_test_output_t output = { 0 };

    assert_true(mla_rpc_execute_procedure(mla_string_const(mla_rpc_procedure_math_sum_name), &input, &output), "Failed to execute RPC procedure");
    assert_equal(output.sum, 7, "RPC procedure returned incorrect sum");

}


inline void ExecuteRpcProcedureVoidInVoidOutTest() {

    assert_true(
        mla_rpc_execute_procedure(
            mla_string_const(mla_rpc_procedure_void_in_name)
        ),
        "Failed to execute void-in/void-out RPC procedure"
    );
}

inline void ExecuteRpcProcedureVoidInNormalOutTest() {
    my_rpc_void_in_output_t output = {0};

    assert_true(
        mla_rpc_execute_procedure_void_input(
            mla_string_const(mla_rpc_procedure_void_in_normal_out_name),
            &output
        ),
        "Failed to execute void-in/normal-out RPC procedure"
    );
    assert_equal(output.value, 42, "RPC procedure returned incorrect value for void-in/normal-out");
}

inline void ExecuteRpcProcedureNormalInVoidOutTest() {
    my_rpc_input_void_out_t input{123};

    assert_true(
        mla_rpc_execute_procedure_void_output(
            mla_string_const(mla_rpc_procedure_normal_in_void_out_name),
            &input
        ),
        "Failed to execute normal-in/void-out RPC procedure"
    );
}



void RegisterRpcTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("FindRpcProcedure", test_category, FindRpcProcedureTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ExecuteRpcProcedure", test_category, ExecuteRpcProcedureTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ExecuteRpcProcedureVoidInVoidOut", test_category, ExecuteRpcProcedureVoidInVoidOutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ExecuteRpcProcedureVoidInNormalOut", test_category, ExecuteRpcProcedureVoidInNormalOutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ExecuteRpcProcedureNormalInVoidOut", test_category, ExecuteRpcProcedureNormalInVoidOutTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}


#endif