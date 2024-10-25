
if [ $1 ]; then
	echo "generating key-pair for $1......"
else
	echo "usage: world2user.sh username"
	exit
fi

echo ""
echo "###################    generate world2_user keys     ###############################"
echo ""

if [ -f /etc/ssl/private/client/world2_$1.cer ]; then
	echo "file /etc/ssl/private/client/world2_$1.cer already exists."
else
	echo "openssl req -new -newkey rsa:1024 -nodes -subj /CN=$1/OU=gmuser/O=world2.com.cn/L=Haidian/ST=Beijing/C=CN -out /etc/ssl/private/client/world2_$1.req -keyout /etc/ssl/private/client/world2_$1.key"
	openssl req -new -newkey rsa:1024 -nodes -subj /CN=$1/OU=gmuser/O=world2.com.cn/L=Haidian/ST=Beijing/C=CN -out /etc/ssl/private/client/world2_$1.req -keyout /etc/ssl/private/client/world2_$1.key

	echo ""
	echo "openssl x509 -CA /etc/ssl/private/world2_ca.cer -days 5475 -CAkey /etc/ssl/private/world2_ca.key -CAserial /etc/ssl/private/world2_ca.srl -req -in /etc/ssl/private/client/world2_$1.req -out /etc/ssl/private/client/world2_$1.cer -extfile /etc/ssl/private/client/world2_$1.cnf -extensions myv3_ca"
	echo ""
	echo "[myv3_ca]" > /etc/ssl/private/client/world2_$1.cnf
	echo "subjectAltName=email:sunzhenyu@world2.cn" >> /etc/ssl/private/client/world2_$1.cnf
	openssl x509 -CA /etc/ssl/private/world2_ca.cer -days 5475 -CAkey /etc/ssl/private/world2_ca.key -CAserial /etc/ssl/private/world2_ca.srl -req -in /etc/ssl/private/client/world2_$1.req -out /etc/ssl/private/client/world2_$1.cer -extfile /etc/ssl/private/client/world2_$1.cnf -extensions myv3_ca
fi

echo ""
echo "openssl pkcs12 -export -clcerts -in /etc/ssl/private/client/world2_$1.cer -inkey /etc/ssl/private/client/world2_$1.key -out /etc/ssl/private/client/world2_$1.p12 -name 'World2 User Certificate'"
rm -f /etc/ssl/private/client/world2_$1.p12
openssl pkcs12 -export -clcerts -in /etc/ssl/private/client/world2_$1.cer -inkey /etc/ssl/private/client/world2_$1.key -out /etc/ssl/private/client/world2_$1.p12 -name "World2 User Certificate"

