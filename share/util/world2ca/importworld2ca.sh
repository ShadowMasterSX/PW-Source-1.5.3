
echo ""
echo "######################## import world2_ca  #############################"
echo ""

echo ""
echo "$JAVA_HOME/bin/keytool -delete -keystore $JAVA_HOME/jre/lib/security/cacerts -storepass changeit -alias world2ca"
$JAVA_HOME/bin/keytool -delete -keystore $JAVA_HOME/jre/lib/security/cacerts -storepass changeit -alias world2ca

echo ""
echo "$JAVA_HOME/bin/keytool -import -keystore $JAVA_HOME/jre/lib/security/cacerts -storepass changeit -alias world2ca -trustcacerts -file /etc/ssl/private/world2_ca.cer"
$JAVA_HOME/bin/keytool -import -keystore $JAVA_HOME/jre/lib/security/cacerts -storepass changeit -alias world2ca -trustcacerts -file /etc/ssl/private/world2_ca.cer


