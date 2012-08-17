#!/bin/bash
# Script Name: devmon    http://igurublog.wordpress.com/downloads/script-devmon/ 
# Requires: udisks bash>=4
# Recommended: consolekit zenity
# License: GNU GENERAL PUBLIC LICENSE Version 3 http://www.gnu.org/licenses/gpl-3.0.txt
# Thanks to Bernard Baeyens (berbae) for code from udisksvm script
#    https://bbs.archlinux.org/viewtopic.php?id=112397

#=========================================================================

defaultmountoptions="noexec,nosuid,noatime"

#=========================================================================

help()
{
	cat << EOF
devmon version 1.0.5
Automounts and unmounts optical and removable drives using udisks
Requires:    udisks bash>=4        Recommended: consolekit zenity
Usage: devmon [AUTOMOUNT-OPTIONS]  # Run as daemon to automount
       devmon [MOUNT-OPTIONS]      # Or run as client to manually un/mount
AUTOMOUNT-OPTIONS:  (these can be used only in daemon mode)
--exec-on-device DEVICE "COMMAND"  Execute COMMAND after mounting DEVICE
--exec-on-label "LABEL" "COMMAND"  Execute COMMAND after mounting LABEL
--exec-on-video "COMMAND"          Execute COMMAND after video DVD mount
--exec-on-audio "COMMAND"          Execute COMMAND after audio CD insertion
--exec-on-disc  "COMMAND"          Execute COMMAND after data CD/DVD mount
--exec-on-drive "COMMAND"          Execute COMMAND after drive mount
--exec-on-unmount "COMMAND"        Execute COMMAND after unmount
--exec-on-remove  "COMMAND"        Execute COMMAND after drive removal
  Where the following in COMMAND will be replaced with:
     %d    mount point directory (eg /media/cd)
     %f    device name (eg /dev/sdd1)
     %l    label of mounted volume
  Multiple --exec-on-XXX options may be used to execute multiple commands.
  Other exec-on-XXX commands are ignored if exec-on-device or -label executed.
--mount-options "OPTIONS"          Default: $defaultmountoptions
--info-on-mount                    Show mounted drive info in a zenity dialog
--no-mount                         Don't mount anything, just exec (disables
                                   --exec-on-video)
--no-unmount                       Don't unmount all removable drives on exit

MOUNT-OPTIONS:  (these can be used only in client mode)
--unmount-removable | -r      Sync and unmount all removable drives and show
                              pop-up dialog (zenity installation required)
--unmount-recent | -c         Unmount most recently mounted removable drive
--unmount-optical | -o        Unmount all optical drives (error pop-up only)
--unmount-all | -u            Same as --unmount-removable --unmount-optical
--unmount DIR|DEVICE          Unmount DEVICE or mount point DIR
--eject DIR|DEVICE            Unmount and eject DEVICE or mount point DIR
--mount-all | -a              Mount all removable and optical drives
--mount DEVICE                Mount DEVICE
--mount-options|--mount-fstype|--unmount-options|--eject-options "OPTIONS"
                              These options will be passed to udisks

UNIVERSAL OPTIONS:  (these can be used in both daemon and client modes)
--ignore-device DEVICE        Ignore DEVICE (eg /dev/sdd1)
--ignore-label "LABEL"        Ignore volume with LABEL
--sync | -s                   Add sync mount option for ext2-4 ntfs ufs, or
                              flush for fat & vfat (slower writing but safer)
--internal                    Also attempt to un/mount internal system drives
                              (this is mostly a fix for esata issues)
--no-gui | -g                 Do not show zenity pop-up dialogs
Instructions and updates:
  http://igurublog.wordpress.com/downloads/script-devmon/
EOF
	exit
}

test2()
{
	if [ "${2:0:1}" = "-" ] || [ "$2" = "" ]; then
		echo "devmon: Option $1 requires an argument" 1>&2
		exit 1
	fi	
}

test3()
{
	if [ "${2:0:1}" = "-" ] || [ "$2" = "" ] || \
	   [ "${3:0:1}" = "-" ] || [ "$3" = "" ]; then
		echo "devmon: Option $1 requires two arguments" 1>&2
		exit 1
	fi	
}

unknown()
{
	echo "devmon: Unknown option $1" 1>&2
	echo "        For help use: devmon --help" 1>&2
	exit 1
}

