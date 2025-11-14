//
// Created by chris on 11/14/2025.
//

#ifndef COREOS_MLA_WEBSOCKET_CLIENT_TEST_H
#define COREOS_MLA_WEBSOCKET_CLIENT_TEST_H

#include "../core-os/http/mla_websocket_client.h"
#include "../core-os-test-support/mla_test_executor.h"

void SimpleLocalEchoTest() {

    mla_websocket_client_t ws_client = mla_websocket_client_invalid();

    assert_true(mla_websocket_client_connect(ws_client, mla_string("ws://localhost:1337"), 10000), "Should connect to WebSocket server");
    assert_true(mla_websocket_client_is_connected(ws_client), "WebSocket client should be connected");

    assert_true(mla_websocket_client_send_text_message(ws_client, mla_string("Hello, WebSocket!"), true), "Should send text message");

    mla_websocket_text_message_t textMessage = mla_websocket_text_message_empty();
    mla_websocket_binary_message_t binaryMessage = mla_websocket_binary_message_empty();

    mla_websocket_client_message_receive_type_t result = mla_websocket_client_receive_message(ws_client, 10000, textMessage, binaryMessage);
    assert_equal((mla_uint8_t)result, MLA_WEBSOCKET_CLIENT_MESSAGE_RECEIVE_TYPE_TEXT, "Should receive text message");

    assert_struct_equal(mla_string_t, textMessage.message, mla_string("Hello, WebSocket!"), "Received message should match sent message");

    assert_true(mla_websocket_client_disconnect(ws_client), "Should disconnect from WebSocket server");
    assert_false(mla_websocket_client_is_connected(ws_client), "WebSocket client should be disconnected");

}

void RegisterWebsocketClientTests(mla_test_executor_t &p_TestExecutor) {
    (void)p_TestExecutor;
    //mla_test_t test = mla_test("SimpleLocalEcho", test_category, SimpleLocalEchoTest);
    //mla_test_executor_register_test(p_TestExecutor, test);

}

#endif