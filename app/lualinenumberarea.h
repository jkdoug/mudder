#ifndef LUALINENUMBERAREA_H
#define LUALINENUMBERAREA_H

#include <QPaintEvent>
#include <QSize>
#include <QWidget>

class LuaEdit;

class LuaLineNumberArea : public QWidget
{
    Q_OBJECT
public:
    explicit LuaLineNumberArea(LuaEdit *editor);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    LuaEdit *m_editor;
};

#endif // LUALINENUMBERAREA_H
