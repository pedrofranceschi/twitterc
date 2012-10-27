#include "http_connection.h"

char *_string_replace(char *string, const char *substr, const char *replacement){
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
	    return (char *)string;
	}
	memcpy ( newstr, oldstr, tok - oldstr );
	memcpy ( newstr + (tok - oldstr), replacement, strlen ( replacement ) );
	memcpy ( newstr + (tok - oldstr) + strlen( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
	memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );
	/* move back head right after the last replacement */
	head = newstr + (tok - oldstr) + strlen( replacement );
	free (oldstr);
    }
	
    free(string);
	
    return newstr;
}

/* char *_html_escape_string(char *string) { */
/*     char *escaped_string = strdup(string); */
/*     return _string_replace(_string_replace(_string_replace(_string_replace(_string_replace(_string_replace(_string_replace(_string_replace(_string_replace(_string_replace(escaped_string, */
/*        "%", "%25"), "#", "%23"), "/", "%2F"), ":", "%3A"), "=", "%3D"), "&", "%26"), " ", "%20"), "?", "%3F"), "+", "%2B"), "@", "%40"); // TODO: escape more stuff */
/*     return string; */
/* } */

char *_html_escape_string(char *string) {
    const char ignore_escape_characters[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '-', '.', '_', '~'};
    char *escaped_string = calloc(strlen(string) * 3, sizeof(char));
    int i, j;

    for(i = 0; i < strlen(string); i++) {
	char current_char = string[i];
	int replace_current_char = 1;
    	for(j = 0; j < sizeof(ignore_escape_characters)/sizeof(char); j++) {
    	    if(ignore_escape_characters[j] == current_char) {
		char *new_string = malloc(sizeof(char) * 3);
		sprintf(new_string, "%c", current_char);
		strcat(escaped_string, new_string);
		// printf("escaped_string: %s\n", escaped_string);
		free(new_string);
		
    		replace_current_char = 0;
    		break;
    	    }
    	}

	if(replace_current_char == 1) {
	    char *new_string = malloc(sizeof(char) * 3);
	    sprintf(new_string, "%%%02X", current_char);
	    strcat(escaped_string, new_string);
	    free(new_string);
	}
    }

    return escaped_string;
}

size_t _write_data( void *buffer, size_t size, size_t nmemb, void *userp )
{
    HTTPConnection *http_connection = (HTTPConnection *)userp;
    int segsize = size * nmemb;
	
    char *response_buffer = http_connection->response_buffer;
    memcpy(((void *)response_buffer) + http_connection->response_buffer_length, buffer, (size_t)segsize);
	
    // Update the write index
    http_connection->response_buffer_length += segsize;

    // Null terminate the buffer
    response_buffer[http_connection->response_buffer_length] = 0;
	
    // Return the number of bytes received, indicating to curl that all is okay
    return segsize;
}

void HTTPConnection_initialize(HTTPConnection *http_connection) {
    http_connection->first_parameter = NULL;
    http_connection->url = NULL;
    http_connection->response_buffer_length = 0;
    http_connection->response_buffer = malloc(sizeof(char) * MAX_BUFFER);
    http_connection->connection_method = HTTPConnectionMethodGET;
}

void HTTPConnection_free(HTTPConnection *http_connection) {
    free(http_connection->response_buffer);
    free(http_connection->url);
    if(http_connection->first_parameter) {
	HTTPParameter_free(http_connection->first_parameter, 1);
    }
    http_connection = NULL;
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
    http_parameter = NULL;
}

void HTTPParameter_free(HTTPParameter *http_parameter, int free_related_params) {
    if(free_related_params != 0) {
	HTTPParameter *current_parameter = http_parameter;
	while(current_parameter != NULL) {
	    HTTPParameter *new_current_parameter = (HTTPParameter *)current_parameter->next_parameter;
	    _HTTPParameter_free(current_parameter);
	    current_parameter = new_current_parameter;
	}
    } else {
	_HTTPParameter_free(http_parameter);
    }
}

int HTTPConnection_perform_request(HTTPConnection *http_connection) {
    CURL *curl;
    CURLcode ret;
    http_connection->response_buffer_length = 0;
	
    curl = curl_easy_init();
    if (!curl) {
	printf("Unable to initialize cURL.\n");
	return -1;
    }
	
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)http_connection); // sets userp
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_data); // sets write function
	
    if(http_connection->connection_method == HTTPConnectionMethodPOST) {
	curl_easy_setopt(curl, CURLOPT_POST, 1);
    }
	
    struct curl_slist *headers = NULL; /* init to NULL is important */
	
    char *parameters_string = malloc(sizeof(char) * 1000);
	
    if(http_connection->first_parameter != NULL) {		
	HTTPParameter *current_parameter = http_connection->first_parameter;
	while(current_parameter != NULL) {
	    char *current_parameter_string = malloc(sizeof(char) * 1000);
	    if(current_parameter->type == HTTPParameterTypeParameter) {
		sprintf(current_parameter_string, "%s=%s&", current_parameter->key, current_parameter->value);
		strcat(parameters_string, current_parameter_string);
	    } else if(current_parameter->type == HTTPParameterTypeHeader) {
		sprintf(current_parameter_string, "%s: %s", current_parameter->key, current_parameter->value);
		headers = curl_slist_append(headers, current_parameter_string);
	    }
			
	    free(current_parameter_string);
	    current_parameter = (HTTPParameter *)current_parameter->next_parameter;
	}
		
	parameters_string[strlen(parameters_string) - 1] = '\0';
		
	if(http_connection->connection_method == HTTPConnectionMethodPOST) {
	    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters_string);
	} else {
	    strcat(http_connection->url, "?");
	    strcat(http_connection->url, parameters_string);
	}
		
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
	
    curl_easy_setopt(curl, CURLOPT_URL, http_connection->url); // sets url
	
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	
    ret = curl_easy_perform(curl);
	
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
	
    free(parameters_string);
	
    if (ret != 0) {
	printf("cURL error (%i): %s\n", ret, curl_easy_strerror(ret));
	return ret;
    }
	
    return 0;
}
