/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "labelitem.h"
#include <labelparser.h>
#include "labelrenderer.h"

#include <QDebug>
#include <QInputDialog>
#include <QGraphicsItem>
#include <QGraphicsScene>

namespace Kst {

LabelItem::LabelItem(View *parent, const QString& txt)
  : ViewItem(parent), _parsed(0), _text(txt) {
}


LabelItem::~LabelItem() {
  delete _parsed;
  _parsed = 0;
}


void LabelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  if (!_parsed) {
    _parsed = Label::parse(_text);
  }

  // We can do better here. - caching
  if (_parsed) {
    const qreal w = pen().widthF();
    painter->save();
    QRect box = rect().adjusted(w, w, -w, -w).toRect();
    QFont font;
    font.setPointSize(16);
    QFontMetrics fm(font);
    painter->translate(QPoint(box.x(), box.y() + fm.height()));
    Label::RenderContext rc(font.family(), font.pointSize(), painter);
    Label::renderLabel(rc, _parsed->chunk);

    // Make sure we have a rect for selection, movement, etc
    setRect(QRectF(box.x(), box.y(), rc.x, fm.height()));

    painter->restore();
  }

  QPen p = pen();
  setPen(Qt::NoPen);
  QGraphicsRectItem::paint(painter, option, widget);
  setPen(p);
}


void CreateLabelCommand::createItem() {
  bool ok;
  QString text = QInputDialog::getText(_view, tr("Kst: Create Label"), tr("Label:"), QLineEdit::Normal, QString::null, &ok);
  if (!ok || text.isEmpty()) {
    return;
  }

  _item = new LabelItem(_view, text);
  _view->setMouseMode(View::Create);
  _view->setCursor(Qt::IBeamCursor);

  //If the mouseMode is changed again before we're done with creation
  //delete ourself.
  connect(_view, SIGNAL(mouseModeChanged()), _item, SLOT(deleteLater()));
  connect(_view, SIGNAL(creationPolygonChanged(View::CreationEvent)),
          _item, SLOT(creationPolygonChanged(View::CreationEvent)));
  connect(_item, SIGNAL(creationComplete()), this, SLOT(creationComplete()));
  //If the item is interrupted while creating itself it will destroy itself
  //need to delete this too in response...
  connect(_item, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));

}


}

#include "labelitem.moc"

// vim: ts=2 sw=2 et
