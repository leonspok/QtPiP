#ifndef WEBCONTENTWIDGET_H
#define WEBCONTENTWIDGET_H

#include <QWidget>
#include <QtWebEngineWidgets>

class WebContentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WebContentWidget(QWidget *parent = nullptr);

    void openUrl(QString urlString);
    void reset();

signals:

public slots:
    void fullscreenRequested(QWebEngineFullScreenRequest request);

private:
    QWebEngineView *webView;
};

#endif // WEBCONTENTWIDGET_H
