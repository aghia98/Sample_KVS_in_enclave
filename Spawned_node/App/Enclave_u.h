#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */

#include "user_types.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OCALL_PRINT_STRING_DEFINED__
#define OCALL_PRINT_STRING_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_string, (const char* str));
#endif

sgx_status_t ecall_put(sgx_enclave_id_t eid, char key[100], char share[410]);
sgx_status_t ecall_get(sgx_enclave_id_t eid, char key[100], char* share);
sgx_status_t ecall_delete(sgx_enclave_id_t eid, char key[100]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
