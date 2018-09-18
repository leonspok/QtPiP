#ifndef FINISHABLESIZEGRIP_H
#define FINISHABLESIZEGRIP_H

#include <QSizeGrip>

class FinishableSizeGrip : public QSizeGrip
{
    Q_OBJECT
public:
    explicit FinishableSizeGrip(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void resizeStarted();
    void resizeFinished();
};

#endif // FINISHABLESIZEGRIP_H
