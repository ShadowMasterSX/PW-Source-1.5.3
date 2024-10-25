#!/bin/bash
echo ""
echo "======================== modify Rules.make ========================"
echo ""
EPWD=`pwd|sed -e 's/\//\\\\\//g'`;
cd cgame
sed -i -e "s/IOPATH=.*$/IOPATH=$EPWD\/iolib/g" -e "s/BASEPATH=.*$/BASEPATH=$EPWD\/cgame/g" Rules.make
