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
	
	char *signature_base_string = malloc(sizeof(char) * parameters_count * 200);
	
	for(i = 0; i < parameters_count; i++) {
		current_parameter = &(http_parameters[i]);
		if(current_parameter->type != HTTPParameterTypeAuthorizationHeader) {
			sprintf(signature_base_string, "%s%s=%s&", signature_base_string, current_parameter->key, current_parameter->value);
		}
	}
	
	signature_base_string[strlen(signature_base_string) - 1] = '\0'; // removes last '&'

	sprintf(signature_base_string, "%s&%s&%s", (http_connection->connection_method == HTTPConnectionMethodPOST ? "POST" : "GET"), _html_escape_string(http_connection->url), _html_escape_string(signature_base_string));
	
	char *sha1_result = malloc(sizeof(char) * 100);
	hmac_sha1("5K2voGiykVvfZ5R5agH8MXxOAblqpLZ12JVzp8Fwr2w&", 44, signature_base_string, strlen(signature_base_string), sha1_result);
	free(signature_base_string);
	
	char *final_signature = malloc(sizeof(char) * 100);
	b64_encode(sha1_result, final_signature);
	free(sha1_result);
	printf("final_signature: %s\n", final_signature);
	
	return final_signature;
}

void TwitterAPI_oauth_authenticate_connection(HTTPConnection *http_connection) {
	HTTPParameter *oauth_consumer_key_parameter = _create_parameter(strdup("oauth_consumer_key"), strdup("CjNHZ0SUxT3mLfFnxAYeJA"));
	
	char *random_oauth_nonce = malloc(sizeof(char) * 32);
	_generate_random_string(random_oauth_nonce, 32);
	HTTPParameter *oauth_nonce_parameter = _create_parameter(strdup("oauth_nonce"), random_oauth_nonce);
	oauth_consumer_key_parameter->next_parameter = oauth_nonce_parameter;
	
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
	while(current_parameter != NULL) {
		if(current_parameter->type == HTTPParameterTypeAuthorizationHeader) { // needs to be on authorization header
			last_configured_parameter->next_parameter = malloc(sizeof(*current_parameter));
			// memcpy(last_configured_parameter->next_parameter, current_parameter, sizeof(*current_parameter));
			// duplicating...
			((HTTPParameter *)last_configured_parameter->next_parameter)->key = strdup(current_parameter->key);
			((HTTPParameter *)last_configured_parameter->next_parameter)->value = strdup(current_parameter->value);
			((HTTPParameter *)last_configured_parameter->next_parameter)->previous_parameter = last_configured_parameter;
			((HTTPParameter *)last_configured_parameter->next_parameter)->type  = HTTPParameterTypeIgnore;
			last_configured_parameter = last_configured_parameter->next_parameter;
		}
		current_parameter = current_parameter->next_parameter;
	}
	
	last_configured_parameter->next_parameter = http_connection->first_parameter;
	http_connection->first_parameter->previous_parameter = last_configured_parameter;
	
	char *signature_string = _generate_signature_for_parameters(oauth_consumer_key_parameter, http_connection);

	// undoing header scheme created to generate signature
	last_configured_parameter->next_parameter = NULL;
	http_connection->first_parameter->previous_parameter = NULL;
	
	HTTPParameter *oauth_signature_parameter = _create_parameter(strdup("oauth_signature"), strdup(_html_escape_string(signature_string)));
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
	
	HTTPParameter *last_parameter = http_connection->first_parameter;
	while(1) {
		if(last_parameter->next_parameter == NULL) {
			break;
		}
		last_parameter = last_parameter->next_parameter;
	}
	
	last_parameter->next_parameter = authorization_parameter;
	
	HTTPParameter_free(oauth_consumer_key_parameter, 1);
}
