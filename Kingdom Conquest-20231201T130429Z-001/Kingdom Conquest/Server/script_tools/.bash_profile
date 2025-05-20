# .bash_profile

# Get the aliases and functions
if [ -f ~/.bashrc ]; then
	. ~/.bashrc
fi

# Sample .bash_profile for CentOS Linux
# rewritten by Christian Steinruecken <cstein@suse.de>
#
# This file is read each time a login shell is started.
# All other interactive shells will only read .bashrc; this is particularly
# important for language settings, see below.

test -z "$PROFILEREAD" && . /etc/profile

# Most applications support several languages for their output.
# To make use of this feature, simply uncomment one of the lines below or
# add your own one (see /usr/share/locale/locale.alias for more codes)
#
#export LANG=de_DE.UTF-8        # uncomment this line for German output
#export LANG=fr_FR.UTF-8        # uncomment this line for French output
#export LANG=es_ES.UTF-8        # uncomment this line for Spanish output


# Some people don't like fortune. If you uncomment the following lines,
# you will have a fortune each time you log in ;-)

#if [ -x /usr/bin/fortune ] ; then
#    echo
#    /usr/bin/fortune
#    echo
#fi

# 
ulimit -H -S -c unlimited
PATH=$PATH:$HOME/bin
TERM=xterm-color
SVN_EDITOR=vi
PMKBASE=~/src/whmkbase
alias mkdbg='make -f $PMKBASE/Makefile_debug.mk'
alias mkdbgp='make -f $PMKBASE/Makefile_gpdebug.mk'
alias mkrls='make -f $PMKBASE/Makefile_release.mk'
alias mkrlsgp='make -f $PMKBASE/Makefile_gprelease.mk'

export PATH LANG TERM SVN_EDITOR PMKBASE
unset USERNAME
stty erase ^H
