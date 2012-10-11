#include "http_connection.h"
#include <json/json.h>

typedef struct {
	char *name;
	char *screen_name;
	char *bio;
} TwitterUser;

typedef struct {
	char *text;
	TwitterUser *author;
	int timestamp;
	struct Tweet *next_tweet, *previous_tweet;
} Tweet;

void TwitterUser_free(TwitterUser *user);
void Tweet_free(Tweet *tweet, int free_related_tweets_too);

int TwitterAPI_search(char *search_term, Tweet **first_search_result);