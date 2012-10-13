#include "http_connection.h"

#define OAUTH_CONSUMER_KEY "CjNHZ0SUxT3mLfFnxAYeJA"
#define OAUTH_CONSUMER_SECRET "5K2voGiykVvfZ5R5agH8MXxOAblqpLZ12JVzp8Fwr2w"

typedef struct {
	char *oauth_token;
	char *oauth_token_secret;
	char *oauth_verifier;
} OAuthOAuthToken;

typedef struct {
	char *access_token;
	char *access_token_secret;
} OAuthAccessToken;

OAuthOAuthToken *current_oauth_token;
OAuthAccessToken *current_access_token;

void OAuthOAuthToken_initialize(OAuthOAuthToken *oauth_token);
void OAuthOAuthToken_free(OAuthOAuthToken *oauth_token);
void OAuthAccessToken_initialize(OAuthAccessToken *access_token);
void OAuthAccessToken_free(OAuthAccessToken *access_token);

void TwitterAPI_oauth_authenticate_connection(HTTPConnection *http_connection);
int TwitterAPI_oauth_request_token_url(char *request_token_url);
int TwitterAPI_oauth_authorize_from_pin(char *pin);