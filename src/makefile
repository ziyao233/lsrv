# lsrv
# File:/src/makefile
# Date:2022.08.30
# By MIT License.
# Copyright(C) 2022 Ziyao.All rights reserved.

# Tools
CC:=cc
ECHO:=echo
RM:=rm
INSTALL:=install

# Direnct
DIR_INSTALL:=/usr/local/bin

# Flags
CFLAGS:=-O0 -g -pedantic -Wextra -Wall -Werror
LDFLAGS:=-l lua -l m -g

# Outputs
OBJS:=lsrv.o
APP:=lsrv


default:build

%.o:%.c
	@$(ECHO) [CC] $< $@
	@$(CC) $< -c $(CFLAGS)

build:$(OBJS)
	@$(ECHO) [LD] $(OBJS) $(APP)
	@$(CC) $(OBJS) -o $(APP) $(LDFLAGS)

clean:
	@$(ECHO) [RM] $(OBJS)
	@$(RM) *.o

disbuild:clean
	@$(ECHO) [RM] $(APP)
	@$(RM) $(APP)

install:default
	@$(ECHO) [INSTALL] $(APP) $(DIR_INSTALL)
	@$(INSTALL) $(APP) $(DIR_INSTALL)
