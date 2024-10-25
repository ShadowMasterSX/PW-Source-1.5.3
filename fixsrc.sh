#!/bin/bash

find . -name "*.h" -exec dos2unix {} \;
find . -name "*.hpp" -exec dos2unix {} \;
find . -name "*.hxx" -exec dos2unix {} \;

find . -name "*.cpp" -exec dos2unix {} \;
find . -name "*.cxx" -exec dos2unix {} \;
find . -name "Makefile" -exec dos2unix {} \;

chmod -R 0755 . ; chmod -R 0755 *
chown -R root:root ; chown -R root:root *
