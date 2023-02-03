#!/bin/bash
# script to send SMS...
# Substitute the api_key and apt_secret by the provider api_key and api_secret


helpFunction()
{
    echo ""
    echo "Usage: $0 -f from_field -t destination_number -m message"
    echo -e "-f From field to go in the message (optional)"
    echo -e "-t Destination number (To) (mandatory)"
    echo -e "-m SMS Message (mandatory)"
    exit 1
}

while getopts "f:t:m:" opt
do
    case "$opt" in
        f ) from="$OPTARG" ;;
        t ) to="$OPTARG" ;;
        m ) message="$OPTARG" ;;
        ? ) helpFunction ;;
   esac
done

if [ -z "$to" ] || [ -z "$message" ]
then
   echo "Some of mandatory parameters are empty";
   helpFunction
fi

if [ -z "$from" ]
then
    from="15013812345"
fi

# only brazilian numbers allowed?
#if [ ${to:0:3} != "593" ]
#then
#    to="593${to}"
#fi

echo "from = $from"
echo "to = $to"
echo "message = $message"
exit

curl -X "POST" "https://rest.nexmo.com/sms/json" \
  -d "from=$from" \
  -d "to=$to" \
  -d "api_key=xx999999" \
  -d "api_secret=xxxxxxxxxxxxxx" \
  -d "text=$message"
