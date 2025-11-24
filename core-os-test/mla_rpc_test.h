//
// Created by christian on 11/20/25.
//

#ifndef COREOS_MLA_RPC_TEST_H
#define COREOS_MLA_RPC_TEST_H

#include "../core-os/rpc/mla_rpc.h"
#include "../core-os-test-support/mla_test_executor.h"

struct my_rpc_test_input_t {
    mla_int32_t a;
    mla_int32_t b;
};

inline void my_rpc_test_input_t_serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {

    const my_rpc_test_input_t* input = static_cast<const my_rpc_test_input_t*>(obj);
    mla_serializer_write_int32(serializer, mla_string_const("a"), input->a);
    mla_serializer_write_int32(serializer, mla_string_const("b"), input->b);

}

inline mla_deserializer_read_result_t my_rpc_test_input_t_deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name) {

    my_rpc_test_input_t* input = static_cast<my_rpc_test_input_t*>(obj);

    if (mla_string_equals_const(property_name, "a")) {
        mla_deserializer_read_int32(deserializer, input->a);
    } else if (mla_string_equals_const(property_name, "b")) {
        mla_deserializer_read_int32(deserializer, input->b);
    } else {
        return MLA_DESERIALIZER_READ_SKIPPED;
    }

}

struct my_rpc_test_output_t {
    mla_int32_t sum;
};

inline void my_rpc_test_output_t_serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {

    const my_rpc_test_output_t* output = static_cast<const my_rpc_test_output_t*>(obj);
    mla_serializer_write_int32(serializer, mla_string_const("sum"), output->sum);

}

inline mla_deserializer_read_result_t my_rpc_test_output_t_deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name) {

    my_rpc_test_output_t* output = static_cast<my_rpc_test_output_t*>(obj);

    if (mla_string_equals_const(property_name, "sum")) {
        mla_deserializer_read_int32(deserializer, output->sum);
    } else {
        return MLA_DESERIALIZER_READ_SKIPPED;
    }
}

mla_bool_t my_rpc_test_procedure_handler(const my_rpc_test_input_t* input, my_rpc_test_output_t* output) {

    output->sum = input->a + input->b;
    return true;
}

#define mla_rpc_procedure_math_sum_name "math/sum"
#define mla_rpc_procedure_math_sum_signature my_rpc_test_input_t, my_rpc_test_output_t

mla_rpc_auto_register_procedure(mla_rpc_procedure_math_sum_name, my_rpc_test_input_t, my_rpc_test_output_t, my_rpc_test_procedure_handler)


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


void RegisterRpcTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("FindRpcProcedure", test_category, FindRpcProcedureTest);
    mla_test_executor_register_test(p_TestExecutor, test);

    test = mla_test("ExecuteRpcProcedure", test_category, ExecuteRpcProcedureTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}


#endif