# parse command line
execoix=0
execomx=0
execovx=0
execoax=0
execodx=0
execolx=0
execoux=0
execorx=0
umntx=0
mntx=0
ejx=0
igdevx=0
iglabx=0
while [ "$1" != "" ]; do
	if [ "${1:0:1}" = "-" ]; then
		case "$1" in
			--help )
				help
				exit
				;;
			# don't use eval on these to preserve command strings
			--exec-on-drive )
				test2 "$1" "$2"
				execoi[$execoix]="$2"
				(( execoix++ ))
				shift
				;;
			--exec-on-disc )
				test2 "$1" "$2"
				execom[$execomx]="$2"
				(( execomx++ ))
				shift
				;;
			--exec-on-video )
				test2 "$1" "$2"
				execov[$execovx]="$2"
				(( execovx++ ))
				shift
				;;
			--exec-on-audio )
				test2 "$1" "$2"
				execoa[$execoax]="$2"
				(( execoax++ ))
				shift
				;;
			--exec-on-device )
				test3 "$1" "$2" "$3"
				execod1[$execodx]="$2"
				execod2[$execodx]="$3"
				(( execodx++ ))
				shift 2
				;;
			--exec-on-label )
				test3 "$1" "$2" "$3"
				execol1[$execolx]="$2"
				execol2[$execolx]="$3"
				(( execolx++ ))
				shift 2
				;;
			--exec-on-unmount )
				test2 "$1" "$2"
				execou[$execoux]="$2"
				(( execoux++ ))
				shift
				;;
			--exec-on-remove )
				test2 "$1" "$2"
				execor[$execorx]="$2"
				(( execorx++ ))
				shift
				;;
			--info-on-mount )
				infomount=1
				;;
			--no-mount )
				nomount=1
				;;
			--sync )
				syncopt=1
				;;
			--unmount-on-exit )
			    # leave for usage compat with versions prior to 1.0.1
				;;
			--no-unmount )
				nounmount=1
				;;
			--unmount-all )
				unmountrem=1
				unmountoptical=1
				;;
			--unmount-removable )
				unmountrem=1
				;;
			--unmount-optical )
				unmountoptical=1
				;;
			--unmount-recent )
				unmountrecent=1
				;;
			--unmount )
				test2 "$1" "$2"
				umnt[$umntx]="$2"
				(( umntx++ ))
				shift
				;;
			--mount-all )
				mountall=1
				;;
			--mount )
				test2 "$1" "$2"
				mnt[$mntx]="$2"
				(( mntx++ ))
				shift
				;;
			--eject )
				test2 "$1" "$2"
				ej[$ejx]="$2"
				(( ejx++ ))
				shift
				;;
			--mount-options )
				test2 "$1" "$2"
				mountoptions="$2"
				shift
				;;
			--mount-fstype )
				test2 "$1" "$2"
				mountfstype="$2"
				shift
				;;
			--unmount-options )
				test2 "$1" "$2"
				unmountoptions="$2"
				shift
				;;
			--eject-options )
				test2 "$1" "$2"
				ejectoptions="$2"
				shift
				;;			
			--internal )
				internal=1
				;;
			--nogui | --no-gui )
				nogui=1
				;;
			--ignore-device )
				test2 "$1" "$2"
				igdev[$igdevx]="$2"
				(( igdevx++ ))
				shift
				;;				
			--ignore-label )
				test2 "$1" "$2"
				iglab[$iglabx]="$2"
				(( iglabx++ ))
				shift
				;;
			--* )
				unknown "$1";;
			-* )
				o="${1:1}"
				while [ "$o" != "" ]; do
					case "${o:0:1}" in
						r )
							unmountrem=1;;
						o )
							unmountoptical=1;;
						u )
							unmountrem=1
							unmountoptical=1
							;;
						a )
							mountall=1;;
						s )
							syncopt=1;;
						c )
							unmountrecent=1;;
						g )
							nogui=1;;
						h )
							help
							exit
							;;
						* )
							unknown "-${o:0:1}";;
					esac
					o="${o:1}"
				done
				;;
		esac
	else
		unknown "$1"
	fi
	shift
done
(( mountmode = umntx + mntx + ejx + unmountrem + unmountoptical + mountall + unmountrecent ))

# Warnings
if [ "$(whoami)" = "root" ]; then
	echo "WARNING: running devmon as root is usually not required or recommended" 1>&2
fi

if (( execoix + execomx + execovx + execoax + execodx + execolx + execoux + execorx \
	  + nounmount + infomount != 0 )) && (( mountmode != 0 )); then
	echo "WARNING: devmon automount options ignored in mount mode" 1>&2
fi

