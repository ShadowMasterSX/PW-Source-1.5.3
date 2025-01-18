
# Perfect World Source Code - Server 1.5.3

### How to Use on Debian 12 | Ubuntu 24.04




## Installation

### root acess is required

#### Installing the required software

```bash
apt-get install -y curl wget ipset git net-tools tzdata ntpdate mariadb-server mariadb-client
```
```bash
apt-get install -y strace mc screen htop default-jdk mono-complete exim4 p7zip-full libpcap-dev
```
```bash
apt-get install -y make gcc g++ libssl-dev libcrypto++-dev libpcre3 libpcre3-dev libtesseract-dev 
```
```bash
apt-get install -y libx11-dev gcc-multilib libc6-dev build-essential libtemplate-plugin-xml-perl
```
```bash
apt-get install -y libdb++-dev libdb-dev libdb5.3 libdb5.3++ libdb5.3++-dev libdb5.3-dbg libdb5.3-dev libxml2-dev
```
#### Installing the optional software
```bash
apt-get install -y apache2 php php-cli php-fpm php-json php-pdo php-zip php-gd php-mbstring php-curl php-xml php-pear php-bcmath php-cgi php-mysqli php-common php-phpseclib php-mysql
```
```bash
apt-get install -y phpmyadmin
```

# How to Build (script)

```bash
cd /root/; nano build.sh

````
### inside this code
```bash
#!/bin/bash
set -e  # Stop the script if any command fails
set -o pipefail  # Ensure pipes return an error if any command fails
trace() {
    local msg="$1"
    echo "[TRACE] $(date '+%Y-%m-%d %H:%M:%S') - $msg"
}
trace "Starting script execution."
GS=$(echo *game)
NET=$(echo *net)
SKILL=$(echo *skill)
trace "Detected directories: GS=$GS, NET=$NET, SKILL=$SKILL"
trace "=========================== setup $NET ==========================="
cd "$NET"
for dir in common io perf mk storage rpc rpcgen lua; do
    trace "Removing old $dir and creating symlink."
    rm -f "$dir"
    ln -s ~/share/$dir/ "$dir"
    trace "Symlink created for $dir."
done
cd ..
trace "=========================== setup iolib ==========================="
if [ ! -d iolib ]; then
    trace "Creating iolib directory."
    mkdir iolib
fi
cd iolib
if [ ! -d inc ]; then
    trace "Creating inc directory inside iolib."
    mkdir inc
fi
cd inc
trace "Removing old includes and creating new symlinks."
rm -f *
includes=(
    "$NET/gamed/auctionsyslib.h"
    "$NET/gamed/sysauctionlib.h"
    "$NET/gdbclient/db_if.h"
    "$NET/gamed/factionlib.h"
    "$NET/common/glog.h"
    "$NET/gamed/gsp_if.h"
    "$NET/gamed/mailsyslib.h"
    "$NET/gamed/privilege.hxx"
    "$NET/gamed/sellpointlib.h"
    "$NET/gamed/stocklib.h"
    "$NET/gamed/webtradesyslib.h"
    "$NET/gamed/kingelectionsyslib.h"
    "$NET/gamed/pshopsyslib.h"
    "$NET/io/luabase.h"
)
for inc in "${includes[@]}"; do
    ln -s "../../$inc" .
    trace "Symlink created for $inc."
done
cd ..
rm -f lib*
trace "Creating library symlinks."
ln -s ../$NET/io/libgsio.a
ln -s ../$NET/gdbclient/libdbCli.a
ln -s ../$NET/gamed/libgsPro2.a
ln -s ../$NET/logclient/liblogCli.a
ln -s ../$SKILL/libskill.a
cd ..
trace "======================== modify Rules.make ========================"
EPWD=$(pwd | sed -e 's/\//\\\//g')
trace "Current working directory: $EPWD"
cd "$GS"
sed -i -e "s/IOPATH=.*$/IOPATH=$EPWD\/iolib/g" -e "s/BASEPATH=.*$/BASEPATH=$EPWD\/$GS/g" Rules.make
cd ..
trace "====================== softlink libskill.so ======================="
cd "$GS/gs"
rm -f libskill.so
ln -s "../../$SKILL/libskill.so"
trace "Symlink created for libskill.so."
cd ../../
trace "Script setup completed successfully."
# Function Definitions with Trace Logs
buildrpcgen() {
    trace "========================== $NET rpcgen ============================"
    cd "$NET"
    ./rpcgen rpcalls.xml
    trace "RPC generation completed."
    cd ..
}
buildgslib() {
    trace "======================= build liblogCli.a ========================="
    cd "$NET/logclient"
    make clean
    make -f Makefile.gs -j$(nproc)
    trace "liblogCli.a built successfully."
    cd ..
    trace "======================== build libgsPro2.a ========================="
    cd gamed
    make clean
    make lib -j$(nproc)
    trace "libgsPro2.a built successfully."
    cd ..
    trace "======================== build libdbCli.a =========================="
    cd gdbclient
    make clean
    make lib -j$(nproc)
    trace "libdbCli.a built successfully."
    cd ..
}
# Add similar tracing to all functions as necessary
if [ $# -gt 0 ]; then
    case "$1" in
        deliver)
            trace "Starting delivery rebuild."
            buildrpcgen
            builddeliver
            ;;
        gs)
            trace "Starting GS rebuild."
            buildrpcgen
            buildshared
            buildgsio
            buildgslib
            buildskill
            buildgame
            install
            ;;
        all)
            trace "Starting full rebuild."
            rebuildall
            ;;
        install)
            trace "Starting installation."
            install
            ;;
        *)
            echo "Invalid option."
            ;;
    esac
fi
trace "Script execution finished."
```

```bash 
chmod 777 build.sh
```

```bash
./build.sh
```
