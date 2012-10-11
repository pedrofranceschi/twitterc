#include <stdio.h>
#include "twitter_api.h"

int main() {	
	// while(1 == 1) {
	Tweet *first_search_result;
	int asd = TwitterAPI_search("iphone", &first_search_result);
	
	Tweet *current_tweet = first_search_result;
	while(current_tweet != NULL) {
		printf("current tweet: %s\n", current_tweet->text);
		printf("date: %s", asctime(current_tweet->created_at));
		printf("user name: %s (%s) - %i\n", current_tweet->author->name, current_tweet->author->screen_name, current_tweet->author->user_id);
		current_tweet = current_tweet->next_tweet;
	}
	
	Tweet_free(current_tweet, 1);
	// sleep(3);
	// }
	
	return 0;
}