driveinfo()    #$1=dev    #Optional $2=quiet 
{
	unset systeminternal usage ismounted presentationnopolicy hasmedia \
			opticaldisc numaudiotracks type partition media blank label
	uinfos=`udisks --show-info $1 2> /dev/null`
	label=`echo "$uinfos" | grep -m 1 "^  label:" | sed 's/ *label: *\(.*\)/\1/'`
	listinfos=`echo "$uinfos" | grep \
				-e "^  system internal:" \
				-e "^  usage:" \
				-e "^  type:" \
				-e "^  is mounted:" \
				-e "^  presentation nopolicy:" \
				-e "^  has media" \
				-e "^  optical disc:" \
				-e "  blank:" \
				-e "  num audio tracks:" \
				-e "^  partition:" \
				-e "  media:"`
	# The change for type= is to take only its first value in listinfos
	listinfos=$(echo "$listinfos" | sed 's/ //g
				s/:/=/
				s/opticaldisc=/&1/
				s/type=\(.*\)/type=${type:-\1}/
				s/[()]//g
				s/partition=/&1/')
	eval "$listinfos"
	if (( internal == 1 )); then
		systeminternal="ignored"
	fi
	# Take only the first character
	hasmedia=${hasmedia:0:1}
	# If "partition:" not find in listinfos, should mean it is not a partition
	partition=${partition:-0}
	nopolicy="$presentationnopolicy"
	if (( mountmode == 0 )) && [ "$systeminternal" != "1" ] && [ "$2" != "quiet" ]; then
		if [ "$usage" = "filesystem" ] || [ "$opticaldisc" = "1" ]; then
			echo "device: [$1]"
			echo "    systeminternal: [$systeminternal]"
			echo "    usage:          [$usage]"
			echo "    type:           [$type]"
			echo "    label:          [$label]"
			echo "    ismounted:      [$ismounted]"
			echo "    nopolicy:       [$nopolicy]"
			echo "    hasmedia:       [$hasmedia]"
			echo "    opticaldisc:    [$opticaldisc]"
			echo "    numaudiotracks: [$numaudiotracks]"
			echo "    blank:          [$blank]"
			echo "    media:          [$media]"
			echo "    partition:      [$partition]"
		fi
	fi
}

ignoredevice()
{
	idx=0
	while (( idx < igdevx )); do
		if [ "$1" = "${igdev[$idx]}" ]; then
			echo "devmon: ignored device $1"
			return 0
		fi
		(( idx++ ))
	done
	return 1
}

ignorelabel()
{
	ilx=0
	while (( ilx < iglabx )); do
		if [ "$1" = "${iglab[$ilx]}" ]; then
			echo "devmon: ignored label $1"
			return 0
		fi
		(( ilx++ ))
	done
	return 1
}

execcommands()  # $exectype "${exec[@]}"
{
	exectype="$1"
	shift
	while [ "$1" != "" ]; do
		usercmd="$1"
		usercmd="${usercmd//%f/$dv}"
		usercmd="${usercmd//%l/'$lb'}"
		usercmd="${usercmd//%d/'$point'}"
		if [ "$usercmd" != "" ]; then
			echo "devmon: [$exectype] eval $usercmd &"
			eval $usercmd &
		fi
		shift
	done
}

mountdev()   # $1=device  [$2=label]  [$3=devtype or fstype]
{
	# set label comment
	if [ "$2" = "" ]; then
		lblcmt=""
	else
		lblcmt="($2)"
	fi
	# set mount-fstype option
	if [ "$mountfstype" != "" ]; then
		fst="--mount-fstype"
	else
		fst=""
	fi
	# set default mount options
	if [ "$mountoptions" != "" ]; then
		mopts="$mountoptions"
	else
		mopts="$defaultmountoptions"
	fi
	# set sync mount options
	if (( syncopt == 1 )); then
		case "$3" in
			ext2 | ext3 | ext4 | ufs | ntfs )
				mopts="$mopts,sync";;
			fat | vfat )
				mopts="$mopts,flush";;
		esac
	fi
	# mount
	mntmsg="devmon: mount $1 --mount-options $mopts $fst $mountfstype  $lblcmt"
	if [ "$3" != "nofs" ]; then
		echo "$mntmsg" 
	fi
	umsg=`udisks --mount $1 --mount-options "$mopts" $fst $mountfstype 2>&1`
	mounterr="$?"
	# get mount point
	point=`echo "$umsg" | grep "^Mounted " | sed 's/^Mounted .* at \(.*\)/\1/'`
	if [ "$mounterr" != "0" ] || [ "$point" = "" ]; then
		# if $3=nofs then there was no apparent filesystem but we tried to mount it 
		# anyway in case it didn't report the filesystem accurately, so ignore the
		# error
		if [ "$3" != "nofs" ]; then
			echo "$umsg" 1>&2
			echo "devmon: error mounting $1 ($mounterr)" 1>&2
			if (( mountmode == 0 )) && (( polkiterrgiven != 1 )) && \
			   [ "$(echo "$umsg" | grep "Not Authorized")" != "" ]; then
				if (( nogui != 1 )); then
					( sleep 3 && WINDOWID="" zenity --error --no-wrap --title="devmon error" \
					  --text="udisks functions are not authorized through policykit,\nso devmon cannot automount drives.\nPlease see devmon's consolekit installation instructions:\n\nhttp://igurublog.wordpress.com/downloads/script-devmon/#install\n\n(To silence this pop-up add --no-gui to devmon's command line.)" &> /dev/null ) &
				fi
				echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" 1>&2
				echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" 1>&2
				echo "udisks functions are not authorized through policykit," 1>&2
				echo "so devmon cannot automount drives." 1>&2
				echo "Please see devmon's consolekit installation instructions:" 1>&2
				echo "http://igurublog.wordpress.com/downloads/script-devmon/#install" 1>&2
				echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" 1>&2
				echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" 1>&2	
				polkiterrgiven=1
			fi
		fi
		uerr=3
		return 3
	elif [ "$3" = "nofs" ]; then
		# no filesystem reported but successful mount
		echo "$mntmsg"
	fi
	echo "$umsg"
}

