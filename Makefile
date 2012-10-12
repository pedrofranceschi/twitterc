FILES += main.c
# FILES += twitter_api.c
FILES += http_connection.c
FILES += twitter_api.c
FILES += twitter_api_oauth.c

LIBRARIES += -lcurl
LIBRARIES += -ljson

EXECUTABLE = twitterc

all:
	gcc $(FILES) $(LIBRARIES) -o $(EXECUTABLE)

run:
	./$(EXECUTABLE)