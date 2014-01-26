#/***************************************************************************
#*                      ZBOSS ZigBee Pro 2007 stack                         *
#*                                                                          *
#*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
#*                       http://www.dsr-wireless.com                        *
#*                                                                          *
#*                            All rights reserved.                          *
#*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
#*                       http://www.claridy.com/                            *
#*                                                                          *
#*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
#*                             Hsinchu, Taiwan.                             *
#*                       http://www.ubec.com.tw/                            *
#*                                                                          *
#*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
#*                       http://www.dsr-wireless.com                        *
#*                                                                          *
#*                            All rights reserved.                          *
#*                                                                          *
#*                                                                          *
#* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
#* under either the terms of the Commercial License or the GNU General      *
#* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
#* may choose which license to receive this code under (except as noted in  *
#* per-module LICENSE files).                                               *
#*                                                                          *
#* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
#* Research LLC.                                                            *
#*                                                                          *
#* GNU General Public License Usage                                         *
#* This file may be used under the terms of the GNU General Public License  *
#* version 2.0 as published by the Free Software Foundation and appearing   *
#* in the file LICENSE.GPL included in the packaging of this file.  Please  *
#* review the following information to ensure the GNU General Public        *
#* License version 2.0 requirements will be met:                            *
#* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
#*                                                                          *
#* Commercial Usage                                                         *
#* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
#* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
#* Agreement provided with the Software or, alternatively, in accordance    *
#* with the terms contained in a written agreement between you and          *
#* ClarIDy/UBEC/DSR.                                                        *
#*                                                                          *
#****************************************************************************
#PURPOSE: 
#*/

#
# TARGETS:
#   all, rebuild, clean, clobber, tags, docs
#

#
# Can't include Options because it includes deps which can be absent.
# Separate makefile includes to Options and Platform.
# Options includes Platform, so it is the only file to be included from non-root makefiles
#
include Platform

all: bldall

clean: cleandep

bldall clean depend :
	if [ -d osif/$(PLATFORM) ] ; then cd osif/$(PLATFORM) && $(MAKE) $@ ; else cd osif/unix  && $(MAKE) $@ ; fi
	cd common                 && $(MAKE) $@
	cd mac                    && $(MAKE) $@
	cd nwk                    && $(MAKE) $@
	cd aps                    && $(MAKE) $@
	cd zdo                    && $(MAKE) $@
	cd secur                  && $(MAKE) $@
	cd tests                  && $(MAKE) $@
	cd devtools               && $(MAKE) $@	

test:
	cd tests                  && $(MAKE) $@

bldall : deps

depend : cleandep deps

deps:
	> deps
	make depend

cleandep :
	> deps

clobber: clean
	$(FIND) . \( -name '*~' -o -name '.#*' -o -name 'core' -o -name 'core.*' -o -type l -a -name Makefile \) -exec rm {} \;
	rm -f deps  TAGS tags BROWSE

tags:
	($(FIND) . -name \*.[ch] -print) | grep -v ".*~" | etags -

rebuild: makefile_links clean depend all

# First remove all stale makefiles, then link makefiles from mk/platform
makefile_links:
	$(FIND) . -type d -name mk | sed -e 's/\/mk//' | while read a ;		     \
        do	    	    	       	      	       	       	      		     \
	  rm -f $$a/Makefile ;							     \
	  if [ -f $$a/mk/$(PLATFORM)/Makefile ] ; then				     \
            eval $(LN)mk/$(PLATFORM)/Makefile $$a/Makefile ;		     		     \
	  else									     \
	    eval $(LN)mk/unix/Makefile $$a/Makefile ; 			     		     \
	  fi ;									     \
        done 	   			      		     	      		     \

test:

docs: doc_api doc_int


doc_int:
	rm -rf doxydoc_int
	doxygen Doxyfile.INT

doc_api:
	rm -rf doxydoc_api
	doxygen Doxyfile.API
