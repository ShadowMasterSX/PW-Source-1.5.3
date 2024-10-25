
if [ not $2 ]; then
	echo "usage: test.sh world2_host world2_clientusername"
	exit
fi

echo ""
echo "############################    test      #####################################"
echo ""
echo "openssl s_client -connect $1:8443 -cert /etc/ssl/private/client/world2_$2.cer -key /etc/ssl/private/client/world2_$2.key -tls1"
openssl s_client -connect $1:443 -cert /etc/ssl/private/client/world2_$2.cer -key /etc/ssl/private/client/world2_$2.key -tls1

