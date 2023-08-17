#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */
#include "sgx_lfence.h" /* for sgx_lfence */

#include <errno.h>
#include <mbusafecrt.h> /* for memcpy_s etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_ENCLAVE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_within_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define ADD_ASSIGN_OVERFLOW(a, b) (	\
	((a) += (b)) < (b)	\
)


typedef struct ms_ecall_put_t {
	char* ms_key;
	char* ms_share;
} ms_ecall_put_t;

typedef struct ms_ecall_get_t {
	char* ms_key;
	char* ms_share;
	size_t ms_share_len;
} ms_ecall_get_t;

typedef struct ms_ecall_delete_t {
	char* ms_key;
} ms_ecall_delete_t;

typedef struct ms_ocall_print_string_t {
	const char* ms_str;
} ms_ocall_print_string_t;

static sgx_status_t SGX_CDECL sgx_ecall_put(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_put_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_put_t* ms = SGX_CAST(ms_ecall_put_t*, pms);
	ms_ecall_put_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_put_t), ms, sizeof(ms_ecall_put_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	char* _tmp_key = __in_ms.ms_key;
	size_t _len_key = 100 * sizeof(char);
	char* _in_key = NULL;
	char* _tmp_share = __in_ms.ms_share;
	size_t _len_share = 410 * sizeof(char);
	char* _in_share = NULL;

	CHECK_UNIQUE_POINTER(_tmp_key, _len_key);
	CHECK_UNIQUE_POINTER(_tmp_share, _len_share);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_key != NULL && _len_key != 0) {
		if ( _len_key % sizeof(*_tmp_key) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_key = (char*)malloc(_len_key);
		if (_in_key == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_key, _len_key, _tmp_key, _len_key)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_share != NULL && _len_share != 0) {
		if ( _len_share % sizeof(*_tmp_share) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_share = (char*)malloc(_len_share);
		if (_in_share == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_share, _len_share, _tmp_share, _len_share)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	ecall_put(_in_key, _in_share);

err:
	if (_in_key) free(_in_key);
	if (_in_share) free(_in_share);
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_get(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_get_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_get_t* ms = SGX_CAST(ms_ecall_get_t*, pms);
	ms_ecall_get_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_get_t), ms, sizeof(ms_ecall_get_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	char* _tmp_key = __in_ms.ms_key;
	size_t _len_key = 100 * sizeof(char);
	char* _in_key = NULL;
	char* _tmp_share = __in_ms.ms_share;
	size_t _len_share = __in_ms.ms_share_len ;
	char* _in_share = NULL;

	CHECK_UNIQUE_POINTER(_tmp_key, _len_key);
	CHECK_UNIQUE_POINTER(_tmp_share, _len_share);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_key != NULL && _len_key != 0) {
		if ( _len_key % sizeof(*_tmp_key) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_key = (char*)malloc(_len_key);
		if (_in_key == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_key, _len_key, _tmp_key, _len_key)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	if (_tmp_share != NULL && _len_share != 0) {
		_in_share = (char*)malloc(_len_share);
		if (_in_share == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_share, _len_share, _tmp_share, _len_share)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

		_in_share[_len_share - 1] = '\0';
		if (_len_share != strlen(_in_share) + 1)
		{
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}
	ecall_get(_in_key, _in_share);
	if (_in_share)
	{
		_in_share[_len_share - 1] = '\0';
		_len_share = strlen(_in_share) + 1;
		if (memcpy_verw_s((void*)_tmp_share, _len_share, _in_share, _len_share)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}
	}

err:
	if (_in_key) free(_in_key);
	if (_in_share) free(_in_share);
	return status;
}

static sgx_status_t SGX_CDECL sgx_ecall_delete(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_ecall_delete_t));
	//
	// fence after pointer checks
	//
	sgx_lfence();
	ms_ecall_delete_t* ms = SGX_CAST(ms_ecall_delete_t*, pms);
	ms_ecall_delete_t __in_ms;
	if (memcpy_s(&__in_ms, sizeof(ms_ecall_delete_t), ms, sizeof(ms_ecall_delete_t))) {
		return SGX_ERROR_UNEXPECTED;
	}
	sgx_status_t status = SGX_SUCCESS;
	char* _tmp_key = __in_ms.ms_key;
	size_t _len_key = 100 * sizeof(char);
	char* _in_key = NULL;

	CHECK_UNIQUE_POINTER(_tmp_key, _len_key);

	//
	// fence after pointer checks
	//
	sgx_lfence();

	if (_tmp_key != NULL && _len_key != 0) {
		if ( _len_key % sizeof(*_tmp_key) != 0)
		{
			status = SGX_ERROR_INVALID_PARAMETER;
			goto err;
		}
		_in_key = (char*)malloc(_len_key);
		if (_in_key == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		if (memcpy_s(_in_key, _len_key, _tmp_key, _len_key)) {
			status = SGX_ERROR_UNEXPECTED;
			goto err;
		}

	}
	ecall_delete(_in_key);

err:
	if (_in_key) free(_in_key);
	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv; uint8_t is_switchless;} ecall_table[3];
} g_ecall_table = {
	3,
	{
		{(void*)(uintptr_t)sgx_ecall_put, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_get, 0, 0},
		{(void*)(uintptr_t)sgx_ecall_delete, 0, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[1][3];
} g_dyn_entry_table = {
	1,
	{
		{0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_print_string(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_string_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_string_t);
	void *__tmp = NULL;


	CHECK_ENCLAVE_POINTER(str, _len_str);

	if (ADD_ASSIGN_OVERFLOW(ocalloc_size, (str != NULL) ? _len_str : 0))
		return SGX_ERROR_INVALID_PARAMETER;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_string_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_string_t));
	ocalloc_size -= sizeof(ms_ocall_print_string_t);

	if (str != NULL) {
		if (memcpy_verw_s(&ms->ms_str, sizeof(const char*), &__tmp, sizeof(const char*))) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		if (_len_str % sizeof(*str) != 0) {
			sgx_ocfree();
			return SGX_ERROR_INVALID_PARAMETER;
		}
		if (memcpy_verw_s(__tmp, ocalloc_size, str, _len_str)) {
			sgx_ocfree();
			return SGX_ERROR_UNEXPECTED;
		}
		__tmp = (void *)((size_t)__tmp + _len_str);
		ocalloc_size -= _len_str;
	} else {
		ms->ms_str = NULL;
	}

	status = sgx_ocall(0, ms);

	if (status == SGX_SUCCESS) {
	}
	sgx_ocfree();
	return status;
}

