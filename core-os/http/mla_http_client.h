//
// Created by christian on 10/6/25.
//

#ifndef COREOS_MLA_HTTP_CLIENT_H
#define COREOS_MLA_MLA_HTTP_CLIENT_H

#include "mla_http_request.h"
#include "mla_http_response.h"

enum mla_http_client_response_status: mla_uint8_t {
    MLA_HTTP_CLIENT_RESPONSE_STATUS_OK,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_WRONG_PROTOCOL,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN_HOST,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_CONNECTION_FAILED,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_TIMEOUT,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_INVALID_RESPONSE,
    MLA_HTTP_CLIENT_RESPONSE_STATUS_ERROR_UNKNOWN
};

struct mla_http_client_t {

};

struct mla_http_client_response {
    mla_http_client_response_status status;
    mla_string_t errorMessage; // Present if status is not OK
    mla_http_response_t response;

};

mla_http_client_t mla_http_client();

mla_http_client_response mla_http_client_send_request(mla_http_client_t &p_Client, const mla_http_request_t &p_Request);


////////////////////////////////////////////////////////////////
/// Helpers
////////////////////////////////////////////////////////////////

inline mla_http_response_t mla_http_client_send_request(const mla_http_request_t &p_Request) {

    mla_http_client_t client = mla_http_client();
    return mla_http_client_send_request(client, p_Request);
}


#endif
