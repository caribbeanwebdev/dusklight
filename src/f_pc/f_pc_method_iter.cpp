/**
 * f_pc_method_iter.cpp
 * Framework - Process Method Iteration
 */

#include "SSystem/SComponent/c_list_iter.h"
#include "f_pc/f_pc_method_iter.h"

// The list iterator dispatch invokes with (node, data); fpcMtdIt_MethodFunc
// takes one arg, so calling it through the casted type traps on wasm. Route
// the real callback through the otherwise-unused data slot instead.
static int fpcMtdIt_Call(node_class* i_node, void* i_method) {
    return ((fpcMtdIt_MethodFunc)i_method)(i_node);
}

int fpcMtdIt_Method(node_list_class* i_nodeList, fpcMtdIt_MethodFunc i_methods) {
    return cLsIt_Method(i_nodeList, fpcMtdIt_Call, (void*)i_methods);
}
