#include "http_connection.h"

char *_string_replace( const char *string, const char *substr, const char *replacement ){
	char *tok = NULL;
	char *newstr = NULL;
	char *oldstr = NULL;
	char *head = NULL;

/* if either substr or replacement is NULL, duplicate string a let caller handle it */
	if ( substr == NULL || replacement == NULL ) return strdup (string);
	newstr = strdup (string);
	head = newstr;
	while ( (tok = strstr ( head, substr ))){
		oldstr = newstr;
		newstr = malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );
	/*failed to alloc mem, free old string and return NULL */
		if ( newstr == NULL ){
			free (oldstr);
			return NULL;
		}
		memcpy ( newstr, oldstr, tok - oldstr );
		memcpy ( newstr + (tok - oldstr), replacement, strlen ( replacement ) );
		memcpy ( newstr + (tok - oldstr) + strlen( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
		memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );
	/* move back head right after the last replacement */
		head = newstr + (tok - oldstr) + strlen( replacement );
		free (oldstr);
	}
	return newstr;
}

char *_html_escape_string(char *string) {
	return _string_replace(_string_replace(_string_replace(_string_replace(_string_replace(_string_replace(_string_replace(_string_replace(string,
           "%", "%25"), "#", "%23"), "/", "%2F"), ":", "%3A"), "=", "%3D"), "&", "%26"), " ", "%20"), "?", "%3F"); // TODO: escape more stuff
}

size_t _write_data( void *buffer, size_t size, size_t nmemb, void *userp )
{
	HTTPConnection *httpConnection = (HTTPConnection *)userp;
	int segsize = size * nmemb;
	
	char *response_buffer = httpConnection->response_buffer;
	memcpy(((void *)response_buffer) + httpConnection->response_buffer_length, buffer, (size_t)segsize);
	
	// Update the write index
	httpConnection->response_buffer_length += segsize;

	// Null terminate the buffer
	response_buffer[httpConnection->response_buffer_length] = 0;
	
	// Return the number of bytes received, indicating to curl that all is okay
	return segsize;
}

void HTTPConnection_initialize(HTTPConnection *httpConnection) {
	httpConnection->first_parameter = NULL;
	httpConnection->url = NULL;
	httpConnection->response_buffer_length = 0;
	httpConnection->response_buffer = malloc(sizeof(char) * MAX_BUFFER);
	httpConnection->connection_method = HTTPConnectionMethodGET;
}

void HTTPConnection_free(HTTPConnection *httpConnection) {
	free(httpConnection->response_buffer);
	HTTPParameter_free(httpConnection->first_parameter, 1);
}

void HTTPParameter_initialize(HTTPParameter *http_parameter) {
	http_parameter->type = HTTPParameterTypeParameter;
	http_parameter->key = NULL;
	http_parameter->value = NULL;
	http_parameter->next_parameter = NULL;
	http_parameter->previous_parameter = NULL;
}

void _HTTPParameter_free(HTTPParameter *http_parameter) {
	free(http_parameter->key);
	free(http_parameter->value);
	free(http_parameter);
}

void HTTPParameter_free(HTTPParameter *http_parameter, int free_related_params) {
	if(free_related_params != 0) {
		HTTPParameter *current_parameter = http_parameter;
		while(current_parameter != NULL) {
			HTTPParameter *new_current_parameter = current_parameter->next_parameter;
			_HTTPParameter_free(current_parameter);
			current_parameter = new_current_parameter;
		}
	} else {
		_HTTPParameter_free(http_parameter);
	}
}

int HTTPConnection_perform_request(HTTPConnection *httpConnection) {
	CURL *curl;
	CURLcode ret;
	httpConnection->response_buffer_length = 0;
	
	curl = curl_easy_init();
	if (!curl) {
		printf("Unable to initialize cURL.\n");
		return 1;
	}
	
	curl_easy_setopt(curl, CURLOPT_URL, httpConnection->url); // sets url
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)httpConnection); // sets userp
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_data); // sets write function
	
	if(httpConnection->connection_method == HTTPConnectionMethodPOST) {
		curl_easy_setopt(curl, CURLOPT_POST, 1);
	}
	
	struct curl_slist *headers = NULL; /* init to NULL is important */
	
	if(httpConnection->first_parameter != NULL) {
		char *parameters_string = malloc(sizeof(char) * 500);
		
		HTTPParameter *current_parameter = httpConnection->first_parameter;
		while(current_parameter != NULL) {
			char *current_parameter_string = malloc(sizeof(char) * 200);
			if(current_parameter->type == HTTPParameterTypeParameter) {
				sprintf(current_parameter_string, "%s=%s&", current_parameter->key, current_parameter->value);
				printf("current_parameter_string p: %s\n", current_parameter_string);
				strcat(parameters_string, current_parameter_string);
			} else if(current_parameter->type == HTTPParameterTypeHeader) {
				sprintf(current_parameter_string, "%s: %s", current_parameter->key, current_parameter->value);
				printf("current_parameter_string h: %s\n", current_parameter_string);
				headers = curl_slist_append(headers, current_parameter_string);
			}
			
			free(current_parameter_string);
			current_parameter = current_parameter->next_parameter;
		}
		
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters_string);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	}
	
	ret = curl_easy_perform(curl);
	
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	
	if (ret != 0) {
		printf("cURL error! (%d) \n", ret);
		return 2;
	}
	
	return 0;
}