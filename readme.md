# Test OS Debian 12

chmod 777 -R setrules.sh chmod 777 gfaction/operations/opgen.pl; chmod 777 rpcgen chmod 777 rpc/xmlcoder.pl

apt-get install -y libxml-dom-perl libxml2-dev libssl-dev libpcre3-dev libssl1.0-dev

First time Build: make configure

Afterwards: make

To copy binaries to server folder

edit install target path as desired, defaults to /pwserver
run make install
