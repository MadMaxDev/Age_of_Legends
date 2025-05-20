#/bin/sh

. $HOME/sh.cfg

./glogrun_d.sh
if [ $is_global -eq 1 ] 
then
	./runGlobal.sh
fi

if [ $is_group -eq 1 ] 
then
	./runGroup.sh
fi
