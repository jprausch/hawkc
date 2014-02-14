/*
 * Header file for hawkc.
 *
 * This header file is all you need to include to use hawkc
 * functionality from outside the hawkc library.
 *
 */
#ifndef HAWKC_H
#define HAWKC_H 1

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAWKCAPI
#define HAWKCAPI
#endif

/** The following macros define buffer size constants.
 *
 * These constants are defined for convenience, to allow
 * the callers of functions that require caller-buffer-allocation
 * to use fixed buffers as opposed to dealing with dynamic
 * allocation and deallocation of memory.
 *
 * The buffer sizes are considerably small to justify the
 * space overhead.
 *
 * These buffer sizes depend on the algorithms and options defined
 * in common.c and must be adjusted if new algorithms are added that
 * require increased buffer sizes.
 *
 * They need to be defined in this header because the structs (which
 * we deliberately expose to the user) make use of the macros.
 */

/*
 * Must match the specifications of the supplied HMAC algorithms.
 * That is, it must be the longest key length generated by all
 * of the algorithms.
 */
#define MAX_HMAC_BYTES 32

/** Maximum size necessary for storing HMACs in base64 encoded form.
 *
 * Depends on MAX_HMAC_BYTES and amounts to MAX_HMAC_BYTES * 4/3 + 2 bytes max. padding
 *
 */
#define MAX_HMAC_BYTES_B64 45

/*
 * Number of bytes to generate for nonce values.
 */
#define MAX_NONCE_BYTES 6

/*
 * Buffer size necessary to store nonce values in hex-coded form.
 * This is  2 * MAX_NONCE_BYTES  (2 hex digits per nonce byte)
 */
#define MAX_NONCE_HEX_BYTES 12

/**
 * Hawkc mostly uses strings that are not null terminated but are associated with a length
 * information. HawkcString encapsulates a character array combined with a length.
 */
typedef struct HawkcString {
	size_t len;
	unsigned char *data;
} HawkcString;

/** hawkc error codes.
 *
 *
 */
typedef enum {
	HAWKC_OK, /* no error */
	HAWKC_PARSE_ERROR, /* Parse error */
	HAWKC_BAD_SCHEME_ERROR, /* Authentication scheme name not Hawk */
	HAWKC_TOKEN_VALIDATION_ERROR, /* Token cannot be validated */
	HAWKC_ERROR_UNKNOWN_ALGORITHM, /* Unknown algorithm */
	HAWKC_CRYPTO_ERROR, /* Some unrecognized error in the crypto library occurred */
	HAWKC_TIME_VALUE_ERROR, /* Not a valid unix time value */
	HAWKC_NO_MEM, /* Unable to allocate memory */
	HAWKC_REQUIRED_BUFFER_TOO_LARGE, /* Required buffer size is too large */
	HAWKC_ERROR, /* unspecific error */
	HAWKC_BASE64_ERROR, /* Unexpected string length or padding in base64 en- or decoding */
    HAWKC_OVERFLOW_ERROR /* Unexpected number value would cause integer overflow */
	/* If you add errors here, add them in common.c also */
} HawkcError;

/*
 * Global handle to pass to all functions.
 * Struct defined below to allow use of the
 * library with automatic variable and no
 * need to allocate memory for context.
 */
typedef struct HawkcContext *HawkcContext;

/*
 * Type for HMAC algorithms supplied by hawkc library.
 */
typedef struct HawkcAlgorithm *HawkcAlgorithm;

/*
 * Memory allocation function pointers. Hawkc allows setting custom
 * allocation functions. For example, if you need some that do
 * memory pooling.
 */
typedef void* (*HawkcMallocFunc)(HawkcContext ctx, size_t size);
typedef void* (*HawkcCallocFunc)(HawkcContext ctx, size_t count, size_t size);
typedef void (*HawkcFreeFunc)(HawkcContext ctx, void *ptr);

/*
 * Type for holding Hawkc Authorization and Server-Authorization
 * header data. This struct is used for storing parsed data as
 * well as for constructing header data before creating a string representation.
 */
typedef struct AuthorizationHeader {
	HawkcString id;
	HawkcString mac;
	HawkcString hash;
	HawkcString nonce;
	time_t ts;
	HawkcString app;
	HawkcString dlg;
	HawkcString ext;

} *AuthorizationHeader;

