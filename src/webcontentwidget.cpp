#include "webcontentwidget.h"

#include <QStackedLayout>

WebContentWidget::WebContentWidget(QWidget *parent) : QWidget(parent)
{
    this->webView = new QWebEngineView(this);
    this->webView->settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
//    this->webView->settings()->setAttribute(QWebEngineSettings::JavascriptCanPaste, true);
//    this->webView->settings()->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    this->webView->settings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, true);

    connect(this->webView->page(), &QWebEnginePage::fullScreenRequested, this, &WebContentWidget::fullscreenRequested);

    QStackedLayout *layout = new QStackedLayout();
    layout->addWidget(this->webView);
    this->setLayout(layout);
}

void WebContentWidget::openUrl(QString urlString) {
    QUrl url = QUrl(urlString);
    this->webView->setUrl(url);
}

void WebContentWidget::reset() {
    this->webView->setUrl(QUrl("about:blank"));
}

void WebContentWidget::fullscreenRequested(QWebEngineFullScreenRequest request) {
    request.accept();
}
