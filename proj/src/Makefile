# name of the program (Minix service)
PROG=proj

.PATH: ${.CURDIR}
.PATH: ${.CURDIR}/labs
.PATH: ${.CURDIR}/game

# source code files to be compiled
SRCS = proj.c video_gr.c keyboard.c utils.c timer.c rtc.c manager.c alphabet.c game.c menu.c gamestate.c gameover.c

# additional compilation flags
# "-Wall -Wextra -Werror -I . -std=c11 -Wno-unused-parameter" are already set
CFLAGS += -pedantic #-D __LCOM_OPTIMIZED__

# list of library dependencies (for Lab 2, only LCF library)
DPADD += ${LIBLCF}
LDADD += -llcf

# include LCOM's makefile that does all the "heavy lifting"
.include <minix.lcom.mk>
