#include "twitter_api.h"

// helper methods

char *_relative_time(time_t past_time) {
	time_t current_time = time(NULL);
	struct tm *time_with_zone = localtime(&current_time);
	current_time = (time_t)((int)time(NULL) - time_with_zone->tm_gmtoff);
	int relative_seconds = current_time - past_time;
	
	char *relative_time_str = malloc(sizeof(char) * 100);
	
	if(relative_seconds < 0) {
		relative_time_str = strdup("future");
	} else if(relative_seconds < 60) {
		sprintf(relative_time_str, "%i second%s", relative_seconds, (relative_seconds != 1 ? "s" : ""));
	} else if(relative_seconds < 60*60) {
		int minutes = (int)floor(relative_seconds/60);
		sprintf(relative_time_str, "%i minute%s", minutes, (minutes != 1 ? "s" : ""));
	} else if(relative_seconds < 60*60*24) {
		int hours = (int)floor(relative_seconds/(60*60));
		sprintf(relative_time_str, "%i hour%s", hours, (hours != 1 ? "s" : ""));
	} else if(relative_seconds < 60*60*24*30) {
		int days = (int)floor(relative_seconds/(60*60*24));
		sprintf(relative_time_str, "%i day%s", days, (days != 1 ? "s" : ""));
	} else if(relative_seconds < 60*60*24*30*12) {
		int months = (int)floor(relative_seconds/(60*60*24*30));
		sprintf(relative_time_str, "%i month%s", months, (months != 1 ? "s" : ""));
	} else {
		int years = (int)floor(relative_seconds/(60*60*24*30*12));
		sprintf(relative_time_str, "%i year%s", years, (years != 1 ? "s" : ""));
	}
	
	return relative_time_str;
}

time_t _date_from_twitter_date(char *twitter_date, int is_search) {
	struct tm date;// = malloc(sizeof(*date));
	if(is_search == 1) {
		strptime(twitter_date, "%a, %d %b %Y %T +0000", &date);
	} else {
		strptime(twitter_date, "%a %b %d %T +0000 %Y", &date);
	}
	
	return mktime(&date);
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
	tweet->id_str = NULL;
	tweet->author = NULL;
	tweet->created_at = NULL;
	tweet->next_tweet = NULL;
	tweet->previous_tweet = NULL;
}

void _Tweet_free(Tweet *tweet) {	
	free(tweet->text);
	free(tweet->id_str);
	
	if(tweet->author) {
		TwitterUser_free(tweet->author);
	}
	
	free(tweet);
}

