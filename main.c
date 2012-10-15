#include <stdio.h>
#include "twitter_api.h"

int main() {	
	// while(1 == 1) {
	// Tweet *first_search_result;
	// int asd = TwitterAPI_search("@pedroh96", &first_search_result);
	// 
	// Tweet *current_tweet = first_search_result;
	// while(current_tweet != NULL) {
	// 	printf("current (%s) tweet: %s\n", current_tweet->id_str, current_tweet->text);
	// 	printf("date: %s", asctime(current_tweet->created_at));
	// 	printf("user name: %s (%s) - %i\n", current_tweet->author->name, current_tweet->author->screen_name, current_tweet->author->user_id);
	// 	current_tweet = current_tweet->next_tweet;
	// }
	// 
	// Tweet_free(current_tweet, 1);
	// sleep(3);
	// }	
	
	// while(1) {
	// HTTPConnection http_connection;
	// HTTPConnection_initialize(&http_connection);
	// 
	// http_connection.url = malloc(sizeof(char) * 100);
	// http_connection.connection_method = HTTPConnectionMethodPOST;
	// strcpy(http_connection.url, "https://api.twitter.com/oauth/request_token");
	// 
	// http_connection.first_parameter = malloc(sizeof(*http_connection.first_parameter));
	// HTTPParameter_initialize(http_connection.first_parameter);
	// http_connection.first_parameter->key = strdup("oauth_callback");
	// http_connection.first_parameter->value = strdup("oob");
	// http_connection.first_parameter->type = HTTPParameterTypeAuthorizationHeader;
	// // printf("http_connection.first_parameter: %i\n", http_connection.first_parameter);
	// 
	// // TwitterAPI_oauth_
	// TwitterAPI_oauth_authenticate_connection(&http_connection, NULL);
	// 
	// int status = HTTPConnection_perform_request(&http_connection);
	// 
	// printf(" body: %s\n", http_connection.response_buffer);
	// HTTPConnection_free(&http_connection);
	
	int access_token_load_error = OAuthAccessToken_load_from_file();
	printf("access_token_load_error: %i\n", access_token_load_error);
	
	if(access_token_load_error != 0) { // error
		char *url = malloc(sizeof(char) * 150);
		int success = TwitterAPI_oauth_request_token_url(url);
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
		
		TwitterAPI_oauth_authorize_from_pin(&str);
	}
	
	Tweet *first_tweet;	
	int success = TwitterAPI_home_timeline(&first_tweet);
	
	Tweet *current_tweet = first_tweet;
	while(current_tweet != NULL) {
		printf("current tweet: %s\n", current_tweet->text);
		printf("date: %s", asctime(current_tweet->created_at));
		printf("user name: %s (%s) - %i\n", current_tweet->author->name, current_tweet->author->screen_name, current_tweet->author->user_id);
	 	printf(" id: %s\n", current_tweet->id_str);
		current_tweet = current_tweet->next_tweet;
	}
	
	// 
	// // while(1) {
	// 	Tweet *first_tweet_timeline;	
	// 	success = TwitterAPI_home_timeline(&first_tweet_timeline);
	// // }
	
	
	
	// free(http_connection);
	// sleep(5);
	// }
	
	return 0;
}