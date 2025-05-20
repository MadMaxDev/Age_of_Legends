#!/bin/sh

exists_pattern_files(){  
    [ -e "$1" ]  
} 

read -p "如果~/bin目录中原来存在配置文件将被覆盖(y/N)" answer
[ -z "$answer" ] && answer="n"
case "$answer" in
[yY1])
echo "开始拷贝...."
    for dir in glogger XDBS4Web XCAAFS4Web XCLS4Web XLP4Web XGS4Web XGDB4Web XExcelDeploy XBDClient4Web XClient4Web XCombatSvr
    do
        dstdir=~/bin/$dir
        mkdir -p $dstdir
        cp -v $dir/*.txt $dstdir/
#        exists_pattern_files("$dir/*.txt") && cp -v $dir/*.txt $dstdir/
#        [ -e $dir/*.key ] && cp -v $dir/*.key $dstdir/
    done
    cp *.txt ~/bin/
    cp *.sh ~/bin/
    chmod +x ~/bin/*.sh
;;

*)
echo "取消操作!"
;;

esac


