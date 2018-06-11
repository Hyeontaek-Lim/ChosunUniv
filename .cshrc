# @(#)Cshrc 1.3 88/02/08 SMI
#################################################################
#
#         .cshrc file
#
#         initial setup file for both interactive and noninteractive
#         C-Shells
#
#################################################################
# set this for all shells

umask 002
set noclobber
set filec

# settings for interactive shells

set history = 100
set ignoreeof
set notify
set savehist = 100

# settings stty attributes
stty erase "^H"
if ($?TERM) then
	stty erase "^H"
	stty -parenb
	stty cs8
	stty -istrip
endif

# set up path

set path=(/sbin /usr/sbin /bin /usr/bin /usr/ccs/bin /usr/ucb /etc)
set path=(/usr/local/bin $path)
set path=(. .. /home/jhshim/util $path)

set prompt="[`echo $cwd`] \!: "

# aliases for all shells
alias cd  'cd \!*;set prompt="[`echo $cwd`] \\!: "'
alias ff            'find . -name \!* -print'
alias cls           'clear'
alias rm            'rm -i'
alias ls            'ls -aF'
alias ll            'ls -alF'

