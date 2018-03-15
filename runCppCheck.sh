#!/bin/sh

cppcheck --xml --xml-version=2 --enable=all -Isrc/app src/app 2> cppcheck-report.xml

