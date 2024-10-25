
if [ $1 ]; then
	echo "generating key-pair for $1......"
else
	echo "usage: world2java.sh hostname"
	exit
fi

echo ""
echo "######################## generate java keystore #############################"
echo ""

if [ -f /etc/ssl/private/client/world2_java_$1.cer ]; then
	echo "file /etc/ssl/private/client/world2_java_$1.cer already exists."
else
	echo ""
	echo "rm -f /etc/ssl/private/client/world2_java_$1.keystore"
	rm -f /etc/ssl/private/client/world2_java_$1.keystore

	echo ""
	echo "keytool -genkey -keystore /etc/ssl/private/client/world2_java_$1.keystore -storepass changeit -keypass changeit -alias java -keyalg RSA -keysize 1024 -validity 5475 -dname 'CN=$1,OU=gmhost,O=world2.com.cn,L=Haidian,ST=Beijing,C=CN'"
	keytool -genkey -keystore /etc/ssl/private/client/world2_java_$1.keystore -storepass changeit -keypass changeit -alias java -keyalg RSA -keysize 1024 -validity 5475 -dname "CN=$1,OU=gmhost,O=world2.com.cn,L=Haidian,ST=Beijing,C=CN"

	echo ""
	echo "keytool -certreq -keystore /etc/ssl/private/client/world2_java_$1.keystore -storepass changeit -keyalg RSA -alias java -file /etc/ssl/private/client/world2_java_$1.req"
	keytool -certreq -keystore /etc/ssl/private/client/world2_java_$1.keystore -storepass changeit -keyalg RSA -alias java -file /etc/ssl/private/client/world2_java_$1.req

	echo ""
	echo "openssl x509 -CA /etc/ssl/private/world2_ca.cer -days 5475 -CAkey /etc/ssl/private/world2_ca.key -CAserial /etc/ssl/private/world2_ca.srl -req -in /etc/ssl/private/client/world2_java_$1.req -out /etc/ssl/private/client/world2_java_$1.cer -extfile /etc/ssl/private/client/world2_java_$1.cnf -extensions myv3_ca"
	echo "[myv3_ca]" > /etc/ssl/private/client/world2_java_$1.cnf
	echo "subjectAltName=email:sunzhenyu@world2.cn" >> /etc/ssl/private/client/world2_java_$1.cnf
	if [ $2 ]; then
		echo ""
	else
		echo "subjectAltName=IP:$1" >> /etc/ssl/private/client/world2_java_$1.cnf
	fi
	openssl x509 -CA /etc/ssl/private/world2_ca.cer -days 5475 -CAkey /etc/ssl/private/world2_ca.key -CAserial /etc/ssl/private/world2_ca.srl -req -in /etc/ssl/private/client/world2_java_$1.req -out /etc/ssl/private/client/world2_java_$1.cer -extfile /etc/ssl/private/client/world2_java_$1.cnf -extensions myv3_ca
fi

echo ""
echo "keytool -import -keystore /etc/ssl/private/client/world2_java_$1.keystore -storepass changeit -alias java -trustcacerts -file /etc/ssl/private/client/world2_java_$1.cer"
keytool -import -keystore /etc/ssl/private/client/world2_java_$1.keystore -storepass changeit -alias java -trustcacerts -file /etc/ssl/private/client/world2_java_$1.cer

