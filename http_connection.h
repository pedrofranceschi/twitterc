#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <string.h>

#define MAX_BUFFER 65536

typedef struct {
	char *response_buffer;
	int response_buffer_length;
	char *url;
} HTTPConnection;

int HTTPConnection_perform_request(HTTPConnection *httpConnection);
void HTTPConnection_initialize(HTTPConnection *httpConnection);
void HTTPConnection_free(HTTPConnection *httpConnection);