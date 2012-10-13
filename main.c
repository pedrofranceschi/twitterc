#include <stdio.h>
#include "twitter_api.h"

int main() {	
	// while(1 == 1) {
	// Tweet *first_search_result;
	// int asd = TwitterAPI_search("iphone", &first_search_result);
	// 
	// Tweet *current_tweet = first_search_result;
	// while(current_tweet != NULL) {
	// 	printf("current tweet: %s\n", current_tweet->text);
	// 	printf("date: %s", asctime(current_tweet->created_at));
	// 	printf("user name: %s (%s) - %i\n", current_tweet->author->name, current_tweet->author->screen_name, current_tweet->author->user_id);
	// 	current_tweet = current_tweet->next_tweet;
	// }
	// 
	// Tweet_free(current_tweet, 1);
	// sleep(3);
	// }	
	
	HTTPConnection httpConnection;
	HTTPConnection_initialize(&httpConnection);
	
	httpConnection.url = malloc(sizeof(char) * 100);
	httpConnection.connection_method = HTTPConnectionMethodPOST;
	strcpy(httpConnection.url, "https://api.twitter.com/oauth/request_token");
	
	httpConnection.first_parameter = malloc(sizeof(*httpConnection.first_parameter));
	HTTPParameter_initialize(httpConnection.first_parameter);
	httpConnection.first_parameter->key = strdup("oauth_callback");
	httpConnection.first_parameter->value = strdup("oob");
	httpConnection.first_parameter->type = HTTPParameterTypeAuthorizationHeader;
	
	// TwitterAPI_oauth_
	TwitterAPI_oauth_authenticate_connection(&httpConnection);
	
	int status = HTTPConnection_perform_request(&httpConnection);
	
	printf(" body: %s\n", httpConnection.response_buffer);
	HTTPConnection_free(&httpConnection);
	// free(httpConnection);
	
	return 0;
}