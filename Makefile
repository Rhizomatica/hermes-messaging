CC=gcc

#LIBRARIES=glib-2.0 gio-2.0
LIBRARIES=dbus-1
CFLAGS= -Wall -Wextra -std=gnu17 -g `pkg-config --cflags $(LIBRARIES)`
LDFLAGS=`pkg-config --libs $(LIBRARIES)` -lm

all: http_server https_server dec_message ofono_daemon

ofono_daemon: ofono_daemon.c send_email.c
	$(CC) $(CFLAGS) -o ofono_daemon ofono_daemon.c send_email.c $(LDFLAGS)

http_server: server.c process_sms.c process_sms.h send_email.c send_email.h
	$(CC) -Wall -Wno-unused-but-set-variable -O3 server.c process_sms.c send_email.c -o http_server

https_server: tls_server.c process_sms.c process_sms.h send_email.c send_email.h
	$(CC) -Wall -Wno-unused-but-set-variable -O3 tls_server.c process_sms.c send_email.c -o https_server -lssl -lcrypto

dec_message: dec_message.c
	$(CC) -Wall -Wno-unused-but-set-variable -O3 dec_message.c -o dec_message

install:
	install -D dec_message /usr/bin/dec_message
#	install -D https_server /usr/bin/https_server

.PHONY: clean
clean:
	rm -f http_server https_server dec_message ofono_daemon
