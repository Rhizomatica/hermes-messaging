CC=gcc

all: http_server https_server dec_message

http_server: server.c process_sms.c process_sms.h send_email.c send_email.h
	$(CC) -Wall -O3 server.c process_sms.c send_email.c -o http_server

https_server: tls_server.c process_sms.c process_sms.h send_email.c send_email.h
	$(CC) -Wall -O3 tls_server.c process_sms.c send_email.c -o https_server -lssl -lcrypto

dec_message: dec_message.c
	$(CC) -Wall -O3 dec_message.c -o dec_message

.PHONY: clean
clean:
	rm -f http_server https_server dec_message