unmountdev()   # $1=device
{
	if [ "$unmountoptions" != "" ]; then
		echo "devmon: unmount $1 --unmount-options $unmountoptions"
		uerrmsg=`udisks --unmount $1 --unmount-options "$unmountoptions" 2>&1`
	else
		echo "devmon: unmount $1"
		uerrmsg=`udisks --unmount $1 2>&1`
	fi
	if [ "$uerrmsg" != "" ]; then
		# bug: udisks returns $?==0 when unmount fails
		echo "$uerrmsg"
		uerr=3
		return 3
	fi
}

ejectdev()
{
	if [ "$ejectoptions" != "" ]; then
		echo "devmon: eject $1 --eject-options $ejectoptions"
		udisks --eject $1 --eject-options "$ejectoptions"
	else
		echo "devmon: eject $1"
		udisks --eject $1
	fi			
	if [ "$?" != "0" ]; then
		uerr=3
		return 3
	fi
}

mountdrive()   # $1=device $2=label  [$3=devtype or fstype]
{
	dv="$1"
	lb="$2"
	tp="$3"
	unset point

	if ( ignoredevice "$dv" ) || ( ignorelabel "$lb" ); then
		return
	fi

	# mount
	if [ "$tp" != "audiocd" ] && (( nomount != 1 )); then
		mountdev $dv "$lb" "$tp"
		if [ "$?" != "0" ]; then
			return
		fi
	fi

	# exec on device
	unset execdone
	x=0
	while (( x < execodx )); do
		if [ "${execod1[$x]}" = "$dv" ]; then
			usercmd="${execod2[$x]}"
			usercmd="${usercmd//%f/$dv}"
			usercmd="${usercmd//%l/'$lb'}"
			usercmd="${usercmd//%d/'$point'}"
			if [ "$usercmd" != "" ]; then
				echo "devmon: [exec on device] eval $usercmd"
				eval $usercmd &
				execdone=1
			fi
		fi
		(( x++ ))
	done
	if (( execdone == 1 )); then return; fi

	# exec on label
	x=0
	while (( x < execolx )); do
		if [ "${execol1[$x]}" = "$lb" ]; then
			usercmd="${execol2[$x]}"
			usercmd="${usercmd//%f/$dv}"
			usercmd="${usercmd//%l/'$lb'}"
			usercmd="${usercmd//%d/'$point'}"
			if [ "$usercmd" != "" ]; then
				echo "devmon: [exec on label] eval $usercmd"
				eval $usercmd &
				execdone=1
			fi
		fi
		(( x++ ))
	done
	if (( execdone == 1 )); then return; fi

	# exec on video
	if [ "$tp" = "dvd" ] && [ "$point" != "" ] && [ -d "$point/VIDEO_TS" ]; then
		echo "devmon: videodvd $dv ($lb) on $point"
		if (( execovx != 0 )); then
			execcommands "exec on video" "${execov[@]}"
		fi
		return
	fi

	# exec on audio
	if [ "$tp" = "audiocd" ]; then
		echo "devmon: audiocd $dv ($lb)"
		if (( execoax != 0 )); then
			execcommands "exec on audio" "${execoa[@]}"
		fi
		return
	fi

	# exec on disc
	if [ "$tp" = "optical" ] || [ "$tp" = "dvd" ]; then
		if [ "$point" != "" ] || (( nomount == 1 )); then
			if (( execomx != 0 )); then
				execcommands "exec on disc" "${execom[@]}"
			fi
		fi
		return
	fi

	# exec on drive
	if [ "$point" != "" ] || (( nomount == 1 )); then
		if (( execoix != 0 )); then
			execcommands "exec on drive" "${execoi[@]}"
		fi
	fi

	# info on mount
	if [ "$point" != "" ] && (( infomount == 1 )) && (( nomount != 1 )); then
		sleep .5
		echo "devmon: [info on mount] $dv"
		if (( nogui != 1 )); then
			WINDOWID="" zenity --info --text="The following device has been mounted:\n\n$(df -hT "$dv" \
			   | grep "$dv" | awk '{print "Device:\\t"$1"\x0AType:\\t"$2"\nSize:\\t\\t"$3"\nUsed:\\t"$4"\n""Avail:\\t"$5"\nUse%:\\t"$6"\nMount:\\t"$7,$8,$9,$10}')\nLabel:\\t$lb" --title="devmon mount" &
		fi
		df -hT "$dv"
	fi
}

