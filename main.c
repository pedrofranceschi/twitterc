#include <stdio.h>
#include "twitter_api.h"

int main(int argc, char *argv[]) {
	int access_token_load_error = OAuthAccessToken_load_from_file();
	printf("access_token_load_error: %i\n", access_token_load_error);
	
	if(access_token_load_error != 0) { // error
		char *url = (char *)malloc(sizeof(char) * 150);
		int error = TwitterAPI_oauth_request_token_url(url);
		printf("URL: %s\n", url);
		free(url);
	
		char str[80];
		int i;
	
		printf("Enter the PIN: ");
		fgets(str, 10, stdin);
	
	/* remove newline, if present */
		i = strlen(str)-1;
		if( str[ i ] == '\n') 
			str[i] = '\0';
	
		TwitterAPI_oauth_authorize_from_pin(str);
	}
	
	TwitterUser twitter_user;
	TwitterUser_initialize(&twitter_user);
	twitter_user.screen_name = strdup("testingthis15");
	
	Tweet retweeted_tweet;
	Tweet_initialize(&retweeted_tweet);
	retweeted_tweet.id_str = strdup("260885034544795651");
	
	Tweet *first_tweet;
	// int error = TwitterAPI_user_timeline(&first_tweet, &twitter_user);
	int error = TwitterAPI_get_retweets(&retweeted_tweet, &first_tweet);
	
	Tweet *current_tweet = first_tweet;
	while(current_tweet != NULL) {
		printf("current tweet: %s\n", current_tweet->text);
		char *date_str = _relative_time(current_tweet->created_at);
		printf("date (%i): %s ago\n", current_tweet->created_at, date_str);
		free(date_str);
		printf("user name: %s (%s) - %i\n", current_tweet->author->name, current_tweet->author->screen_name, current_tweet->author->id_str);
		printf(" id: %s\n\n", current_tweet->id_str);
		current_tweet = (Tweet *)current_tweet->next_tweet;
	}
}
