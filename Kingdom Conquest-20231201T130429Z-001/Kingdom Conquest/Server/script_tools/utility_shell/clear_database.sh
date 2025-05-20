#!/bin/sh

. $HOME/sh.cfg

create_ims=1
create_gm=1

WORK_DIR=~/src/

# web_cur
echo "CREATE DATABASE $group_db"
mysql -u$mysql_user -p$mysql_pass -e "DROP DATABASE $group_db"
mysql -u$mysql_user -p$mysql_pass -e "CREATE DATABASE $group_db"
mysql -u$mysql_user -p$mysql_pass -D$group_db < $WORK_DIR""PRJ/SQL/web/table.sql 
mysql -u$mysql_user -p$mysql_pass -D$group_db < $WORK_DIR""PRJ/SQL/web/table-gedamis.sql 
mysql -u$mysql_user -p$mysql_pass -D$group_db < $WORK_DIR""PRJ/SQL/web/sp.sql 
mysql -u$mysql_user -p$mysql_pass -D$group_db < $WORK_DIR""PRJ/SQL/web/sp-gedamis.sql 
mysql -u$mysql_user -p$mysql_pass -D$group_db < $WORK_DIR""PRJ/SQL/web/91pay.sql
mysql -u$mysql_user -p$mysql_pass -D$group_db < $WORK_DIR""PRJ/SQL/web/91pay.tbl.sql

# $ims_db
if [ $create_ims -eq "1" ]
then
	echo "CREATE DATABASE $ims_db"
	mysql -u$mysql_user -p$mysql_pass -e "DROP DATABASE $ims_db"
	mysql -u$mysql_user -p$mysql_pass -e "CREATE DATABASE $ims_db"
	mysql -u$mysql_user -p$mysql_pass -D$ims_db < $WORK_DIR""PRJ/SQL/web_ims/table.sql 
	mysql -u$mysql_user -p$mysql_pass -D$ims_db < $WORK_DIR""PRJ/SQL/web_ims/sp.sql 
fi

# gm
if [ $create_gm -eq "1" ]
then
	echo "CREATE DATABASE web_gm"
	mysql -u$mysql_user -p$mysql_pass -e "DROP DATABASE gm"
	mysql -u$mysql_user -p$mysql_pass -e "CREATE DATABASE gm"
	mysql -u$mysql_user -p$mysql_pass -Dgm < $WORK_DIR""PRJ/SQL/gm/gm.sql
fi

# 清理排行榜文件
echo "CLEAR rank_file"
rm $HOME"/"bin/XLP4Web/rank_file -rf
