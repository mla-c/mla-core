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

mla_declare_serializable(my_rpc_test_input_t)
mla_declare_serializable(my_rpc_test_output_t)
#define mla_rpc_procedure_math_sum_name "math/sum"
#define mla_rpc_procedure_math_sum_signature my_rpc_test_input_t, my_rpc_test_output_t

mla_rpc_auto_register_procedure(mla_rpc_procedure_math_sum_name, my_rpc_test_input_t, my_rpc_test_output_t, my_rpc_test_procedure_handler)


/// void-like input, normal output
struct my_rpc_void_in_output_t {
    mla_int32_t value;
};

inline void my_rpc_void_in_output_t_serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {
    const auto* o = static_cast<const my_rpc_void_in_output_t*>(obj);
    mla_serializer_write_int32(serializer, mla_string_const("value"), o->value);
}

inline mla_deserializer_read_result_t my_rpc_void_in_output_t_deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name) {
    auto* o = static_cast<my_rpc_void_in_output_t*>(obj);
    if (mla_string_equals_const(property_name, "value")) {
        mla_deserializer_read_int32(deserializer, o->value);
    }
    return MLA_DESERIALIZER_READ_SKIPPED;
}

mla_declare_serializable(my_rpc_void_in_output_t)


/// normal input, void-like output
struct my_rpc_input_void_out_t {
    mla_int32_t inputValue;
};

inline void my_rpc_input_void_out_t_serialize(mla_serializer_t& serializer, const mla_pointer_t obj) {
    const auto* in = static_cast<const my_rpc_input_void_out_t*>(obj);
    mla_serializer_write_int32(serializer, mla_string_const("inputValue"), in->inputValue);
}

inline mla_deserializer_read_result_t my_rpc_input_void_out_t_deserialize(mla_deserializer_t& deserializer, mla_pointer_t obj, const mla_string_t& property_name) {
    auto* in = static_cast<my_rpc_input_void_out_t*>(obj);
    if (mla_string_equals_const(property_name, "inputValue")) {
        mla_deserializer_read_int32(deserializer, in->inputValue);
    }
    return MLA_DESERIALIZER_READ_SKIPPED;
}

mla_declare_serializable(my_rpc_input_void_out_t)


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