mountalldrives()
{
	# Mount all optical drives, no exec
	x=0
	while [ -e /dev/sr$x ]; do
		driveinfo /dev/sr$x
		if [ "$numaudiotracks" = "" ]; then
			numaudiotracks=0
		fi
		if [ "$systeminternal" != "1" ] && [ "$opticaldisc" = "1" ] && \
		   [ "$ismounted" != "1" ] && [ "$hasmedia" != "0" ] && \
		   [ "$blank" != "1" ] && (( numaudiotracks == 0 )) && \
		   [ "$nopolicy" != "1" ]; then
			if ( ignoredevice "/dev/sr$x" ) || ( ignorelabel "$label" ); then
				(( x++ ))
				continue
			fi
			mountdev /dev/sr$x "$label"
			eval notejectedsr$x=1
		fi
		(( x++ ))
	done
	# Mount removable drives, no exec
	IFSOLD="$IFS"
	IFS=$'\n'
	partlist=`grep " sd[a-z0-9]*$" /proc/partitions | sed 's/.* \(sd[a-z0-9]*\)/\1/'`
	for p in $partlist; do
		if ( ignoredevice "/dev/$p" ); then
			continue
		fi
		driveinfo /dev/$p
		if ( ignorelabel "$label" ); then
			continue
		else
			if [ "$systeminternal" != "1" ] && [ "$opticaldisc" != "1" ] && \
					   [ "$ismounted" = "0" ] && [ "$nopolicy" != "1" ]; then
				if [ "$usage" = "filesystem" ]; then
					echo "mountdev /dev/$p $label $type"
					mountdev /dev/$p "$label" "$type"
				else
					mountdev /dev/$p "$label" nofs
				fi
			fi
		fi
	done
	IFS="$IFSOLD"	
}

trapexit()
{
	kill $COPROC_PID 2> /dev/null

	# prevent trap code from executing multiple times on different signals
	if (( trapdone != 1 )); then
		trapdone=1
		# Unmount All
		if (( nounmount != 1 )); then
			IFSOLD="$IFS"
			IFS=$'\n'
			uerr=0
			partlist=`grep " sd[a-z0-9]*$" /proc/partitions | \
			          sed 's/.* \(sd[a-z0-9]*\)/\1/'`
			for p in $partlist; do
				if ( ignoredevice "/dev/$p" ); then
					continue
				fi
				driveinfo /dev/$p
				if ( ignorelabel "$label" ); then
					continue
				else
					if [ "$systeminternal" != "1" ] && [ "$opticaldisc" != "1" ] && \
							   [ "$usage" = "filesystem" ] && [ "$ismounted" = "1" ]; then
						echo "devmon: [on exit] unmount /dev/$p &"
						udisks --unmount /dev/$p &
						if [ "$?" != "0" ]; then
							uerr=3
						fi
					fi
				fi
			done
			IFS="$IFSOLD"
		fi
		echo 'devmon: stopped'
		exit $uerr
	fi
}

