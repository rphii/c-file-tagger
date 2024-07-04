# EARLY VERSION - DO NOT USE

# c-file-tagger

## Install

    git clone https://github.com/rphii/c-file-tagger
    cd c-file-tagger/src
    make install

## Usage

List all tags:

    cft --list

Tags have to be matched exactly (yes, case sensitive). (...reminder that this is an early version,
but I want the program to be quite flexible later on, so I'm thinking of adding an option, or
something...)

List files associated with either `sky` or `cloud`:

    cft --any sky,cloud
    cft -O sky,cloud

List files associated with `sky` and `cloud`:

    cft --and sky,cloud
    cft -A sky,cloud

List files associated with neither `sky` nor `cloud`:

    cft --not sky,cloud
    cft -N sky,cloud

If one were to combine `--any` `--and` and `--not`, the current order is fixed and as written in the
current sentence. Later on I want to allow more complex expressions. One can for example search for
any file with tags `sky` or `cloud`, of which include `girl` and `blue`, but is not `nfsw` nor
`sketchy`:

    cft --any sky,cloud --and girl,blue --not nsfw,sketchy

Almost none of the other options work and are more of a sketched out plan or route I might take. All
the other options can be listed with:

    cft --help

## Adding Tags

The current version is hard-programmed to look for a file in `/home/hp/.config/tags.cft`.
Yes, that is suboptimal, and I should change that asap.

Either way, the file format is basically a `.csv`. The first entry is a file name (or anything you
want, basically, doesn't have to be a file) and the rest are all tags associated with said file.

## About Tags

Tags are your normal, run-off-the-mill strings, with one small exception: They support a basic
folder structure. You can simply make one with a `:`. So, one can for example, instead of tagging
files with a generous:

    dragon-ball-wallpaper,anime
    bleach-wallpaper,anime
    some-anime-wallpaper,anime

rather use this approach:

    dragon-ball-wallpaper,anime:dragon-ball
    bleach-wallpaper,anime:bleach
    some-anime-wallpaper,anime

The folder structuring with `:` allows for more than a single level, too.

When searching for files with `anime`, one will find all three files regardless of folder or not.
To only find the Bleach-Wallpaper, you would do `cft --any anime:bleach`, which will respect your
choice of tag and only list those tagged accordingly.

