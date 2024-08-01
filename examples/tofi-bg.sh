#!/bin/bash

##### WHAT IS THIS #####
#
# - I'm using tofi here, basically rofi (but I like tofi more; faster, IMO simpler...)
# - selecting any of the entries in tofi will try to change the wallpaper, using 
#   the variable `wallpaper_changer`
# - it starts searching for tag files in the `cft_input_path` variable; adjust
#   if you want it somewhere else
#
#
##### USAGE #####
#
# spawn tofi with a selection of all tags
#
#   ./tofi-bg.sh                      
#
#
# spawn tofi with a selection of the current image
#
#   ./tofi-bg.sh "$(cat $HOME/.config/cft/bg-current.txt)"
#
#
# spawn tofi with a list of bookmarks of tags
# (each bookmark may be a comma-separated list of tags that
#  get searched with the --and switch)
#
#   ./tofi-bg.sh $HOME/.config/cft/tofi-bg-history-custom     
#

set -e

input=$1

histfile_auto=$HOME/.config/cft/tofi-bg-history-auto
histfile_custom=$HOME/.config/cft/tofi-bg-history-custom
cft_input_path=$HOME/Images         # adjust if you have a different directory
wallpaper_changer=wp                # install/link wp.sh to some place like /usr/bin/wp
histsearch=false
tofi_settings="--require-match=false --height=978 --width=682 --anchor=left --margin-left=8"

touch "$histfile_auto"
touch "$histfile_custom"

if [[ "$histfile_custom" == "$input" ]]; then
    all_output=$(cat "$histfile_custom")
    histsearch=true
elif [[ -n "$input" ]]; then
    all_output=$(cft -Tire "$cft_input_path" -l $(cft -ire "$cft_input_path" -Ld no | grep "$input"))
else
    all_output=$(cft -Tire "$cft_input_path" -l)
fi

if [ $histsearch = "true" ]; then

    while IFS= read -r line; do
        cmp=$(echo "$line" | awk '{print $2}')
        all_tags+="${cmp}"$'\n'
    done < "$histfile_custom"

    search=$(echo "$all_tags" | tofi --history-file=$histfile_custom $tofi_settings --placeholder-text="Bookmarks")
else
    num_tags=$(echo "$all_output" | head -n 1)
    if [[ -n "${input}" ]]; then
        num_tags+=" [${input}]"
    fi
    all_tags=$(echo "$all_output" | tail -n+2)
    search=$(echo "$all_tags" | tofi --history-file=$histfile_auto $tofi_settings --placeholder-text="$num_tags")
    search=$(echo "$search" | awk '{print $2}')
    #echo SEARCH: "$search"
fi


if [[ -n "$search" ]]; then

    # check if the result is in the history
    if [ $histsearch = "true" ]; then
        exists_in_hist=false
        while IFS= read -r line; do
            cmp=$(echo "$line" | awk '{print $2}')
            #echo COMPARE: [$cmp], [$search]
            if [[ "$cmp" == "$search" ]]; then
                exists_in_hist=true
                break
            fi
        done < "$histfile_custom"
        #echo "EXISTS IN HIST? $exists_in_hist"
        if [ "$exists_in_hist" = "false" ]; then
            echo "ADD $search TO HIST"
            echo "1 $search" >> $histfile_custom
        fi
    fi

    # finally, change background
    files=$(cft -ireA "$cft_input_path" "$search")
    $wallpaper_changer "$files"
fi

