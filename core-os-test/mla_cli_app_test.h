//
// Created by christian on 9/13/25.
//

#ifndef COREOS_MLA_CLI_APP_TEST_H
#define COREOS_MLA_CLI_APP_TEST_H

#include "../core-os-test-support/mla_test_executor.h"
#include "../core-os/cli/mla_cli_app_t.h"
#include "../core-os/system/mla_stream.h"

inline void SimpleNavigationTest() {

    mla_cli_module_t root = mla_cli_module("Root");
    mla_cli_module_t subModule = mla_cli_module("SubModule1");
    mla_cli_module_t subSubModule = mla_cli_module("SubSubModule1");
    mla_array_list_add(subModule.subModules, subSubModule);

    mla_array_list_add(root.subModules, subModule);

    mla_cli_module_t subModule2 = mla_cli_module("SubModule2");
    mla_array_list_add(root.subModules, subModule2);

    mla_cli_app_t app = mla_cli_app_init(root, mla_stream_noop_output());
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should start with root module active");
    assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");

    // Process no input
    mla_cli_app_update_and_process_input(app,mla_stream_noop_input() , mla_stream_noop_output());
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should start with root module active");
    assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");

    mla_string_t buffer = mla_string("SubModule1\n");
    mla_cli_app_update_and_process_input(app, mla_stream_input_from_buffer((mla_byte_t*)buffer.data, buffer.length) , mla_stream_noop_output());
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)2, "App should have navigated to SubModule1");
    assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("SubModule1"), "Active module should be SubModule1");

    buffer = mla_string("SubSubModule1\n");
    mla_cli_app_update_and_process_input(app, mla_stream_input_from_buffer((mla_byte_t*)buffer.data, buffer.length) , mla_stream_noop_output());
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)3, "App should have navigated to SubSubModule1");
    assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 2)->moduleName, mla_string("SubSubModule1"), "Active module should be SubSubModule1");

    buffer = mla_string("exit\n");
    mla_cli_app_update_and_process_input(app, mla_stream_input_from_buffer((mla_byte_t*)buffer.data, buffer.length) , mla_stream_noop_output());
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)2, "App should have navigated back to SubModule1");
    assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 1)->moduleName, mla_string("SubModule1"), "Active module should be SubModule1");

    buffer = mla_string("exit\n");
    mla_cli_app_update_and_process_input(app, mla_stream_input_from_buffer((mla_byte_t*)buffer.data, buffer.length) , mla_stream_noop_output());
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should have navigated back to Root");
    assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");

    buffer = mla_string("exit\n");
    mla_cli_app_update_and_process_input(app, mla_stream_input_from_buffer((mla_byte_t*)buffer.data, buffer.length) , mla_stream_noop_output());
    assert_equal(mla_array_list_size(app.activeModules), (mla_size_t)1, "App should have not navigated");
    assert_struct_equal(mla_string_t, mla_array_list_get_ref(app.activeModules, 0)->moduleName, mla_string("Root"), "Active module should be Root");


}

inline void RegisterCliAppTests(mla_test_executor_t &p_TestExecutor) {

    mla_test_t test = mla_test("SimpleNavigation", test_category, SimpleNavigationTest);
    mla_test_executor_register_test(p_TestExecutor, test);

}

#endif
