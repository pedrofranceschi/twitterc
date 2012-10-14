#include "twitter_api_oauth.h"

void _generate_random_string(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

	srand(time(NULL));
	int i;
    for (i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

HTTPParameter *_create_parameter(char *key, char *value) {
	HTTPParameter *parameter = malloc(sizeof(*parameter));
	HTTPParameter_initialize(parameter);
	parameter->key = key;
	parameter->value = value;
	return parameter;
}

int compare_parameters(const void *a, const void *b) 
{
	HTTPParameter *parameter_a = (HTTPParameter *)a;
	HTTPParameter *parameter_b = (HTTPParameter *)b;
		
	return strcmp(parameter_a->key, parameter_b->key);;
}

void OAuthOAuthToken_initialize(OAuthOAuthToken *oauth_token) {
	oauth_token->oauth_token = NULL;
	oauth_token->oauth_token_secret = NULL;
	oauth_token->oauth_verifier = NULL;
}

void OAuthOAuthToken_free(OAuthOAuthToken *oauth_token) {
	free(oauth_token->oauth_token);
	free(oauth_token->oauth_token_secret);
	free(oauth_token->oauth_verifier);
}

void OAuthAccessToken_initialize(OAuthAccessToken *access_token) {
	access_token->access_token = NULL;
	access_token->access_token_secret = NULL;
}

void OAuthAccessToken_free(OAuthAccessToken *access_token) {
	free(access_token->access_token);
	free(access_token->access_token_secret);
}

char *_generate_signature_for_parameters(HTTPParameter *first_parameter, HTTPConnection *http_connection) {
	int parameters_count = 0;
	HTTPParameter *current_parameter = first_parameter;
	while(current_parameter != NULL) {
		parameters_count++;
		current_parameter = current_parameter->next_parameter;
	}
	
	HTTPParameter http_parameters[parameters_count];
	int i = 0;
	
	current_parameter = first_parameter;
	while(current_parameter != NULL) {
		http_parameters[i] = *current_parameter;
		i++;
		current_parameter = current_parameter->next_parameter;
	}
	
	// sort parameters alphabetically
	qsort(http_parameters, parameters_count, sizeof(HTTPParameter), compare_parameters);
	
	char *signature_base_string = malloc(sizeof(char) * 500);
	
	for(i = 0; i < parameters_count; i++) {
		current_parameter = &(http_parameters[i]);
		if(current_parameter->type != HTTPParameterTypeAuthorizationHeader) {
			sprintf(signature_base_string, "%s%s=%s&", signature_base_string, current_parameter->key, current_parameter->value);
		}
	}
	
	signature_base_string[strlen(signature_base_string) - 1] = '\0'; // removes last '&'

	sprintf(signature_base_string, "%s&%s&%s", (http_connection->connection_method == HTTPConnectionMethodPOST ? "POST" : "GET"), _html_escape_string(http_connection->url), _html_escape_string(signature_base_string));
	
	unsigned char sha1_result[30];// = malloc(sizeof(char) * 32);
	char *key_string = malloc(sizeof(char) * 500);
	if(current_oauth_token) {
		sprintf(key_string, "%s&%s", OAUTH_CONSUMER_SECRET, current_oauth_token->oauth_token_secret);
	} else if(current_access_token) {
		sprintf(key_string, "%s&%s", OAUTH_CONSUMER_SECRET, current_access_token->access_token_secret);
	} else {
		sprintf(key_string, "%s&", OAUTH_CONSUMER_SECRET);
	}
	
	unsigned int md_len = 32;
	
	// result = HMAC(EVP_sha256(), key, 4, data, 28, NULL, NULL);
	int success = hmac_sha1(key_string, strlen(key_string), signature_base_string, strlen(signature_base_string), sha1_result);
	// int success = HMAC(EVP_sha1(), key_string, strlen(key_string), signature_base_string, strlen(signature_base_string), sha1_result, &md_len);
	
	// if(success != 0) {
	// 	printf(" Error generating oauth_signature (HMAC-SHA1). \n");
	// 	return NULL;
	// }
	
	free(signature_base_string);
	
	char *final_signature = malloc(sizeof(char) * 500);
	b64_encode(&sha1_result, final_signature);
	free(key_string);
	
	return final_signature;
}

void TwitterAPI_oauth_authenticate_connection(HTTPConnection *http_connection) {
	HTTPParameter *oauth_consumer_key_parameter = _create_parameter(strdup("oauth_consumer_key"), strdup(OAUTH_CONSUMER_KEY));
	
	char *random_oauth_nonce = malloc(sizeof(char) * 32);
	_generate_random_string(random_oauth_nonce, 32);
	HTTPParameter *oauth_nonce_parameter = _create_parameter(strdup("oauth_nonce"), strdup(random_oauth_nonce));
	oauth_consumer_key_parameter->next_parameter = oauth_nonce_parameter;
	free(random_oauth_nonce);
	
	HTTPParameter *oauth_signature_method_parameter = _create_parameter(strdup("oauth_signature_method"), strdup("HMAC-SHA1"));
	oauth_nonce_parameter->previous_parameter = oauth_consumer_key_parameter;
	oauth_nonce_parameter->next_parameter = oauth_signature_method_parameter;
	
	char *oauth_timestamp = malloc(sizeof(char) * 30);
	sprintf(oauth_timestamp, "%i", (int)time(0));
	HTTPParameter *oauth_timestamp_parameter = _create_parameter(strdup("oauth_timestamp"), oauth_timestamp);
	oauth_signature_method_parameter->previous_parameter = oauth_nonce_parameter;
	oauth_signature_method_parameter->next_parameter = oauth_timestamp_parameter;
	
	HTTPParameter *oauth_version_parameter = _create_parameter(strdup("oauth_version"), strdup("1.0"));
	oauth_timestamp_parameter->previous_parameter = oauth_signature_method_parameter;
	oauth_timestamp_parameter->next_parameter = oauth_version_parameter;
	oauth_version_parameter->previous_parameter = oauth_signature_method_parameter;
	
	// OAUTH TOKEN IF OAuthAuthentication HAS ONE
	
	HTTPParameter *current_parameter = http_connection->first_parameter;
	HTTPParameter *last_configured_parameter = oauth_version_parameter;
	
	if(current_access_token != NULL) {
		HTTPParameter *oauth_token_parameter = _create_parameter(strdup("oauth_token"), _html_escape_string(current_access_token->access_token));
		oauth_token_parameter->previous_parameter = oauth_version_parameter;
		oauth_version_parameter->next_parameter = oauth_token_parameter;
		last_configured_parameter = oauth_token_parameter;
	}
	
	while(current_parameter != NULL) {
		if(current_parameter->type == HTTPParameterTypeAuthorizationHeader) { // needs to be on authorization header
			last_configured_parameter->next_parameter = malloc(sizeof(*current_parameter));
			// memcpy(last_configured_parameter->next_parameter, current_parameter, sizeof(*current_parameter));
			// duplicating...
			((HTTPParameter *)last_configured_parameter->next_parameter)->key = strdup(current_parameter->key);
			((HTTPParameter *)last_configured_parameter->next_parameter)->value = strdup(current_parameter->value);
			((HTTPParameter *)last_configured_parameter->next_parameter)->previous_parameter = last_configured_parameter;
			((HTTPParameter *)last_configured_parameter->next_parameter)->type = HTTPParameterTypeIgnore;
			last_configured_parameter = last_configured_parameter->next_parameter;
		}
		current_parameter = current_parameter->next_parameter;
	}
	
	last_configured_parameter->next_parameter = http_connection->first_parameter;
	if(http_connection->first_parameter) {
		http_connection->first_parameter->previous_parameter = last_configured_parameter;
	}
	
	char *signature_string = _generate_signature_for_parameters(oauth_consumer_key_parameter, http_connection);
	
	printf(" signature_string: %s\n", signature_string);

	// undoing header scheme created to generate signature
	last_configured_parameter->next_parameter = NULL;
	if(http_connection->first_parameter) {
		http_connection->first_parameter->previous_parameter = NULL;
	}
	
	HTTPParameter *oauth_signature_parameter = _create_parameter(strdup("oauth_signature"), _html_escape_string(strdup(signature_string)));
	oauth_signature_parameter->type = HTTPParameterTypeAuthorizationHeader;
	
	last_configured_parameter->next_parameter = oauth_signature_parameter;
	oauth_signature_parameter->previous_parameter = last_configured_parameter;
	
	char *authorization_string = malloc(sizeof(char) * 500);
	strcat(authorization_string, "OAuth ");
	
	current_parameter = oauth_consumer_key_parameter;
	while(current_parameter != NULL) {
		sprintf(authorization_string, "%s%s=\"%s\", ", authorization_string, current_parameter->key, current_parameter->value);
		current_parameter = current_parameter->next_parameter;
	}
	
	authorization_string[strlen(authorization_string) - 2] = '\0'; // removes last ','
	
	printf("authorization_string: %s\n", authorization_string);
	
	HTTPParameter *authorization_parameter = malloc(sizeof(*authorization_parameter));
	HTTPParameter_initialize(authorization_parameter);
	authorization_parameter->type = HTTPParameterTypeHeader;
	authorization_parameter->key = strdup("Authorization");
	authorization_parameter->value = authorization_string;
	
	HTTPParameter_free(oauth_consumer_key_parameter, 1);
	
	HTTPParameter *last_parameter = http_connection->first_parameter;
	
	while(last_parameter != NULL) {
		if(last_parameter->next_parameter == NULL) {
			break;
		}
		last_parameter = last_parameter->next_parameter;
	}
	
	if(http_connection->first_parameter) {
		last_parameter->next_parameter = authorization_parameter;
	} else {
		http_connection->first_parameter = authorization_parameter;
	}
}

int TwitterAPI_oauth_request_token_url(char *request_token_url) {
	HTTPConnection http_connection;
	HTTPConnection_initialize(&http_connection);
	
	http_connection.url = strdup("http://api.twitter.com/oauth/request_token");
	http_connection.connection_method = HTTPConnectionMethodPOST;
	
	http_connection.first_parameter = malloc(sizeof(*http_connection.first_parameter));
	HTTPParameter_initialize(http_connection.first_parameter);
	http_connection.first_parameter->key = strdup("oauth_callback");
	http_connection.first_parameter->value = strdup("oob");
	http_connection.first_parameter->type = HTTPParameterTypeAuthorizationHeader;
	// printf("http_connection.first_parameter: %i\n", http_connection.first_parameter);
	
	// TwitterAPI_oauth_
	TwitterAPI_oauth_authenticate_connection(&http_connection);
	
	int status = HTTPConnection_perform_request(&http_connection);
	if(status != 0) return status;
	
	printf(" body: %s\n", http_connection.response_buffer);
	
	current_oauth_token = malloc(sizeof(*current_oauth_token));
	OAuthOAuthToken_initialize(current_oauth_token);
	
	int i = 0;
	char *substring = strtok(http_connection.response_buffer, "=&");
	
	while(substring != NULL)
	{
		if(i == 1) { // oauth_token
			current_oauth_token->oauth_token = strdup(substring);
		} else if(i == 3) { // oauth_token_secret
			current_oauth_token->oauth_token_secret = strdup(substring);
		}
		
		i += 1;
		substring = strtok(NULL, "=&");
	}
	
	if(i < 5) {
		return -1;
	}
	
	printf("oauth_token: %s\n", current_oauth_token->oauth_token);
	printf("oauth_token_secret: %s\n", current_oauth_token->oauth_token_secret);
	
	HTTPConnection_free(&http_connection);
	
	sprintf(request_token_url, "http://api.twitter.com/oauth/authenticate?oauth_token=%s", current_oauth_token->oauth_token);
	
	return 0;
}

int TwitterAPI_oauth_authorize_from_pin(char *pin) {
	if(current_oauth_token->oauth_token_secret == NULL) {
		return -1;
	}
	
	current_oauth_token->oauth_verifier = strdup(pin);

	HTTPConnection http_connection;
	HTTPConnection_initialize(&http_connection);
	
	http_connection.url = strdup("http://api.twitter.com/oauth/access_token");
	http_connection.connection_method = HTTPConnectionMethodPOST;
	
	http_connection.first_parameter = malloc(sizeof(*http_connection.first_parameter));
	HTTPParameter_initialize(http_connection.first_parameter);
	http_connection.first_parameter->key = strdup("oauth_verifier");
	http_connection.first_parameter->value = strdup(current_oauth_token->oauth_verifier);
	
	HTTPParameter *second_parameter = malloc(sizeof(*http_connection.first_parameter));
	HTTPParameter_initialize(second_parameter);
	second_parameter->key = strdup("oauth_token");
	second_parameter->value = strdup(current_oauth_token->oauth_token);
	http_connection.first_parameter->next_parameter = second_parameter;
	
	TwitterAPI_oauth_authenticate_connection(&http_connection);
	
	int status = HTTPConnection_perform_request(&http_connection);
	if(status != 0) return status;
	
	printf(" body: %s\n", http_connection.response_buffer);
	
	OAuthOAuthToken_free(current_oauth_token);
	current_oauth_token = NULL;
	
	current_access_token = malloc(sizeof(*current_access_token));
	OAuthAccessToken_initialize(current_access_token);
	
	int i = 0;
	char *substring = strtok(http_connection.response_buffer, "=&");
	
	while(substring != NULL)
	{
		if(i == 1) { // oauth_token
			current_access_token->access_token = strdup(substring);
		} else if(i == 3) { // oauth_token_secret
			current_access_token->access_token_secret = strdup(substring);
		}
		
		i += 1;
		substring = strtok(NULL, "=&");
	}
	
	if(i < 5) {
		return -1;
	}
	
	printf("access_token: %s\n", current_access_token->access_token);
	printf("access_token_secret: %s\n", current_access_token->access_token_secret);
	
	// current_oauth_token = malloc(sizeof(*current_oauth_token));
	// OAuthOAuthToken_initialize(current_oauth_token);
	// 
	// int i = 0;
	// char *substring = strtok(http_connection.response_buffer, "=&");
	// 
	// while(substring != NULL)
	// {
	// 	if(i == 1) { // oauth_token
	// 		current_oauth_token->oauth_token = strdup(substring);
	// 	} else if(i == 3) { // oauth_token_secret
	// 		current_oauth_token->oauth_token_secret = strdup(substring);
	// 	}
	// 	
	// 	i += 1;
	// 	substring = strtok(NULL, "=&");
	// }
	// 
	// if(i < 5) {
	// 	return -1;
	// }
	// 
	// printf("oauth_token: %s\n", current_oauth_token->oauth_token);
	// printf("oauth_token_secret: %s\n", current_oauth_token->oauth_token_secret);
	// 
	HTTPConnection_free(&http_connection);
	// 
	// sprintf(request_token_url, "http://api.twitter.com/oauth/authenticate?oauth_token=%s", current_oauth_token->oauth_token);
	
	return 0;
}

