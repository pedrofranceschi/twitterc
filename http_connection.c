#include "http_connection.h"

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
	httpConnection->url = NULL;
	httpConnection->response_buffer_length = 0;
	httpConnection->response_buffer = malloc(sizeof(char) * MAX_BUFFER);
}

void HTTPConnection_free(HTTPConnection *httpConnection) {
	free(httpConnection->response_buffer);
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
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)httpConnection ); // sets userp
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_data ); // sets write function

	ret = curl_easy_perform(curl);

	curl_easy_cleanup(curl);

	if (ret != 0) {
		printf("cURL error! (%d) \n", ret);
		return 2;
	}
	
	return 0;
}