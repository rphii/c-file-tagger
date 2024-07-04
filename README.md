# c-file-tagger

list all tags:

    ./cft

list all files associated with tag

    ./cft [tag] list

add files to tag:

    ./cft [tag] add [filename(s)]

remove files from tag:

    ./cft [tag] remove [filename(s)]


--- maybe do it like so?

    ./cft [--virtual]
    ./cft [filenames] --tag [tag,tag2] [--virtual]          # add tag and tag2 to files
    ./cft [filenames] --untag [tag,tag2] [--virtual]        # untag tag and tag2 from files
    ./cft [filenames] --copy [destination] [--virtual]      # copy all tags from filenams to destination
    ./cft [filenames] --link [anchor] [--virtual]           # linke all files to that of d
    ./cft [filenames] --remove [--virtual]                  # remove all tags: similar to rm
    ./cft [filenames] --move [destination] [--virtual]      # rename entries, keeping all their tags. similar to: cp
    ./cft --any [tag,tag2] [--virtual]                      # (*) show files tagged with either tag or tag2
    ./cft --and [tag,tag2] [--virtual]                      # (*) show files tagged with tag and tag2
    ./cft --not [--any/--and]                               # (*) show files not tagged with certain tags

technically I do not need virtual, since one can just make a tag oneself, called --virtual
or do I? if one wants to link to nonexisting files?

file format?

    "filename 1": "tag","tag2"
    "filename 2": "tag","tag3"
    "filename 3": "tag",
    "filename 4" -> "filename 2"                                # a link
    "filename 5": "tag" -> "filename 2","filename 3"            # a double link with custom tags



