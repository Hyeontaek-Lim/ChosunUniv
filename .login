# @(#)login 1.7 89/09/05 SMI

if ( ! ${?DT} ) then
stty -istrip
setenv TERM `tset -Q -`
endif

if (($TERM == "network") || ($TERM == "dialup"))  then
setenv  TERM `tset -Q - -e^H vt100`
endif
if ($TERM == "vt100")  then
setenv  TERM `tset -Q - -e^H vt100`
endif

#Mail Notificatios for a Terminals.
biff y

#
# if possible, start the windows system.  Give user a chance to bail out
#
if ( `tty` == "/dev/console" ) then

	if ( $TERM == "sun" || $TERM == "AT386" ) then

		if ( ${?OPENWINHOME} == 0 ) then        
			setenv OPENWINHOME /usr/openwin
		endif        

		echo ""
		echo -n "Starting OpenWindows in 5 seconds (type Control-C to interrupt)"
		sleep 5
		echo ""
		$OPENWINHOME/bin/openwin
		clear           # get rid of annoying cursor rectangle
		logout          # logout after leaving windows system

	endif

endif