# Client Mode
if (( mountmode != 0 )); then
	uerr=0
	if (( unmountrem == 1 )) || (( unmountrecent == 1 )); then
		y=0
		unset udrive zpid
		IFSOLD="$IFS"
		IFS=$'\n'
		if (( unmountrem == 1 )); then
			# Unmount All Removable Drives
			partlist=`grep " sd[a-z0-9]*$" /proc/partitions | sed 's/.* \(sd[a-z0-9]*\)/\1/'`
			msgtitle="devmon unmount"
		else
			# Unmount Recent
			partlist=`mount | grep "^/dev/.* on " | sed 's/^\/dev\/\(.*\) on .*/\1/' \
			          | grep -v -e "null" -e "shm" -e "mapper" -e "snd" \
			          -e "video" -e "random" | tac`
			msgtitle="devmon unmount recent"
		fi
		for p in $partlist; do
			if ( ignoredevice "/dev/$p" ); then
				continue
			fi
			driveinfo /dev/$p
			if ( ignorelabel "$label" ); then
				continue
			else
				if [ "$systeminternal" != "1" ] && [ "$opticaldisc" != "1" ] \
						   && [ "$ismounted" = "1" ]; then
					udrive[$y]="/dev/$p"
					(( y++ ))
					if (( unmountrem != 1 )); then break; fi
				fi
			fi
		done
		IFS="$IFSOLD"
		if (( y == 0 )); then
			msg="No removable drives are mounted"
			echo "$msg"
		else
			msg="Unmounting ${udrive[@]}...\n\n(This dialog will close when the devices are unmounted)"
			echo "Preparing to unmount ${udrive[@]}"
		fi
		if (( nogui != 1 )); then
			WINDOWID="" zenity --info --title="$msgtitle" --text="$msg" &> /dev/null &
			zpid=$!
		fi
		if (( y > 0 )); then
			echo "devmon: sync"
			sync
			for d in ${udrive[@]}; do
				unmountdev $d
				if [ "$?" != "0" ] && (( nogui != 1 )); then
					driveinfo "$d" quiet
					if [ "$label" = "" ]; then
						lb=""
					else
						lb=" ($label)"
					fi
					msg="Unmount error on $d$lb:\n\n$uerrmsg"
					WINDOWID="" zenity --error --title="$msgtitle" --text="$msg" &> /dev/null &
				fi	
			done
			echo "devmon: sync"
			sync
		fi
		if [ "$zpid" != "" ]; then
			sleep 2
			kill $zpid 2> /dev/null
		fi
	fi

	# Unmount Optical
	if (( unmountoptical == 1 )); then
		x=0
		while [ -e "/dev/sr$x" ]; do
			if ( ignoredevice "/dev/sr$x" ); then
				(( x++ ))
				continue
			fi
			driveinfo /dev/sr$x
			if ( ignorelabel "$label" ); then
				(( x++ ))
				continue
			else
				if [ "$systeminternal" != "1" ] && [ "$opticaldisc" = "1" ] && \
						   [ "$ismounted" = "1" ]; then
					unmountdev /dev/sr$x
					if [ "$?" != "0" ] && (( nogui != 1 )); then
						if [ "$label" = "" ]; then
							lb=""
						else
							lb=" ($label)"
						fi
						msg="Unmount error on /dev/sr$x$lb:\n\n$uerrmsg"
						WINDOWID="" zenity --error --title="devmon unmount optical" \
						                   --text="$msg" &> /dev/null &
					fi	
				fi
			fi
			(( x++ ))
		done
	fi

	# Unmount DIR|DEVICE
	if (( umntx > 0 )); then
		x=0
		while (( x < umntx )); do
			d="${umnt[$x]}"
			# remove trailing slash
			if [ "$d" != "/" ]; then
				d="${d%/}"
			fi
			if [ "${d:0:5}" = "/dev/" ]; then
				# Unmount DEVICE
				unmountdev "$d"
			else
				# Unmount DIR
				if [ "$(dirname "$d")" = "." ]; then
					if [ -d "$(pwd)/$d" ]; then
						d="$(pwd)/$d"
					elif [ -d "/media/$d" ]; then
						d="/media/$d"
					elif [ -e "/dev/$d" ] && [ "$(mount | grep "^/dev/$d on ")" != "" ]; then
						unmountdev "/dev/$d"
						(( x++ ))
						continue
					fi
				fi
				if [ ! -d "$d" ]; then
					echo "devmon: No such directory or mounted device $d" 1>&2
					uerr=3
				else
					dv=`mount | grep -m 1 " on $d type " | awk '{print $1}'`
					if [ "$dv" = "" ]; then
						echo "devmon: Nothing mounted on $d (mtab)" 1>&2
						uerr=3
					else
						unmountdev "$dv"
					fi
				fi
			fi
			(( x++ ))
		done
	fi

	# Eject DIR|DEVICE
	if (( ejx > 0 )); then
		x=0
		while (( x < ejx )); do
			d="${ej[$x]}"
			# remove trailing slash
			if [ "$d" != "/" ]; then
				d="${d%/}"
			fi
			dv=""
			if [ "${d:0:5}" = "/dev/" ]; then
				# Eject DEVICE
				dv="$d"
			else
				# Eject DIR
				if [ "$(dirname "$d")" = "." ]; then
					if [ -d "$(pwd)/$d" ]; then
						d="$(pwd)/$d"
					elif [ -d "/media/$d" ]; then
						d="/media/$d"
					elif [ -e "/dev/$d" ] && [ "$(mount | grep "^/dev/$d on ")" != "" ]; then
						dv="/dev/$d"
					fi
				fi
				if [ "$dv" = "" ]; then
					if [ ! -d "$d" ]; then
						echo "devmon: No such directory or mounted device $d" 1>&2
						uerr=3
					else
						dv=`mount | grep -m 1 " on $d type " | awk '{print $1}'`
						if [ "$dv" = "" ]; then
							echo "devmon: Nothing mounted on $d (mtab)" 1>&2
							uerr=3
						fi
					fi
				fi
			fi
			if [ "$dv" != "" ]; then
				driveinfo "$dv"
				if [ "$systeminternal" != "1" ] && [ "$opticaldisc" = "1" ] && \
				   [ "$ismounted" = "1" ]; then
					unmountdev "$dv"
				fi
				ejectdev "$dv"
			fi
			(( x++ ))
		done
	fi
	
	# Mount DEVICE
	if (( mntx > 0 )); then
		x=0
		while (( x < mntx )); do
			d="${mnt[$x]}"
			# remove trailing slash
			if [ "$d" != "/" ]; then
				d="${d%/}"
			fi
			if [ "$(dirname "$d")" = "." ] && [ "${d:0:5}" != "/dev/" ]; then
				d="/dev/$d"
			fi
			driveinfo "$d" quiet
			if [ "$opticaldisc" = "1" ]; then
				mountdev $d "$label"
			else
				mountdev $d "$label" "$type"
			fi
			(( x++ ))
		done	
	fi

	# Mount All Unmounted
	if (( mountall == 1 )); then
		mountalldrives
	fi

	exit $uerr