void Tweet_free(Tweet *tweet, int free_related_tweets) {
	if(free_related_tweets != 0) {
		Tweet *current_tweet = tweet;
		while(current_tweet != NULL) {
			Tweet *new_current_tweet = (Tweet *)current_tweet->next_tweet;
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
		char *current_tweet_text = (char *)json_object_get_string(current_tweet_text_json);
		current_tweet->text = (char *)strdup(current_tweet_text);
		// json_object_put(current_tweet_text_json);
		
		struct json_object *current_tweet_created_at_json = json_object_object_get(current_tweet_json, "created_at");
		char *current_tweet_created_at = strdup((char *)json_object_get_string(current_tweet_created_at_json));
		current_tweet->created_at = malloc(sizeof(struct tm));
		_date_from_twitter_date(current_tweet_created_at, 0);
		free(current_tweet_created_at);
		// json_object_put(current_tweet_created_at_json);
		
		struct json_object *current_tweet_id_str_json = json_object_object_get(current_tweet_json, "id_str");
		char *current_tweet_id_str = (char *)json_object_get_string(current_tweet_id_str_json);
		current_tweet->id_str = strdup(current_tweet_id_str);
		// json_object_put(current_tweet_id_str_json);
		// printf("current_tweet_id_str: %s\n", current_tweet->id_str);
				
		
		TwitterUser *current_tweet_author = malloc(sizeof(*current_tweet_author));
		TwitterUser_initialize(current_tweet_author);
		
		struct json_object *current_tweet_author_name_json = json_object_object_get(current_tweet_json, "from_user_name");
		char *current_tweet_author_name = (char *)json_object_get_string(current_tweet_author_name_json);
		current_tweet_author->name = (char *)strdup(current_tweet_author_name);
		// json_object_put(current_tweet_author_name_json);
		
		struct json_object *current_tweet_author_screen_name_json = json_object_object_get(current_tweet_json, "from_user");
		char *current_tweet_author_screen_name = (char *)json_object_get_string(current_tweet_author_screen_name_json);
		current_tweet_author->screen_name = (char *)strdup(current_tweet_author_screen_name);
		// json_object_put(current_tweet_author_screen_name_json);
		
		struct json_object *current_tweet_author_id_json = json_object_object_get(current_tweet_json, "from_user_id");
		current_tweet_author->user_id = (int)json_object_get_int(current_tweet_author_id_json);
		// json_object_put(current_tweet_author_id_json);
		
		current_tweet->author = current_tweet_author;
		
		if(*first_search_result == NULL) {
			current_tweet->previous_tweet = NULL;
			*first_search_result = current_tweet;
			previous_tweet = (Tweet *)*first_search_result;
		} else {
			previous_tweet->next_tweet = current_tweet;
			current_tweet->previous_tweet = previous_tweet;
			previous_tweet = current_tweet;
		}
		
		// json_object_put(current_tweet_json);
	}
	
	previous_tweet->next_tweet = NULL;
	
	// printf("body: %s\n", http_connection.response_buffer);
	
	// free(search_api_url);
	// json_object_put(tweets);
	json_object_put(json_parser);
	// free(tweets);
	// free(json_parser);
	HTTPConnection_free(&http_connection);
	
	return 0;
}

int _parse_timeline_from_json(char *json_response, Tweet **first_tweet) {
	struct json_object *json_parser = json_tokener_parse(json_response);
	
	if(json_parser == NULL || is_error(json_parser)) return -1;
	if(json_object_get_type(json_parser) != json_type_array) return -2;
	
	Tweet *previous_tweet = NULL;
	*first_tweet = NULL;
	
	int i;
	
	for(i = 0; i < json_object_array_length(json_parser); i++) {
		Tweet *current_tweet = malloc(sizeof(*current_tweet));
		Tweet_initialize(current_tweet);
		
		struct json_object *current_tweet_json = json_object_array_get_idx(json_parser, i);
		
		struct json_object *current_tweet_text_json = json_object_object_get(current_tweet_json, "text");
		char *current_tweet_text = (char *)json_object_get_string(current_tweet_text_json);
		current_tweet->text = (char *)strdup(current_tweet_text);
		// json_object_put(current_tweet_text_json);
		
		printf("1\n");
		
		struct json_object *current_tweet_created_at_json = json_object_object_get(current_tweet_json, "created_at");
		char *current_tweet_created_at = strdup((char *)json_object_get_string(current_tweet_created_at_json));
		current_tweet->created_at = _date_from_twitter_date(current_tweet_created_at, 0);
		free(current_tweet_created_at);
		// json_object_put(current_tweet_created_at_json);
		
		printf("2 \n");
		
		struct json_object *current_tweet_id_str_json = json_object_object_get(current_tweet_json, "id_str");
		char *current_tweet_id_str = (char *)json_object_get_string(current_tweet_id_str_json);
		// printf("current_tweet_id_str: %s\n", current_tweet_id_str);
		current_tweet->id_str = strdup(current_tweet_id_str);
		// json_object_put(current_tweet_id_str_json);
		
		printf("3\n");
		
		TwitterUser *current_tweet_author = malloc(sizeof(*current_tweet_author));
		TwitterUser_initialize(current_tweet_author);
		
		struct json_object *current_tweet_user_json = json_object_object_get(current_tweet_json, "user");
		
		printf("4\n");
		
		struct json_object *current_tweet_author_name_json = json_object_object_get(current_tweet_user_json, "name");
		char *current_tweet_author_name = (char *)json_object_get_string(current_tweet_author_name_json);
		current_tweet_author->name = (char *)strdup(current_tweet_author_name);
		// json_object_put(current_tweet_author_name_json);
		
		struct json_object *current_tweet_author_screen_name_json = json_object_object_get(current_tweet_user_json, "screen_name");
		char *current_tweet_author_screen_name = (char *)json_object_get_string(current_tweet_author_screen_name_json);
		current_tweet_author->screen_name = (char *)strdup(current_tweet_author_screen_name);
		// json_object_put(current_tweet_author_screen_name_json);
		
		struct json_object *current_tweet_author_id_json = json_object_object_get(current_tweet_user_json, "id_str");
		current_tweet_author->user_id = (int)json_object_get_int(current_tweet_author_id_json);
		// json_object_put(current_tweet_author_id_json);
		
		current_tweet->author = current_tweet_author;
		
		if(*first_tweet == NULL) {
			current_tweet->previous_tweet = NULL;
			*first_tweet = current_tweet;
			previous_tweet = *first_tweet;
		} else {
			previous_tweet->next_tweet = current_tweet;
			current_tweet->previous_tweet = previous_tweet;
			previous_tweet = current_tweet;
		}
		
		// json_object_put(current_tweet_json);
		// json_object_put(current_tweet_text_json);
		// json_object_put(current_tweet_user_json);
		json_object_put(current_tweet_json);
	}
	
	previous_tweet->next_tweet = NULL;
	
	// free(search_api_url);
	// json_object_put(tweets);
	// free(tweets);
	// json_object_put(json_parser);
	// free(json_parser);
	json_object_put(json_parser);
	
	return 0;
}

int TwitterAPI_home_timeline(Tweet **first_tweet) {	
	HTTPConnection http_connection;
	HTTPConnection_initialize(&http_connection);
	http_connection.url = strdup("https://api.twitter.com/1.1/statuses/home_timeline.json");
	TwitterAPI_oauth_authenticate_connection(&http_connection);
	
	int status = HTTPConnection_perform_request(&http_connection);
	if(status != 0) return status;
	
	printf(" body: %s\n", http_connection.response_buffer);
	
	status = _parse_timeline_from_json(http_connection.response_buffer, first_tweet);
	HTTPConnection_free(&http_connection);
	return status;
}

int TwitterAPI_mentions_timeline(Tweet **first_tweet) {	
	HTTPConnection http_connection;
	HTTPConnection_initialize(&http_connection);
	http_connection.url = strdup("https://api.twitter.com/1.1/statuses/mentions_timeline.json");
	TwitterAPI_oauth_authenticate_connection(&http_connection);
	
	int status = HTTPConnection_perform_request(&http_connection);
	if(status != 0) return status;
	
	printf(" body: %s\n", http_connection.response_buffer);
	
	status = _parse_timeline_from_json(http_connection.response_buffer, first_tweet);
	// printf(" got \n");
	HTTPConnection_free(&http_connection);
	return status;
}

int TwitterAPI_user_timeline(Tweet **first_tweet, TwitterUser *user) {	
	HTTPConnection http_connection;
	HTTPConnection_initialize(&http_connection);
	http_connection.url = strdup("https://api.twitter.com/1.1/statuses/user_timeline.json");
	TwitterAPI_oauth_authenticate_connection(&http_connection);
	
	int status = HTTPConnection_perform_request(&http_connection);
	if(status != 0) return status;
	
	printf(" body: %s\n", http_connection.response_buffer);
	
	status = _parse_timeline_from_json(http_connection.response_buffer, first_tweet);
	HTTPConnection_free(&http_connection);
	return status;
}
