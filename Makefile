FILES += main.c
# FILES += twitter_api.c
FILES += http_connection.c
FILES += twitter_api.c
FILES += twitter_api_oauth.c
FILES += crypto/*.c

LIBRARIES += -lcurl
LIBRARIES += -ljson-c
# LIBRARIES += -lcrypto

EXECUTABLE = twitterc

all:
	gcc $(FILES) $(LIBRARIES) -o $(EXECUTABLE)

run:
	./$(EXECUTABLE)
