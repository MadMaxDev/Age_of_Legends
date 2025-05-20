#!/bin/sh
cd ..
for dir in CAAFS4Chat GMS4Chat GZLS4Chat GMS4Game DBS4Game XCMNSVR
do
	ln -s ../Sln-mk/Prj-EXT-slackware.mk $dir/Prj-EXT.mk
done
