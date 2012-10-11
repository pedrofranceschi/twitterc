FILES += main.c
# FILES += twitter_api.c
FILES += http_connection.c
FILES += twitter_api.c
FILES += debug_new.cpp

LIBRARIES += -lcurl
LIBRARIES += -ljson

EXECUTABLE = twitterc

all:
	gcc $(FILES) $(LIBRARIES) -o $(EXECUTABLE)

run:
	./$(EXECUTABLE)