/*
 * Type for holding Hawkc WWW-Authenticate
 * header data. This struct is used for storing parsed data as
 * well as for constructing header data before creating a string representation.
 */
typedef struct WwwAuthenticateHeader {
	time_t ts;
	HawkcString tsm;
} *WwwAuthenticateHeader;

/* The global Hawkc context.
 *
 * header_in is intended for received Authorization or Server-Authorization header
 * depending on whether the library is used on the server- or client side.
 *
 * header_out is intended for Authorization or Server-Authorization header to send,
 * depending on whether the library is used on the server- or client side.
 *
 * www_authenticate_header is used either way, depending on use on
 * the server- or client side.
 *
 * hmac_buffer, ts_hmac_buffer and nonce_buffer are used as buffers to write HMAC
 * signatures and nonce to. There are three corresponding HawkcStrings to point
 * to the buffers.
 *
 *
 */
struct HawkcContext {
	HawkcMallocFunc malloc;
	HawkcCallocFunc calloc;
	HawkcFreeFunc free;
	HawkcError error;
	char error_string[1024];
	int offset;

	HawkcAlgorithm algorithm;
	HawkcString password;

	HawkcString method;
	HawkcString path;
	HawkcString host;
	HawkcString port;

	struct AuthorizationHeader header_in;
	struct AuthorizationHeader header_out;
	struct WwwAuthenticateHeader www_authenticate_header;

	unsigned char hmac_buffer[MAX_HMAC_BYTES_B64];
	unsigned char ts_hmac_buffer[MAX_HMAC_BYTES_B64];
	unsigned char nonce_buffer[MAX_NONCE_HEX_BYTES];
	HawkcString hmac;
	HawkcString ts_hmac;
	HawkcString nonce;
};

/*
 * Initialize a hawkc context.
 *
 * Normal usage would be like this:
 *
 * struct HawkcContext ctx;
 * hawkc_context_init(&ctx);
 * ...
 *
 */
void HAWKCAPI hawkc_context_init(HawkcContext ctx);

/*
 * Set the clock offset to use if context is used in a client implementation.
 */
void HAWKCAPI hawkc_context_set_clock_offset(HawkcContext ctx,int offset);

/*
 * Set the malloc function to use internally. Defaults to standard malloc.
 */
void* HAWKCAPI hawkc_malloc(HawkcContext ctx, size_t size);

/*
 * Set the calloc function to use internally. Defaults to standard calloc.
 */
void* HAWKCAPI hawkc_calloc(HawkcContext ctx, size_t count, size_t size);

/*
 * Set the free function to use internally. Defaults to standard free.
 */
void HAWKCAPI hawkc_free(HawkcContext ctx, void *ptr);

/*
 * Set the password to be used for signing and signature validation.
 */
void HAWKCAPI hawkc_context_set_password(HawkcContext ctx,unsigned char *password, size_t len);

/*
 * Set the algorithm to be used for signing and signature validation.
 */
void HAWKCAPI hawkc_context_set_algorithm(HawkcContext ctx,HawkcAlgorithm algorithm);

/*
 * Set the request method that will be part of the signature base string.
 */
void HAWKCAPI hawkc_context_set_method(HawkcContext ctx,unsigned char *method, size_t len);

/*
 * Set the request path that will be part of the signature base string.
 */
void HAWKCAPI hawkc_context_set_path(HawkcContext ctx,unsigned char *path, size_t len);

/*
 * Set the request host that will be part of the signature base string.
 */
void HAWKCAPI hawkc_context_set_host(HawkcContext ctx,unsigned char *host, size_t len);

/*
 * Set the request port that will be part of the signature base string.
 */
void HAWKCAPI hawkc_context_set_port(HawkcContext ctx,unsigned char *port, size_t len);

/*
 * Set the id-parameter to be placed in outgoing headers or which has be parsed
 * from an incoming header.
 */
void HAWKCAPI hawkc_context_set_id(HawkcContext ctx,unsigned char *id, size_t len);

/*
 * Set the ext-parameter to be placed in outgoing headers or which has be parsed
 * from an incoming header.
 *
 * FIXME: We need to doc here, whether double qoutes must be escaped or unescaped.
 * See https://github.com/algermissen/hawkc/issues/2
 */
