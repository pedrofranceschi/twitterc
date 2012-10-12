#include <json/json.h>
#include <time.h>
// #include "http_connection.h"
#include "twitter_api_oauth.h"

typedef struct {
	char *name;
	char *screen_name;
	int user_id;
} TwitterUser;

typedef struct {
	char *text;
	TwitterUser *author;
	struct tm *created_at;
	struct Tweet *next_tweet, *previous_tweet;
} Tweet;

char *_html_escape_string(char *string);
void TwitterUser_free(TwitterUser *user);
void Tweet_free(Tweet *tweet, int free_related_tweets_too);

int TwitterAPI_search(char *search_term, Tweet **first_search_result);