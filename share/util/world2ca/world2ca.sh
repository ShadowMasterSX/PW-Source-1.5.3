
echo ""
echo "#######################    generate World2 CA keys   #################################"
echo ""
echo "rm -f /etc/ssl/private/world2_*"
rm -f /etc/ssl/private/world2_*
echo "rm -f /etc/ssl/private/client/world2_*"
rm -f /etc/ssl/private/client/world2_*

echo "mkdir -p -m go= /etc/ssl/private"
mkdir -p -m go= /etc/ssl/private
echo "mkdir -p -m go= /etc/ssl/private/client"
mkdir -p -m go= /etc/ssl/private/client

echo ""
echo "openssl req -new -newkey rsa:1024 -nodes -subj /CN=ca.world2.com.cn/OU=ca/O=world2.com.cn/L=Haidian/ST=Beijing/C=CN -out /etc/ssl/private/world2_ca.csr -keyout /etc/ssl/private/world2_ca.key"
openssl req -new -newkey rsa:1024 -nodes -subj /CN=ca.world2.com.cn/OU=ca/O=world2.com.cn/L=Haidian/ST=Beijing/C=CN -out /etc/ssl/private/world2_ca.csr -keyout /etc/ssl/private/world2_ca.key
echo ""
echo "[myv3_ca]" > /etc/ssl/private/world2_ca.cnf
echo "subjectAltName=email:sunzhenyu@world2.cn" >> /etc/ssl/private/world2_ca.cnf
echo "crlDistributionPoints=URI:http://ca.world2.com.cn/ca/ca.crl" >> /etc/ssl/private/world2_ca.cnf
echo "openssl x509 -trustout -signkey /etc/ssl/private/world2_ca.key -days 5475 -req -in /etc/ssl/private/world2_ca.csr -out /etc/ssl/private/world2_ca.cer -extfile /etc/ssl/private/world2_ca.cnf -extensions myv3_ca"
openssl x509 -signkey /etc/ssl/private/world2_ca.key -days 5475 -req -in /etc/ssl/private/world2_ca.csr -out /etc/ssl/private/world2_ca.cer -extfile /etc/ssl/private/world2_ca.cnf -extensions myv3_ca

echo ""
echo "openssl pkcs12 -export -clcerts -in /etc/ssl/private/world2_ca.cer -inkey /etc/ssl/private/world2_ca.key -out /etc/ssl/private/world2_ca.p12 -name 'World2.com.cn CA'"
openssl pkcs12 -export -clcerts -in /etc/ssl/private/world2_ca.cer -inkey /etc/ssl/private/world2_ca.key -out /etc/ssl/private/world2_ca.p12 -name "World2.com.cn CA"

echo ""
echo "echo 02 > /etc/ssl/private/world2_ca.srl"
echo "02" > /etc/ssl/private/world2_ca.srl

