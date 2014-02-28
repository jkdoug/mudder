TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    lua52 \
    core \
    logging \
    editor \
    client

client.depends = lua52 \
                 core \
                 logging \
                 editor

editor.depends = core
