#include "twitter_api.h"

// helper methods

char *_char_replace(char *st, char *orig, char *repl) {
	static char buffer[4096];
	char *ch;
	if (!(ch = strstr(st, orig)))
		return st;
	strncpy(buffer, st, ch-st);  
	buffer[ch-st] = 0;
	sprintf(buffer+(ch-st), "%s%s", repl, ch+strlen(orig));
	return buffer;
}

char *_html_escape_string(char *string) {
	return _char_replace(_char_replace(string, " ", "%20"), "#", "%23"); // TODO: escape more stuff
}

// struct methods

void TwitterUser_initialize(TwitterUser *user) {
	user->name = NULL;
	user->screen_name = NULL;
	user->bio = NULL;
}

void TwitterUser_free(TwitterUser *user) {
	free(user->name);
	free(user->screen_name);
	free(user->screen_name);
}

void Tweet_initialize(Tweet *tweet) {
	tweet->text = NULL;
	tweet->author = NULL;
	tweet->timestamp = 0;
	tweet->next_tweet = NULL;
	tweet->previous_tweet = NULL;
}

void _Tweet_free(Tweet *tweet) {	
	free(tweet->text);
	
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
	HTTPConnection httpConnection;
	HTTPConnection_initialize(&httpConnection);
	
	char *search_api_url = malloc(sizeof(char) * 150);
	strcpy(search_api_url, "http://search.twitter.com/search.json?q=");
	strcat(search_api_url, _html_escape_string(search_term));
	httpConnection.url = search_api_url;
	
	int status = HTTPConnection_perform_request(&httpConnection);
	if(status != 0) return status;
	
	struct json_object *tweets = json_object_object_get(json_tokener_parse(httpConnection.response_buffer), "results");
	
	Tweet *previous_tweet = NULL;
	*first_search_result = NULL;
	
	int i;
	
	for(i = 0; i < json_object_array_length(tweets); i++) {
		Tweet *current_tweet = malloc(sizeof(*current_tweet));
		Tweet_initialize(current_tweet);
		
		struct json_object *current_tweet_json = json_object_array_get_idx(tweets, i);
		struct json_object *current_tweet_text_json = json_object_object_get(current_tweet_json, "text");
		
		current_tweet->text = (char *)strdup(json_object_get_string(current_tweet_text_json));
		
		if(*first_search_result == NULL) {
			current_tweet->previous_tweet = NULL;
			*first_search_result = current_tweet;
			previous_tweet = *first_search_result;
		} else {
			previous_tweet->next_tweet = current_tweet;
			current_tweet->previous_tweet = previous_tweet;
			previous_tweet = current_tweet;
		}
		
		free(current_tweet_json);
		free(current_tweet_text_json);
	}
	
	previous_tweet->next_tweet = NULL;
	
	free(search_api_url);
	free(tweets);
	HTTPConnection_free(&httpConnection);
	
	return 0;
}