#ifndef MUTEDOVERLAYWIDGET_H
#define MUTEDOVERLAYWIDGET_H

#include <QWidget>
#include <QPoint>

class MutedOverlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MutedOverlayWidget(QWidget *parent = nullptr);

    bool isMuted();
    void setMuted(bool muted);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool muted;
};

#endif // MUTEDOVERLAYWIDGET_H