fi

# Daemon Mode
if [ "$mountfstype" != "" ]; then
	echo "WARNING: --mount-fstype ignored in daemon mode" 1>&2
	mountfstype=""
fi
if [ "$unmountoptions" != "" ]; then
	echo "WARNING: --unmount-options ignored in daemon mode" 1>&2
	unmountoptions=""
fi
if [ "$ejectoptions" != "" ]; then
	echo "WARNING: --eject-options ignored in daemon mode" 1>&2
	ejectoptions=""
fi
pidcount=`ps h -C ${0//*\//} -o pid | wc -l`
if (( pidcount > 2 )); then
	echo
	echo "WARNING: multiple instances of devmon appear to be running"
	echo
fi

# Trigger udisks daemon start two ways
if [ -e /dev/sr0 ]; then
	udisks --poll-for-media /dev/sr0
fi
udisks --show-info /dev/sda > /dev/null
sleep 2 # helps successful sr0 startup mount on reboot


# Startup Mounting
if (( nomount != 1 )); then
	mountalldrives
fi

# Start monitoring
coproc udisks --monitor
err=$?
trap trapexit EXIT SIGINT SIGTERM SIGQUIT
trap "echo devmon: ignored HUP" SIGHUP

if [ $err != "0" ] || [ ! ps -p $COPROC_PID &>/dev/null ]; then
	echo "devmon: unable to start udisks --monitor" 1>&2
	echo "        is udisks installed and dbus running?" 1>&2
	exit 2
fi


