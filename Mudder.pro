TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    lua52 \
    core \
    logging \
    client

client.depends = lua52 \
                 core \
                 logging
