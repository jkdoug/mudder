/*
  Mudder, a cross-platform text gaming client

  Copyright (C) 2014 Jason Douglas
  jkdoug@gmail.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/


#include "consoledocumentlayout.h"
#include <QFontMetrics>
#include <QTextBlock>
#include "logging.h"

ConsoleDocumentLayout::ConsoleDocumentLayout(QTextDocument *doc) :
    QAbstractTextDocumentLayout(doc),
    m_width(0),
    m_maximumWidth(0),
    m_maximumWidthBlockNumber(0),
    m_blockCount(1),
    m_scroll(0)
{
}

ConsoleDocumentLayout::~ConsoleDocumentLayout()
{
}

void ConsoleDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
    QRectF r(context.clip);

    int y = r.height();

    QTextBlock textBlock(bottomBlock(context.cursorPosition));
    while (y > 0 && textBlock.isValid())
    {
        QTextLayout * textLayout = textBlock.layout();

        QVector<QTextLayout::FormatRange> selections;
        int blockPos = textBlock.position();
        int blockLen = textBlock.length();
        foreach (const QAbstractTextDocumentLayout::Selection &sel, context.selections)
        {
            const int selStart = sel.cursor.selectionStart() - blockPos;
            const int selEnd = sel.cursor.selectionEnd() - blockPos;

            if (selStart < blockLen && selEnd > 0 && selEnd > selStart)
            {
                QTextLayout::FormatRange o;
                o.start = selStart;
                o.length = selEnd - selStart;
                o.format = sel.format;
                selections.append(o);
            }
        }

        y -= blockBoundingRect(textBlock).height();

        textLayout->draw(painter, QPoint(0, y), selections);

        textBlock = textBlock.previous();
    }
}

int ConsoleDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    Q_UNUSED(accuracy)

    QTextBlock textBlock(bottomBlock());
    ensureBlockLayout(textBlock);
    if (!textBlock.isValid())
    {
        return -1;
    }

    QPointF offset;
    QRectF rectBlock(blockBoundingRect(textBlock));
    while (textBlock.isValid() && !rectBlock.contains(point))
    {
        offset.ry() += rectBlock.height();

        textBlock = textBlock.previous();
        rectBlock = blockBoundingRect(textBlock).adjusted(0, offset.y(), 0, offset.y());
    }

    if (!textBlock.isValid())
    {
        return -1;
    }

    int charOffset = 0;
    QPointF pos(point - offset);
    QTextLayout *textLayout = textBlock.layout();

    for (int n = 0; n < textLayout->lineCount(); n++)
    {
        QTextLine line(textLayout->lineAt(n));
        const QRectF lineRect(line.naturalTextRect());

        if (lineRect.top() > pos.y())
        {
            charOffset = qMax(charOffset, line.textStart() + line.textLength());
        }
        else if (lineRect.bottom() <= pos.y())
        {
            charOffset = qMin(charOffset, line.textStart());
        }
        else
        {
            charOffset = line.xToCursor(pos.x(), QTextLine::CursorBetweenCharacters);
            break;
        }
    }

    return textBlock.position() + charOffset;
}

int ConsoleDocumentLayout::pageCount() const
{
    return 1;
}

QSizeF ConsoleDocumentLayout::documentSize() const
{
    return QSizeF(m_maximumWidth, document()->lineCount());
}

QRectF ConsoleDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
    Q_UNUSED(frame)

    return QRectF(0, 0, qMax(m_width, m_maximumWidth), qreal(INT_MAX));
}


QRectF ConsoleDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    if (!block.isValid())
    {
        return QRectF();
    }

    QTextLayout *textLayout = block.layout();
    if (!textLayout->lineCount())
    {
        const_cast<ConsoleDocumentLayout*>(this)->layoutBlock(block);
    }

    QRectF boundingRect;
    if (block.isVisible())
    {
        boundingRect = QRectF(QPointF(0, 0), textLayout->boundingRect().bottomRight());

        if (textLayout->lineCount() == 1)
        {
            boundingRect.setWidth(qMax(boundingRect.width(), textLayout->lineAt(0).naturalTextWidth()));
        }

        qreal margin = document()->documentMargin();
        boundingRect.adjust(0, 0, margin, 0);

        if (!block.next().isValid())
        {
            boundingRect.adjust(0, 0, 0, margin);
        }
    }

    return boundingRect;
}

void ConsoleDocumentLayout::ensureBlockLayout(const QTextBlock &block) const
{
    if (!block.isValid())
    {
        return;
    }

    QTextLayout *textLayout = block.layout();
    if (!textLayout->lineCount())
    {
        const_cast<ConsoleDocumentLayout*>(this)->layoutBlock(block);
    }
}

void ConsoleDocumentLayout::setTextWidth(qreal width)
{
    m_width = width;
    m_maximumWidth = width;

    relayout();
}

void ConsoleDocumentLayout::documentChanged(int from, int charsRemoved, int charsAdded)
{
    Q_UNUSED(charsRemoved)

    QTextDocument *doc = document();
    int newBlockCount = doc->blockCount();

    QTextBlock changeStartBlock(doc->findBlock(from));
    QTextBlock changeEndBlock(doc->findBlock(qMax(0, from + charsAdded - 1)));

    QTextBlock block(changeStartBlock);
    if (changeStartBlock == changeEndBlock && newBlockCount == m_blockCount)
    {
        int blockLineCount = block.layout()->lineCount();
        if (block.isValid() && blockLineCount)
        {
            QRectF oldBoundingRect(blockBoundingRect(block));
            layoutBlock(block);

            QRectF newBoundingRect(blockBoundingRect(block));
            if (newBoundingRect.height() == oldBoundingRect.height())
            {
                return;
            }
        }
    }
    else
    {
        do
        {
            block.clearLayout();
            if (block == changeEndBlock)
            {
                break;
            }
            block = block.next();
        } while (block.isValid());
    }

    if (newBlockCount != m_blockCount)
    {
        int changeEnd = changeEndBlock.blockNumber();
        int blockDiff = newBlockCount - m_blockCount;
        int oldChangeEnd = changeEnd - blockDiff;

        if (m_maximumWidthBlockNumber > oldChangeEnd)
        {
            m_maximumWidthBlockNumber += blockDiff;
        }

        m_blockCount = newBlockCount;
        if (m_blockCount == 1)
        {
            m_maximumWidth = blockWidth(doc->firstBlock());
        }
    }
}

QTextBlock ConsoleDocumentLayout::bottomBlock(int scroll) const
{
    if (!document())
    {
        return QTextBlock();
    }

    QTextBlock block;
    if (scroll > 0)
    {
        m_scroll = scroll;
        block = document()->findBlockByNumber(m_scroll - 1);
    }
    else if (scroll < 0 && m_scroll > 0)
    {
        block = document()->findBlockByNumber(m_scroll - 1);
    }
    else
    {
        if (scroll == 0)
        {
            m_scroll = 0;
        }

        block = document()->lastBlock();
    }

    if (block.text().isEmpty())
    {
        return block.previous();
    }

    return block;
}

void ConsoleDocumentLayout::layoutBlock(const QTextBlock &block)
{
    QTextDocument *doc = document();
    qreal margin = doc->documentMargin();
    qreal blockMaximumWidth = 0;

    qreal height = 0;
    QTextLayout *textLayout = block.layout();
    QTextOption option = doc->defaultTextOption();
    textLayout->setTextOption(option);

    int extraMargin = 0;
    if (option.flags() & QTextOption::AddSpaceForLineAndParagraphSeparators)
    {
        QFontMetrics metrics(block.charFormat().font());
        extraMargin += metrics.width(QChar(0x21B5));
    }

    textLayout->beginLayout();
    qreal availableWidth = m_width;
    if (availableWidth <= 0)
    {
        availableWidth = qreal(INT_MAX);
    }
    availableWidth -= 2 * margin + extraMargin;

    while (true)
    {
        QTextLine line(textLayout->createLine());
        if (!line.isValid())
        {
            break;
        }

        line.setLeadingIncluded(true);
        line.setLineWidth(availableWidth);
        line.setPosition(QPointF(margin, height));

        height += line.height();

        blockMaximumWidth = qMax(blockMaximumWidth, line.naturalTextWidth() + 2 * margin);
    }
    textLayout->endLayout();

    const_cast<QTextBlock&>(block).setLineCount(block.isVisible() ? textLayout->lineCount() : 0);

    if (blockMaximumWidth > m_maximumWidth)
    {
        m_maximumWidth = blockMaximumWidth;
        m_maximumWidthBlockNumber = block.blockNumber();
    }
    else if (block.blockNumber() == m_maximumWidthBlockNumber && blockMaximumWidth < m_maximumWidth)
    {
        m_maximumWidth = 0;

        QTextBlock maximumBlock;
        QTextBlock b(doc->firstBlock());
        while (b.isValid())
        {
            qreal blockMaximumWidth = blockWidth(b);
            if (blockMaximumWidth > m_maximumWidth)
            {
                m_maximumWidth = blockMaximumWidth;
                maximumBlock = b;
            }
            b = b.next();
        }

        if (maximumBlock.isValid())
        {
            m_maximumWidthBlockNumber = maximumBlock.blockNumber();
        }
    }
}

qreal ConsoleDocumentLayout::blockWidth(const QTextBlock &block)
{
    QTextLayout *textLayout = block.layout();
    if (!textLayout->lineCount())
    {
        return 0;
    }

    qreal blockWidth = 0;
    for (int n = 0; n < textLayout->lineCount(); n++)
    {
        QTextLine line(textLayout->lineAt(n));
        blockWidth = qMax(line.naturalTextWidth() + 8, blockWidth);
    }

    return blockWidth;
}

void ConsoleDocumentLayout::relayout()
{
    QTextBlock block(document()->firstBlock());
    while (block.isValid())
    {
        block.layout()->clearLayout();
        block.setLineCount(block.isVisible() ? 1 : 0);
        block = block.next();
    }

    emit update();
}
