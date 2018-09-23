#include "mutedoverlaywidget.h"
#include "iostream"

#include <QMouseEvent>
#include <QCursor>
#include <QStyleOption>
#include <QPainter>

MutedOverlayWidget::MutedOverlayWidget(QWidget *parent) : QWidget(parent)
{
    this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    this->setAttribute(Qt::WA_NoSystemBackground, true);

    this->muted = false;
}

bool MutedOverlayWidget::isMuted() {
    return this->muted;
}

void MutedOverlayWidget::setMuted(bool muted) {
    this->muted = muted;
    if (muted) {
        this->setStyleSheet("background-color: #404040; border: 2px solid #A0A0A0;");
    } else {
        this->setStyleSheet("background-color: transparent; border: none;");
    }
}

void MutedOverlayWidget::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    this->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
