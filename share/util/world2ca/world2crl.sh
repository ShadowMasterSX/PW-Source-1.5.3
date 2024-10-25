if [ not $1 ]; then
	echo "usage: world2crl.sh world2_username"
	exit
fi

echo ""
echo "#######################    generate World2 CRL     #################################"
echo ""
echo "openssl ca -gencrl -crldays 5475 -revoke /etc/ssl/private/client/world2_$1.cer -keyfile /etc/ssl/private/world2_ca.key"

echo ""


