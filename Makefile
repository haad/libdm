USE_FORT?= no  # network protocol library                                                                    

LIB=    dm

SRCS=   libdm_ioctl.c
MAN=    

WARN= 4

CPPFLAGS+=      -I${.CURDIR}
CFLAGS = -g
INCS=           libdm.h netbsd-dm.h
INCSDIR=        /usr/include

.include <bsd.lib.mk>
