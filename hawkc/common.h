#ifndef HAWKC_COMMON_H
#define HAWKC_COMMON_H 1

#include <ctype.h>
#include <math.h>
#include "config.h"
#include "hawkc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Callback function types for the Auth-header parser.
 */
typedef HawkcError (*HawkcSchemeHandler) (HawkcContext ctx, HawkcString scheme, void*data);
typedef HawkcError (*HawkcParamHandler) (HawkcContext ctx, HawkcString key, HawkcString value, void*data);

/** Structure for the Algorithm typedef in hawkc.h
 */
struct HawkcAlgorithm {
	const char* name;
};

/*
 * Size of the static base string buffer used to generate HMACs.
 * This value should be large enough to hold Hawk base strings
 * containing URLs of common sizes.
 *
 * For occasional larger sizes hawkc will dynamically allocate
 * and free a larger buffer. See MAX_DYN_BASE_BUFFER_SIZE.
 */
#define BASE_BUFFER_SIZE 512

/*
 * In the case of base strings that exceed the static BASE_BUFFER_SIZE hawkc
 * will dynamically allocate a buffer to hold the base string. In order to
 * prevent very long URLs (possibly attacks) from leading to very large allocations
 * MAX_DYN_BASE_BUFFER_SIZE sets the upper bounds of dynamic allocation.
 */
#define MAX_DYN_BASE_BUFFER_SIZE 2048

/*
 * Size for timestamp base string buffers. Buffer must be
 * large enough to hold the following string:
 * hawk.1.ts\n1375085388\n (21 bytes).
 *
 * We add a little space and make it 30.
 */
#define TS_BASE_BUFFER_SIZE 30

/**
 * Set the context error for error retrieval by the caller.
 */
HawkcError HAWKCAPI hawkc_set_error(HawkcContext ctx, HawkcError e, const char *fmt, ...);

/* Calculate the length of the Hawk header base string used for HMAC generation
 *
 * Useful to check or determine buffer sizes.
 */
size_t hawkc_calculate_base_string_length(HawkcContext ctx, AuthorizationHeader header);

/** Create Hawk header base string to be used for HMAC generation.
 *
 */
void hawkc_create_base_string(HawkcContext ctx, AuthorizationHeader header, unsigned char* base_buf, size_t *base_len);

/** Create Hawk header timestamp base string to be used for HMAC generation in WWW-Authenticate response
 * headers.
 *
 */
void hawkc_create_ts_base_string(HawkcContext ctx, WwwAuthenticateHeader header, unsigned char* buf, size_t *len);


/** Parse an Authorization or WWW-Authenticate header.
 *
 * This will parse headers conforming to http://tools.ietf.org/html/draft-ietf-httpbis-p7-auth#section-4
 * except for the use of token68 tokens. In other words, you won't be able to parse
 * HTTP Basic Auth Authorization headers with this.
 *
 * Function parses a string 'value' of 'len' bytes and will call the scheme handler for the scheme
 * token and the param_handler for each parameter/value pair encountered.
 *
 * Parsed parts will not be copied, the provided HawkString variables simply point to the portions
 * of the supplied string 'value'.
 *
 * Caveat: This means that extracted quoted strings will contain the escape characters. It is
 * the responsibility of the caller to make a copy of the quoted string and remove the \.
 */
HawkcError hawkc_parse_auth_header(HawkcContext ctx, unsigned char *value, size_t len, HawkcSchemeHandler scheme_handler, HawkcParamHandler param_handler, void *data);

/** Fixed time byte-wise comparision.
 *
 * Return 1 if the supplied byte sequences are byte-wise equal, 0 otherwise.
 */
int hawkc_fixed_time_equal(unsigned char *lhs, unsigned char * rhs, size_t len);

/** Turn an unsigned char array into an array of hex-encoded bytes.
 *
 * The result will encode each bye as a two-chars hex value (00 to ff)
 * and thus be twice as long as the input.
 *
 * The caller is responsible to provide a buffer of at least 2xlen
 * bytes to hold the result.
 *
 * Does not \0 terminate the created string.
 */
void hawkc_bytes_to_hex(const unsigned char *bytes, size_t len, unsigned char *buf);


/*
 * Determine the number of digits in a time_t value.
 */
unsigned int hawkc_number_of_digits(time_t t);


/*
 * Parse a unix time value from a string. If the string is not parsable, this function returns HAWKC_TIME_PARSE_ERROR.
 */
HawkcError parse_time(HawkcContext ctx, HawkcString ts, time_t *tp);


/*
 * On some target environments I had problems compiling since digittoint wasn't
 * available. Here I provide my own implementation of digittoint.
 */
int my_digittoint(char ch);




#ifdef __cplusplus
} // extern "C"
#endif


#endif /* !defined HAWKC_COMMON_H */
 
