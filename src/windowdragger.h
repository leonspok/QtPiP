#ifndef WINDOWDRAGGER_H
#define WINDOWDRAGGER_H

#include <QWidget>
#include <QPoint>

class WindowDragger : public QWidget
{
    Q_OBJECT
public:
    explicit WindowDragger(QWidget *parent = nullptr);

signals:
    void draggedInDirection(QPoint direction);
    void draggingFinished();
    void onMouseHover();
    void onMouseLeave();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool isDragging;
    QPoint previousPosition;

    QWidget *getTopmostWidget();
};

#endif // WINDOWDRAGGER_H
