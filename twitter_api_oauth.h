#include "http_connection.h"
#include <openssl/engine.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <wordexp.h>

#define OAUTH_CONSUMER_KEY "LPcrqudMsYNNVDeBnBzg"
#define OAUTH_CONSUMER_SECRET "C56sD93snQCf2zyJZHb7HT2bbEkgwzgMipG2mlsfQ6c"
#define ACCESS_TOKEN_FILE "~/.twitterc"

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
int OAuthAccessToken_load_from_file();
int OAuthAccessToken_save_current_to_file();

void TwitterAPI_oauth_authenticate_connection(HTTPConnection *http_connection);
int TwitterAPI_oauth_request_token_url(char *request_token_url);
int TwitterAPI_oauth_authorize_from_pin(char *pin);