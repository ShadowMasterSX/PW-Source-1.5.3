I USED DEBIAN ON WSL TO COMPILE

chmod 777 setrules.sh chmod 777 gfaction/operations/opgen.pl; chmod 777 rpcgen chmod 777 rpc/xmlcoder.pl

install libxml-dom-perl install libxml2-dev:i386 install libssl-dev:i386 install libpcre3-dev:i386 install libssl1.0-dev:i386

First time Build: make configure

Afterwards: make

To copy binaries to server folder

edit install target path as desired, defaults to /pwserver
run make install
