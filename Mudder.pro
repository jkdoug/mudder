TEMPLATE = subdirs

SUBDIRS += \
    lua52 \
    logging \
    core \
    editor \
    client

core.depends = logging

editor.depends = core \
                 logging

client.depends = lua52 \
                 core \
                 logging \
                 editor
