#include <stdio.h>
#include "twitter_api.h"

int main() {
	// HTTPConnection httpConnection;
	// HTTPConnection_initialize(&httpConnection);
	// httpConnection.url = "http://www.google.com.br/";
	// 
	// HTTPConnection_perform_request(&httpConnection);
	// 
	// // printf(" response buffer: %s\n", httpConnection.response_buffer);
	// 
	// HTTPConnection_free(&httpConnection);
	
	// while(1 == 1) {
	Tweet *first_search_result;
	int asd = TwitterAPI_search("iphone", &first_search_result);
	
	// printf("first_search_result 2: %i\n", first_search_result);
	
	Tweet *current_tweet = first_search_result;
	while(current_tweet != NULL) {
		printf("current tweet: %s\n", current_tweet->text);
		current_tweet = current_tweet->next_tweet;
	}
	
	Tweet_free(current_tweet, 1);
	// sleep(3);
	// }
	
	return 0;
}