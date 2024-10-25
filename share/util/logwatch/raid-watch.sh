#!/bin/sh
# raid-watch
# Sun Zhenyu, 9/2/04
# sunzhenyu@world2.cn

#Address    Type              Manufacturer/Model         Capacity  Status
#---------------------------------------------------------------------------
#d0b0t1d0   RAID 1 (Mirrored) ADAPTEC  RAID-1            35003MB   Optimal

# get output from /usr/dpt/raidutil and log it if necessary
/usr/dpt/raidutil -L logical | perl -ne 's/.*Status\n//g; s/.*-----\n//g; s/.*Optimal\n//g; s/^\n//; @_ = /(.*)\n/g; foreach(@_){ if($_) { use Sys::Syslog; openlog("Kernel","cons, pid","kern"); syslog("alert","raid-watch alert!!! : $_"); closelog();  } }'
