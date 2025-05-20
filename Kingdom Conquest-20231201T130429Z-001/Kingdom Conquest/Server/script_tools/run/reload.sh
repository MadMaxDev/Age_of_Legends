#!/bin/sh

alias mkdbg='make -f $PMKBASE/Makefile_debug.mk'

. $HOME/sh.cfg

bin_stop() {
	cd ~/bin
	./stop.sh
	cd ~
}
bin_stop_dbs() {
	killall XDBS4Web_d
}
bin_start() {
	cd ~/bin
	./run.sh
	cd ~
}
bin_start_dbs() {
	cd ~/bin/
	./XDBS4Web/XDBS4Web_d dbs4web_cfg.txt
	cd ~
}
src_svn() {
	cd ~/src
	svn update
	cd ~
}
src_compile() {
	cd ~/src
	mkdbg dirs
	mkdbg dep
	mkdbg all
#	mkdbg install
	cd ~
}
bin_install() {
	cd ~/src
	mkdbg install
	cd ~
}
src_compile_dbs() {
	cd ~/src
	mkdbg dirs
	mkdbg dep
	mkdbg all
	cd ~/src/PRJ/TTY/DBS4Web
	mkdbg install
	cd ~
}
src_db() {
#    $mysql -u$USER -p$PASS -e "drop database $DBIMS"
read -p "创建全局数据库 $DBIMS (y/N)" answer
    [ -z "$answer" ] && answer="n"
case "$answer" in
[yY])
	$mysql -u$USER -p$PASS -e "create database $DBIMS"
	$mysql -u$USER -p$PASS -D$DBIMS < ./src/PRJ/SQL/web_ims/table.sql
	$mysql -u$USER -p$PASS -D$DBIMS < ./src/PRJ/SQL/web_ims/sp.sql
    ;;
*)
    echo "取消操作!"
    ;;
esac

#	$mysql -u$USER -p$PASS -e "drop database $DB"
read -p "创建游戏数据库 $DB (y/N)" answer
    [ -z "$answer" ] && answer="n"
case "$answer" in
[yY])
	$mysql -u$USER -p$PASS -e "create database $DB"
	$mysql -u$USER -p$PASS -D$DB < ./src/PRJ/SQL/web/table.sql
	$mysql -u$USER -p$PASS -D$DB < ./src/PRJ/SQL/web/sp.sql
	$mysql -u$USER -p$PASS -D$DB < ./src/PRJ/SQL/web/table-gedamis.sql
	$mysql -u$USER -p$PASS -D$DB < ./src/PRJ/SQL/web/sp-gedamis.sql
	$mysql -u$USER -p$PASS -D$DB < ./src/PRJ/SQL/web/91pay.tbl.sql
	$mysql -u$USER -p$PASS -D$DB < ./src/PRJ/SQL/web/91pay.sql
    ;;
*)
    echo "取消操作!"
    ;;
esac

}
src_db_dbs() {
	$mysql -u$USER -p$PASS -D$DBIMS < ./src/PRJ/SQL/web_ims/sp.sql
	
	$mysql -u$USER -p$PASS -D$DB < ./src/PRJ/SQL/web/sp.sql
	$mysql -u$USER -p$PASS -D$DB < ./src/PRJ/SQL/web/sp-gedamis.sql
}
src_mb() {
    cd ~/mb
    svn update
#    if [ -d ~/bin/excel_tables ]; then
#        echo 'excel_tables exists';
#    else
#        mkdir ~/bin/excel_tables
#    fi
#	cp -fvr ./* ~/bin/excel_tables/ >/dev/null
#	./bin/ExcelToSQL/ExcelToSQL_d >/dev/null
#	$mysql -u$USER -p$PASS -D$DB --default-character-set=utf8 < ./bin/ExcelToSQL/excel_table.sql
# 在XExcelDeploy_d中已经进行了mb的拷贝，所以就不用上面的拷贝了，否则重复拷贝了一次
# 需要的只是在XExcelDeploy目录中配置一下源目录为$(HOME)/mb即可
	~/bin/XExcelDeploy/XExcelDeploy_d
}

case $1 in
		all)
			# 更新代码
			src_svn
			# 编译
			src_compile
			# 程序停止
			bin_stop
            # 安装程序
            bin_install
			# 重建 table
		#	src_db
			# 导入 sp
			src_db_dbs
			# 导入 mb
			src_mb
			# 启动程序
			bin_start
# 更新下载资源
#~/cpupdate.sh
			;;
		dbs_sp)
			src_svn
			src_db_dbs
		#	src_mb
			;;
		dbs)
			bin_stop_dbs
			src_svn
			src_compile_dbs
		#	src_db
			src_db_dbs
		#	src_mb
			bin_start_dbs
			;;
        createdb)
            src_db
            ;;
		mb)
			src_mb
			;;
		*)
			echo "usage: $0 all|dbs|dbs_sp|mb|createdb"
			exit 1
			;;
esac