void HAWKCAPI hawkc_context_set_ext(HawkcContext ctx,unsigned char *ext, size_t len);

/*
 * Parse incoming authorization header. On the client side this will be the used
 * for the Authorization header, on the server side, this will be used for the
 * Hawk-defined Server-Authorization header.
 *
 * Header and number of bytes to parse are to be supplied as the value and len
 * parameters.
 */
HawkcError HAWKCAPI hawkc_parse_authorization_header(HawkcContext ctx, unsigned char *value, size_t len);

/*
 * Caculate the buffer size necessary to store an authorization header value generated
 * from the current state of the context.
 *
 * Call this function before calling hawkc_create_authorization_header().
 */
HawkcError HAWKCAPI hawkc_calculate_authorization_header_length(HawkcContext ctx, size_t *required_len);

/*
 * Create an authorization header value from the current state of the context. The generated header
 * will be written to the supplied buffer. The number of bytes actually written is put into the
 * len parameter.
 */
HawkcError HAWKCAPI hawkc_create_authorization_header(HawkcContext ctx, unsigned char* buf, size_t *len);


/*
 * Use the context to validate the HMAC of the authorization header that has been
 * parsed before using hawkc_parse_authorization_header(). This is intended
 * for use on client (Authorization hedare) and server side (Serer-Authorization
 * header).
 */
HawkcError HAWKCAPI hawkc_validate_hmac(HawkcContext ctx, int *is_valid);

/*
 * Set the timestamp to be used in WWW-Authenticate header.
 */
void HAWKCAPI hawkc_www_authenticate_header_set_ts(HawkcContext ctx, time_t ts);

/*
 * Parse incoming WWW-Authorization header.
 * This is for client-side only because WWW-Authenticate headers are only sent by servers.
 *
 * Header and number of bytes to parse are to be supplied as the value and len
 * parameters.
 *
 */
HawkcError HAWKCAPI hawkc_parse_www_authenticate_header(HawkcContext ctx, unsigned char *value, size_t len);

/*
 * Caculate the buffer size necessary to store a WWW-Authenticate header value generated
 * from the current state of the context.
 *
 * Call this function before calling hawkc_create_www_authenticate_header().
 */
HawkcError HAWKCAPI hawkc_calculate_www_authenticate_header_length(HawkcContext ctx, size_t *required_len);

/*
 * Create a WWW-Authenticate header value from the current state of the context. The generated header
 * will be written to the supplied buffer. The number of bytes actually written is put into the
 * len parameter.
 */
HawkcError HAWKCAPI hawkc_create_www_authenticate_header(HawkcContext ctx, unsigned char* buf, size_t *len);


/** Obtain HMAC algorithm for specified name.
 *
 * Returns the algorithm or NULL if not found.
 */
HawkcAlgorithm HAWKCAPI hawkc_algorithm_by_name(char *name, size_t len);

/** Obtain human readable string for the provided error code.
 *
 */
char* HAWKCAPI hawkc_strerror(HawkcError e);

/** Get a human readable message about the last error
 * condition that ocurred for the given context.
 *
 */
char * HAWKCAPI hawkc_get_error(HawkcContext ctx);

/** Get the hawkc error code that occurred last in the
 * given context.
 *
 */
HawkcError HAWKCAPI hawkc_get_error_code(HawkcContext ctx);

/*
 * Specialized version of itoa for auth header creation.
 * Writes the unix timestamp value to buf and returns the number of digits written.
 *
 * Handles negative time_t values to be able to represent time differences.
 *
 * Caller is required to allocate enaough space to hold the number in string
 * form. Beware that a negative value needs one byte more for the sign.
 *
 * FIXME: Add parameter to pass in the buffer size to perform internal check
 * agains writing past buffer end.
 * See https://github.com/algermissen/hawkc/issues/14
 */
size_t HAWKCAPI hawkc_ttoa(unsigned char* buf, time_t value);


/** The algorithms and options defined by hawkc.
 *
 * Please refer to common.c for their definition.
 */
extern HawkcAlgorithm HAWKC_SHA_256;
extern HawkcAlgorithm HAWKC_SHA_1;



#ifdef __cplusplus
} // extern "C"
#endif

#endif /* !defined HAWKC_H */

