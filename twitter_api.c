#include "twitter_api.h"

// helper methods

struct tm *_date_from_twitter_date(char *twitter_date) {
	struct tm *date = malloc(sizeof(*date));
	strptime(twitter_date, "%a, %d %b %Y %T +0000", date);
	return date;
}

// struct methods

void TwitterUser_initialize(TwitterUser *user) {
	user->name = NULL;
	user->screen_name = NULL;
	user->user_id = 0;
}

void TwitterUser_free(TwitterUser *user) {
	free(user->name);
	free(user->screen_name);
	free(user);
}

void Tweet_initialize(Tweet *tweet) {
	tweet->text = NULL;
	tweet->author = NULL;
	tweet->created_at = NULL;
	tweet->next_tweet = NULL;
	tweet->previous_tweet = NULL;
}

void _Tweet_free(Tweet *tweet) {	
	free(tweet->text);
	free(tweet->created_at);
	
	if(tweet->author) {
		TwitterUser_free(tweet->author);
	}
	
	free(tweet);
}

void Tweet_free(Tweet *tweet, int free_related_tweets) {
	if(free_related_tweets != 0) {
		Tweet *current_tweet = tweet;
		while(current_tweet != NULL) {
			Tweet *new_current_tweet = current_tweet->next_tweet;
			_Tweet_free(current_tweet);
			current_tweet = new_current_tweet;
		}
	} else {
		_Tweet_free(tweet);
	}
}

// twitter api methods

int TwitterAPI_search(char *search_term, Tweet **first_search_result) {	
	HTTPConnection http_connection;
	HTTPConnection_initialize(&http_connection);
	
	char *search_api_url = malloc(sizeof(char) * 150);
	strcpy(search_api_url, "http://search.twitter.com/search.json?q=");
	strcat(search_api_url, _html_escape_string(search_term));
	http_connection.url = search_api_url;
	
	int status = HTTPConnection_perform_request(&http_connection);
	if(status != 0) return status;
	
	struct json_object *json_parser = json_tokener_parse(http_connection.response_buffer);
	if(json_parser == NULL) return -1;
	struct json_object *tweets = json_object_object_get(json_parser, "results");
	if(tweets == NULL) return -2;
	
	Tweet *previous_tweet = NULL;
	*first_search_result = NULL;
	
	int i;
	
	for(i = 0; i < json_object_array_length(tweets); i++) {
		Tweet *current_tweet = malloc(sizeof(*current_tweet));
		Tweet_initialize(current_tweet);
		
		struct json_object *current_tweet_json = json_object_array_get_idx(tweets, i);
		
		struct json_object *current_tweet_text_json = json_object_object_get(current_tweet_json, "text");
		char *current_tweet_text = json_object_get_string(current_tweet_text_json);
		current_tweet->text = (char *)strdup(current_tweet_text);
		free(current_tweet_text);
		free(current_tweet_text_json);
		
		struct json_object *current_tweet_created_at_json = json_object_object_get(current_tweet_json, "created_at");
		char *current_tweet_created_at = json_object_get_string(current_tweet_created_at_json);
		current_tweet->created_at = _date_from_twitter_date(current_tweet_created_at);
		free(current_tweet_created_at_json);
		free(current_tweet_created_at);
		
		
		TwitterUser *current_tweet_author = malloc(sizeof(*current_tweet_author));
		TwitterUser_initialize(current_tweet_author);
		
		struct json_object *current_tweet_author_name_json = json_object_object_get(current_tweet_json, "from_user_name");
		char *current_tweet_author_name = json_object_get_string(current_tweet_author_name_json);
		current_tweet_author->name = (char *)strdup(current_tweet_author_name);
		free(current_tweet_author_name_json);
		free(current_tweet_author_name);
		
		struct json_object *current_tweet_author_screen_name_json = json_object_object_get(current_tweet_json, "from_user");
		char *current_tweet_author_screen_name = json_object_get_string(current_tweet_author_screen_name_json);
		current_tweet_author->screen_name = (char *)strdup(current_tweet_author_screen_name);
		free(current_tweet_author_screen_name_json);
		free(current_tweet_author_screen_name);
		
		struct json_object *current_tweet_author_id_json = json_object_object_get(current_tweet_json, "from_user_id");
		current_tweet_author->user_id = json_object_get_int(current_tweet_author_id_json);
		free(current_tweet_author_id_json);
		
		current_tweet->author = current_tweet_author;
		
		
		if(*first_search_result == NULL) {
			current_tweet->previous_tweet = NULL;
			*first_search_result = current_tweet;
			previous_tweet = *first_search_result;
		} else {
			previous_tweet->next_tweet = current_tweet;
			current_tweet->previous_tweet = previous_tweet;
			previous_tweet = current_tweet;
		}
		
		// json_object_put(current_tweet_json);
		// json_object_put(current_tweet_text_json);
		free(current_tweet_json);
	}
	
	previous_tweet->next_tweet = NULL;
	
	// free(search_api_url);
	// json_object_put(tweets);
	free(tweets);
	json_object_put(json_parser);
	free(json_parser);
	HTTPConnection_free(&http_connection);
	
	return 0;
}

int TwitterAPI_home_timeline(Tweet **first_tweet) {
	// http://api.twitter.com/1.1/statuses/home_timeline.json
	
	HTTPConnection http_connection;
	HTTPConnection_initialize(&http_connection);
	
	// char *search_api_url = malloc(sizeof(char) * 150);
	// strcpy(search_api_url, "http://search.twitter.com/search.json?q=");
	// strcat(search_api_url, _html_escape_string(search_term));
	http_connection.url = strdup("http://api.twitter.com/1.1/statuses/home_timeline.json");
	
	// http_connection.first_parameter = malloc(sizeof(*http_connection.first_parameter));
	// HTTPParameter_initialize(http_connection.first_parameter);
	// http_connection.first_parameter->key = strdup("count");
	// http_connection.first_parameter->value = strdup("20");
	
	TwitterAPI_oauth_authenticate_connection(&http_connection);
	
	int status = HTTPConnection_perform_request(&http_connection);
	if(status != 0) return status;
	
	printf(" body: %s\n", http_connection.response_buffer);
	
	HTTPConnection_free(&http_connection);
	
	// struct json_object *json_parser = json_tokener_parse(http_connection.response_buffer);
	// if(json_parser == NULL) return -1;
	// struct json_object *tweets = json_object_object_get(json_parser, "results");
	// if(tweets == NULL) return -2;
	// 
	// Tweet *previous_tweet = NULL;
	// *first_search_result = NULL;
	
}
