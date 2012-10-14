#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <string.h>

#define MAX_BUFFER 65536

typedef enum {
	HTTPParameterTypeParameter = 1,
	HTTPParameterTypeHeader = 2,
	HTTPParameterTypeAuthorizationHeader = 3,
	HTTPParameterTypeIgnore = 4
} HTTPParameterType;

typedef struct {
	char *key;
	char *value;
	struct HTTPParameter *next_parameter, *previous_parameter;
	HTTPParameterType type;
} HTTPParameter;

typedef enum {
	HTTPConnectionMethodGET,
	HTTPConnectionMethodPOST,
} HTTPConnectionMethod;

typedef struct {
	char *response_buffer;
	int response_buffer_length;
	char *url;
	HTTPParameter *first_parameter;
	HTTPConnectionMethod connection_method;
} HTTPConnection;

void HTTPConnection_initialize(HTTPConnection *http_connection);
void HTTPConnection_free(HTTPConnection *http_connection);
void HTTPParameter_initialize(HTTPParameter *http_parameter);
void HTTPParameter_free(HTTPParameter *http_parameter, int free_related_params);

int HTTPConnection_perform_request(HTTPConnection *http_connection);

char *_html_escape_string(char *string);