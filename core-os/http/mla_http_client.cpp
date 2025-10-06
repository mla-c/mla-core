//
// Created by christian on 10/6/25.
//

#include "mla_http_client.h"

#include "../network/mla_network.h"

mla_http_client_t mla_http_client() {
    return {};
}


mla_bool_t __mla_http_client_resolve_host(mla_http_client_response& response, const mla_string_t & url, mla_network_host & host) {

}

mla_http_client_response mla_http_client_send_request(mla_http_client_t &p_Client, const mla_http_request_t &p_Request) {

    mla_http_client_response response = { MLA_HTTP_CLIENT_RESPONSE_STATUS_OK };

    mla_network_host host;

    if (!__mla_http_client_resolve_host(response, p_Request.url, host)) {
        return response;
    }

    mla_stream_output_t connection = mla_stream_noop_output();

    if (!__mla_http_client_connect(response, host, connection)) {
        return response;
    }





}
