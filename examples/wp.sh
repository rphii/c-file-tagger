#!/bin/bash

OPTIND=1         # Reset in case getopts has been used previously in the shell.

cft_input_path=$HOME/Images/Wallhaven   # adjust if you have a different directory
images_path=$cft_input_path

pick_background() {
    previous=$(cat $HOME/.config/cft/bg-current.txt)
    bg=""
    if [[ $# -gt 1 ]]; then
        while : ; do
            index=$(($RANDOM%$#+1))
            bg=${@:$index:1}
            base=${bg##*/}
            short=${base%.*}
            [[ $previous == $short ]] || break # make sure we have a different wallpaper than before
        done
    elif [[ $# -gt 0 ]]; then
        bg=${@:1:1}
        base=${bg##*/}
        short=${base%.*}
    fi
    if [[ -z "$bg" ]]; then
        while : ; do
            bg=$(find $cft_input_path -type f \( -name "*.png" -o -name "*.jpg" \) | shuf -n 1)
            base=${bg##*/}
            short=${base%.*}
            [[ $previous == $short ]] || break # make sure we have a different wallpaper than before
        done
    fi
    fade=1 #$(((RANDOM % 1)+1))
    echo $short > ~/.config/cft/bg-current.txt
    swwwargs="--transition-type simple --transition-step 7 --transition-duration $fade --transition-fps 120"
    swww img -o eDP-1 "$bg" $swwwargs 2>/dev/null
    swww img -o HDMI-A-1 "$bg" $swwwargs 2>/dev/null
}

restart_waybar() {
    pid=$(pidof waybar)
    if [[ -n "$pid" ]]; then
        pkill waybar
        nohup waybar >/dev/null 2>&1 &
    fi
}

propose_config() {
    change=$(OPENCV_LOG_LEVEL=OFF python $HOME/Tools/estimage_textcol_from_image.py "$bg")
    style="$HOME/.config/waybar/style.css"
    style_proposed="$HOME/.config/waybar/style.css${change}"
    if [ -f "${style_proposed}" ]; then
        update=$(diff ${style} ${style_proposed})
        if [[ -n "$update" ]]; then
            cp "${style_proposed}" "${style}"
            restart_waybar
        fi
    else
        echo "doesn't exist"
    fi
}

arguments() {

    any=""
    and=""
    not=""
    list=false

    POSITIONAL_ARGS=()
    while [[ $# -gt 0 ]]; do
        case $1 in
            -A|--and)
                and="$2"
                shift
                shift
                ;;
            -O|--any)
                any="$2"
                shift
                shift
                ;;
            -N|--not)
                not="$2"
                shift
                shift
                ;;
            -l|--list)
                list=true
                shift
                ;;
            -*--*)
                echo "Unknown option $1"
                exit 1
                ;;
            *)
                POSITIONAL_ARGS+=("$1") # save positional arg
                shift
                ;;
        esac
    done

    set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

    cmdline=""
    if [[ -n "$any" ]]; then
        cmdline="${cmdline} --any $any"
    fi
    if [[ -n "$and" ]]; then
        cmdline="${cmdline} --and $and"
    fi
    if [[ -n "$not" ]]; then
        cmdline="${cmdline} --not $not"
    fi
    if $list; then
        cmdline=" --list-files"
        cmd="cft -deir n ${cft_input_path} ${cmdline}"
        eval $cmd
        exit
    fi

}

arguments $@

if [[ -n "$cmdline" ]]; then
    cmd="cft -eir ${cft_input_path} ${cmdline}"
    wp=($(eval "$cmd"))
    if [[ ${#wp[@]} -gt 0 ]]; then
        pick_background "${wp[@]}"
    fi
else
    pick_background "${POSITIONAL_ARGS[@]}"
fi

if test -f "$HOME/Tools/estimage_textcol_from_image.py"; then
    propose_config
fi


