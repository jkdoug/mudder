TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    lua52 \
    app

app.depends = lua52
