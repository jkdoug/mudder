#include "lualinenumberarea.h"
#include "luaedit.h"

LuaLineNumberArea::LuaLineNumberArea(LuaEdit *editor) :
    QWidget(editor)
{
    m_editor = editor;
}

QSize LuaLineNumberArea::sizeHint() const
{
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LuaLineNumberArea::paintEvent(QPaintEvent *event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}
