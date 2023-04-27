# HERMES Messaging System

The concept of this project is to implement bridges between email
users and messaging services, like SMS or Whatsapp.

Simple SMS (Nexmo provider supported for now) <-> e-mail bridge is already provided.

Both binaries are created with make: http_server and https_server, which
have roughly the same functionality, and the following parameters:

 $ ./https_server <port_nr>

Add cert.pem and key.pem in the local directory for the https_server to work.

The received SMS (from gateway to remote users) in the format
```
<email1> [<email2> <email3>] message text
```
will be sent over email.

The sending part (from remote station) send email with the format:
```
<fone_number> message text
```
Some sample scripts to send SMS are in send_sms folder.

# Author

rafael@rhizomatica.org

