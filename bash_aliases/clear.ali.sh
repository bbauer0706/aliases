#!/bin/bash
##############################################################################
#                                                                            #
#                         CLEAR COMMAND ALIASES                              #
#                                                                            #
##############################################################################

# Original typos
alias clera='clear'
alias clare='clear'
alias claer='clear'
alias cler='clear'
alias cear='clear'
alias claar='clear'
alias clar='clear'
alias cleaar='clear'
alias clearr='clear'
alias cleasr='clear'
alias cleaer='clear'
alias cleer='clear'
alias ccllear='clear'
alias lear='clear'
alias cleqr='clear'
alias clewr='clear'
alias clezr='clear'

# Additional common typos
alias clera='clear'
alias cliare='clear'
alias cliear='clear'
alias cliar='clear'
alias cliaar='clear'
alias clieer='clear'
alias cliere='clear'
alias clieaar='clear'
alias cliearr='clear'
alias cliare='clear'
alias cliare='clear'
alias cliaer='clear'
alias cleiar='clear'
alias cleiaar='clear'
alias cleiear='clear'
alias cleiaer='clear'
alias cleier='clear'
alias cleieer='clear'
alias cleiere='clear'
alias cleieaar='clear'
alias cleiearr='clear'

# More typos with different patterns
alias celar='clear'
alias celaar='clear'
alias celear='clear'
alias celaer='clear'
alias cealr='clear'
alias ceaalr='clear'
alias cealr='clear'
alias cealr='clear'
alias cealr='clear'
alias claeir='clear'
alias claier='clear'
alias claieer='clear'
alias claiere='clear'
alias claieaar='clear'
alias claiearr='clear'

# One-handed typos (common when typing fast)
alias vlera='clear'
alias vlare='clear'
alias vlear='clear'
alias vlaer='clear'
alias vler='clear'
alias vear='clear'
alias clera='clear'
alias xlera='clear'
alias xlare='clear'
alias xlear='clear'
alias xlaer='clear'
alias xler='clear'
alias xear='clear'

# Keyboard layout typos
alias ckear='clear'
alias ckera='clear'
alias ckare='clear'
alias cjear='clear'
alias cjera='clear'
alias cjare='clear'

# Double letter typos
alias cclear='clear'
alias cllear='clear'
alias cleear='clear'
alias cleaar='clear'
alias clearr='clear'

# Missing first letter
alias lear='clear'
alias lera='clear'
alias lare='clear'
alias laer='clear'
alias ler='clear'
alias ear='clear'

# Common finger slip patterns
alias clrear='clear'
alias clrear='clear'
alias clrear='clear'
alias clrear='clear'
alias clrear='clear'

# Backwards/mixed up
alias raelc='clear'
alias aelrc='clear'
alias learc='clear'
alias rceal='clear'

# German keyboard specific typos (QWERTZ layout)
# Z and Y are swapped compared to QWERTY
alias cleaz='clear'
alias clezr='clear'
alias clez='clear'
alias cleayr='clear'
alias cleay='clear'
alias cleyar='clear'
alias cleyr='clear'
alias cley='clear'

# Ä, Ö, Ü next to P, L, O respectively - common adjacent key errors
alias cleäar='clear'
alias cleär='clear'
alias cleä='clear'
alias cleöar='clear'
alias cleör='clear'
alias cleö='clear'
alias cleüar='clear'
alias cleür='clear'
alias cleü='clear'

# German ß key (next to 0) accidentally hit
alias cleaß='clear'
alias cleß='clear'
alias clßear='clear'
alias cßlear='clear'

# Adjacent key mistakes on German keyboard
# C is next to V and X
alias vleak='clear'
alias vlera='clear'
alias xleak='clear'

# L is next to Ö, K, O
alias cöear='clear'
alias cöera='clear'
alias ckear='clear'
alias coear='clear'

# E is next to R, W, D
alias clwar='clear'
alias clwear='clear'
alias cldar='clear'
alias cldear='clear'

# A is next to S, Y (on German keyboard)
alias clesar='clear'
alias clesr='clear'
alias cleyr='clear'
alias cleyar='clear'

# R is next to T, F, E
alias cleat='clear'
alias cleatar='clear'
alias cleaft='clear'
alias cleafr='clear'

# Common German typo patterns with umlauts accidentally included
alias cläar='clear'
alias clöar='clear'
alias clüar='clear'
alias cläer='clear'
alias clöer='clear'
alias clüer='clear'

# Super short versions for really bad typos
alias cr='clear'
alias cl='clear'
alias ce='clear'
alias le='clear'

##############################################################################
#                     SMART COMMAND-NOT-FOUND HANDLER                       #
##############################################################################

# Smart command-not-found handler for clear command
command_not_found_handle() {
    local cmd="$1"

    # Convert to lowercase for comparison
    local lower_cmd=$(echo "$cmd" | tr '[:upper:]' '[:lower:]')

    # Check if command contains all letters of "clear" (c, l, e, a, r)
    if [[ "$lower_cmd" =~ c ]] && [[ "$lower_cmd" =~ l ]] && [[ "$lower_cmd" =~ e ]] && [[ "$lower_cmd" =~ a ]] && [[ "$lower_cmd" =~ r ]]; then
        # Additional checks to avoid false positives
        local cmd_length=${#lower_cmd}

        # Only trigger for commands between 3-8 characters (reasonable for clear typos)
        if [ "$cmd_length" -ge 3 ] && [ "$cmd_length" -le 8 ]; then
            clear
            return 0
        fi
    fi

    # Fall back to default command not found behavior
    if [ -x /usr/lib/command-not-found ]; then
        /usr/lib/command-not-found -- "$1"
        return $?
    elif [ -x /usr/share/command-not-found/command-not-found ]; then
        /usr/share/command-not-found/command-not-found -- "$1"
        return $?
    else
        echo "bash: $1: command not found" >&2
        return 127
    fi
}