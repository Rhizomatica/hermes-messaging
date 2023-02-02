#!/bin/bash
#
curl -X POST https://messages-sandbox.nexmo.com/v1/messages \
-u 'api_key:api_secret' \
-H 'Content-Type: application/json' \
-H 'Accept: application/json' \
-d '{
    "from": "14157386102",
    "to": "5511998728111",
    "message_type": "text",
    "text": "Testando zap zap 1 2 3",
    "channel": "whatsapp"
  }'
