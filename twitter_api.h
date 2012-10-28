#include <json/json.h>
#include <time.h>
// #include "http_connection.h"
#include "twitter_api_oauth.h"

typedef struct {
	char *name;
	char *screen_name;
	char *id_str;
} TwitterUser;

typedef struct {
	char *text;
	char *id_str;
	TwitterUser *author;
	time_t created_at;
	struct Tweet *next_tweet, *previous_tweet;
} Tweet;

char *_relative_time(time_t past_time);
char *_html_escape_string(char *string);
void TwitterUser_free(TwitterUser *user);
void Tweet_free(Tweet *tweet, int free_related_tweets_too);

int TwitterAPI_search(char *search_term, Tweet **first_search_result);
int TwitterAPI_home_timeline(Tweet **first_tweet);
int TwitterAPI_mentions_timeline(Tweet **first_tweet);
int TwitterAPI_user_timeline(Tweet **first_tweet, TwitterUser *user);
int TwitterAPI_statuses_update(char *text, Tweet *in_reply_to_tweet);
int TwitterAPI_statuses_destroy(Tweet *statuses);
int TwitterAPI_get_retweets(Tweet *tweet, Tweet **first_tweet);
int TwitterAPI_statuses_retweet(Tweet *tweet);