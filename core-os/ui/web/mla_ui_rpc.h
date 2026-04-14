//
// Created by christian on 12/5/25.
//

#ifndef COREOS_MLA_UI_RPC_H
#define COREOS_MLA_UI_RPC_H

#include "../../rpc/mla_rpc.h"
#include "../../serializer/mla_serializer.h"



#define mla_rpc_procedure_ui_surfaces_name "ui/surfaces"
#define mla_rpc_procedure_ui_surfaces_signature void, mla_ui_rpc_surface_infos_t
mla_bool_t mla_ui_rpc_get_surfaces_handler(const mla_platform_pointer_t input, mla_ui_rpc_surface_infos_t* output);



#endif