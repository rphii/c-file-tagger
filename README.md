# EARLY VERSION
# c-file-tagger

[](https://github.com/rphii/c-file-tagger/assets/46871963/a05e7a42-4153-417c-8d75-81342cbb66dd)

- *\*The command `wp` is just a bash script that sets my background from a list of files.*
- *\*The video is from an older date and the arguments have changed slighly*

## Install

    git clone https://github.com/rphii/c-file-tagger
    cd c-file-tagger/src
    make install

## Usage

List all tags:

    cft --input tags.cft --list-tags
    cft -il tags.cft

    # list only tags of filename-A & filename-B
    cft --input tags.cft --list-tags filename-A filename-B

List all files:

    cft --input tags.cft --list-files
    cft -iL tags.cft

    # list only files filename-A & filename-B
    cft --input tags.cft --list-files filename-A filename-B

List all tags with files associated

    cft --input tags.cft --list-tags --list-files
    cft -ilL tags.cft

    # to disable decoration
    cft --input tags.cft --list-tags --list-files --decoration no
    cft -ilLd tags.cft no

List all files with tags associated

    cft --list-files --list-tags
    cft -Ll

    # to disable decoration
    cft --input tags.cft --list-files --list-tags --decoration no
    cft -iLld tags.cft no     # should in theory give the same output as your file(s) (combined)

Tags have to be matched exactly (yes, case sensitive). (...reminder that this is an early version,
but I want the program to be quite flexible later on, so I'm thinking of adding an option, or
something...)

List files associated with either `sky` or `cloud`:

    cft --input tags.cft --any sky,cloud
    cft -i tags.cft -O sky,cloud

List files associated with `sky` and `cloud`:

    cft --input tags.cft --and sky,cloud
    cft -i tags.cft -A sky,cloud

List files associated with neither `sky` nor `cloud`:

    cft --input tags.cft --not sky,cloud
    cft -i tags.cft -N sky,cloud

If one were to combine `--any` `--and` and `--not`, the current order is fixed and as written in the
current sentence. Later on I want to allow more complex expressions. One can for example search for
any file with tags `sky` or `cloud`, of which include `girl` and `blue`, but is not `nfsw` nor
`sketchy`:

    cft --input tags.cft --any sky,cloud --and girl,blue --not nsfw,sketchy

Searching files for substrings is also possible with the help of other CLI tools:

    cft --input tags.cft --any $(cft --list-tags --decorate no | grep SUBSTRING-TO-SEARCH | tr '\n' ',')
    cft -i tags.cft -O $(cft -ld no | grep FUZZY-SEARCH | tr '\n' ',')

Almost none of the other options work and are more of a sketched out plan or route I might take. All
the other options can be listed with:

    cft --help

## Adding Tags

By default no input nor output file is provided. Tag files have to end in `.cft`.

The file format is similar to that of a `.csv`. The first entry is a file name (or anything you
want, basically, doesn't have to be a file) and the rest are all tags associated with said file
(delimiter is a comma).

It is possible to specify the use of **one output file**:

    cft --output some/output/file.cft
    cft -o some/output/file.cft

One can also specify **multiple** additional input files:

    cft --input path/file-A.cft --input path/file-B.cft
    cft -ii path/file-A.cft path/file-B.cft
    cft --input ~/some/directory --recursive      # searches all files ending in *.cft recursively
    cft -ir ~/some/directory

It is also possible to merge tag files. The sources are `--input` and it will merge into `--output`.

    cft --merge -ii path/file-A.cft path/file-B.cft     # merge into default file
    cft --merge -oii path/output.cft path/file-A.cft path/file-B.cft

If one has filenames in their tags file, one can let **cft** expand the paths using `--expand`.

    cft --input tags.cft --expand --list-files
    cft -ieL tags.cft

    # this will expand based on the following rules
    filename    -> prepend directory of tags file, starting from root
    ~/filename  -> replace ~ with home directory
    ../filename -> prepend directory of tags file, starting from root. any '..' will also get replaced
    /filename   -> since already at root, do nothing (will still treat '..')

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

To add tags with the cli to **one output file**, one can do:

    cft -o path/output.cft string-to-tag --tag list,of,tags
    cft -o path/output.cft string-to-tag -t list,of,tags

### TODO so I won't forget
- undo? hmm
- -lL filename = wrong count of files in description!
- don't add -t tag: (empty subfolder) as tag
- clean up the code since it's messy -> especially since I switched from array of tags to lookup tables ...