# Monitoring Loop
while ps -p $COPROC_PID &>/dev/null; do
	read -u ${COPROC[0]}
	echo "==========================================="
	echo "$REPLY"
	event="${REPLY%:*}"
	devpath="${REPLY#*:}"
	devpath="/dev/${devpath##*/}"
    if [ "$event" != "" ] && [ "$devpath" != "/dev/" ] && [ -e "$devpath" ]; then
		case $event in
			added )
				driveinfo $devpath
				if [ "$systeminternal" != "1" ] && [ "$ismounted" = "0" ] && \
				   [ "$nopolicy" != "1" ]; then
				    if [ "$usage" = "filesystem" ]; then
						mountdrive $devpath "$label" "$type"
					else
						mountdrive $devpath "$label" nofs
					fi
				fi
				;;
			job-changed )
				;;
			removed )
				;;
			changed )
				driveinfo $devpath
				eval notejected=\$notejected${devpath#/dev/}
				eval devmounted=\$devmounted${devpath#/dev/}
				eval devmounted${devpath#/dev/}="$ismounted"
				# If notejected==1 then cd has not been ejected and was probably
				# manually unmounted, so don't automount it.  Otherwise
				# devmon will instantly mount any manual unmount.
				if [ "$systeminternal" != "1" ] && [ "$opticaldisc" = "1" ] && \
				   [ "$ismounted" != "1" ] && [ "$hasmedia" != "0" ] && \
				   [ "$blank" != "1" ] && (( notejected != 1 )) && \
				   [ "$nopolicy" != "1" ]; then
					if [ "$media" = "optical_dvd" ]; then
						mountdrive $devpath "$label" dvd
					elif (( numaudiotracks > 0 )); then
						mountdrive $devpath "$label" audiocd
					else
						mountdrive $devpath "$label" optical
					fi
					eval notejected${devpath#/dev/}=1
				else
					if [ "$systeminternal" != "1" ] && \
						 [ "$ismounted" != "1" ] && [ "$hasmedia" = "0" ]; then
						# disc ejected
						echo "devmon: $devpath eject detected"
						eval notejected${devpath#/dev/}=0
					fi
					if [ "$systeminternal" != "1" ] && [ "$ismounted" != "1" ] && \
					   [ "$nopolicy" != "1" ] && [ "$devmounted" = "1" ]; then					
						# exec-on-unmount
						if (( execoux != 0 )); then
							if ( ! ignoredevice "$devpath" ) && ( ! ignorelabel "$label" ); then
								dv="$devpath"
								execcommands "exec on unmount" "${execou[@]}"
							fi
						fi
					fi
				fi
			;;
		esac
    elif [ "$event" = "removed" ]; then
		eval unset devmounted${devpath#/dev/}
		# exec-on-remove
		if (( execorx != 0 )); then
			if ( ! ignoredevice "$devpath" ); then
				unset lb point
				dv="$devpath"
				execcommands "exec on remove" "${execor[@]}"
			fi
		fi
    fi
done

exit

# CHANGELOG
# 1.0.5: --exec-on-unmount now executes only once per unmount
# 1.0.4: added --exec-on-unmount, --exec-on-remove
#        added multiple instance warning
#        %f device spec no longer passed in quotes to commands
# 1.0.3: help updated for sync
#        corrected exec-on-drive bug introduced in 1.0.1
# 1.0.2: --sync adds sync for ntfs
# 1.0.1: added --sync
#        added --no-unmount; unmount-on-exit is now default
#        obey UDISKS_PRESENTATION_NOPOLICY to inhibit automount
#        added unmount-all error pop-ups
# 1.0.0: added --unmount-recent
#        improved udisks mount info to stdout
#        improved mount/unmount without apparent filesystem
# 0.9.5: ignore trailing slash in un/mount dir/dev specs
#        attempt to mount no filesystem in case there is one
# 0.9.4: polkit error more selective
# 0.9.3: corrected problems with spaces in volume labels
#        corrected un/mounting of partitionless devices
#        corrected --ignore-devices problem
#        added not authorized pop-up error
# 0.9.2: added --internal
# 0.9.1: corrected --unmount-on-removable not recognized
# 0.9.0: changed --unmount-all to --unmount-removable
#        added --unmount-all (now includes optical)
#        added --unmount-optical
#        added --unmount DIR|DEVICE
#        added --eject DIR|DEVICE
#        added --mount-all
#        added --mount DEVICE
#        added --info-on-mount
#        added pass options to udisks
#        better error msg from udisks
#        individual drive eject detection
#        no limit on number of optical drives
# 0.8.2: more verbose errors
#        run as root changed to warning
# 0.8.1: added --unmount-on-exit
#        adjusted start daemon trigger, timing
#        added 'do not run as root' catcher
#        added trap to ignore SIGHUP

