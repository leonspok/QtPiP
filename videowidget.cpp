#include "videowidget.h"
#include <stdexcept>
#include <iostream>

#include <QtGui/QOpenGLContext>
#include <QtCore/QMetaObject>

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod(reinterpret_cast<VideoWidget*>(ctx), "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}

VideoWidget::VideoWidget(QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{

    this->mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if (!this->mpv)
        throw std::runtime_error("could not create mpv context");

#ifdef QT_DEBUG
    mpv::qt::set_option_variant(this->mpv, "terminal", "yes");
#endif
    mpv::qt::set_option_variant(this->mpv, "msg-level", "all=v");
    mpv::qt::set_option_variant(this->mpv, "ytdl", "yes");
    if (mpv_initialize(this->mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Make use of the MPV_SUB_API_OPENGL_CB API.
    mpv::qt::set_option_variant(this->mpv, "vo", "opengl-cb");

    // Request hw decoding, just for testing.
    mpv::qt::set_option_variant(this->mpv, "hwdec", "auto");

    // Set youtube-dl best quality
    mpv::qt::set_option_variant(this->mpv, "ytdl-format", "bestvideo[height<=?720][fps>=?30][vcodec!=?vp9]+bestaudio/best");

    mpv::qt::set_option_variant(this->mpv, "display-fps", "60");
    mpv::qt::set_option_variant(this->mpv, "video-sync", "display-resample");

    this->mpv_gl = reinterpret_cast<mpv_opengl_cb_context *>(mpv_get_sub_api(this->mpv, MPV_SUB_API_OPENGL_CB));
    if (!this->mpv_gl)
        throw std::runtime_error("OpenGL not compiled in");
    mpv_opengl_cb_set_update_callback(this->mpv_gl, VideoWidget::on_update, reinterpret_cast<void *>(this));
    connect(this, &VideoWidget::frameSwapped, this, &VideoWidget::swapped);

    mpv_observe_property(this->mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(this->mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_set_wakeup_callback(this->mpv, wakeup, this);
}

VideoWidget::~VideoWidget()
{
    makeCurrent();
    if (this->mpv_gl)
        mpv_opengl_cb_set_update_callback(this->mpv_gl, nullptr, nullptr);
    // Until this call is done, we need to make sure the player remains
    // alive. This is done implicitly with the mpv::qt::Handle instance
    // in this class.
    mpv_opengl_cb_uninit_gl(this->mpv_gl);
}

void VideoWidget::openUrl(QString urlString)
{
    const QByteArray c_urlString = urlString.toUtf8();
    const char *args[] = {"loadfile", c_urlString.data(), nullptr};
    mpv_command_async(this->mpv, 0, args);
}

bool VideoWidget::isPlaying() {
    QVariant isPaused = mpv::qt::get_property_variant(this->mpv, "pause");
    return !isPaused.toBool();
}

void VideoWidget::play() {
    mpv::qt::set_property_variant(this->mpv, "pause", false);
}

void VideoWidget::pause() {
    mpv::qt::set_property_variant(this->mpv, "pause", true);
}

float VideoWidget::position() {
    return mpv::qt::get_property_variant(this->mpv, "time-pos").toFloat();
}

void VideoWidget::setPosition(float newPosition) {
    double duration = mpv::qt::get_property_variant(this->mpv, "duration").toDouble();
    double currentTime = duration * static_cast<double>(newPosition);
    mpv::qt::set_property_variant(this->mpv, "time-pos", currentTime);
}

void VideoWidget::initializeGL()
{
    int r = mpv_opengl_cb_init_gl(this->mpv_gl, nullptr, get_proc_address, nullptr);
    if (r < 0)
        throw std::runtime_error("could not initialize OpenGL");
}

void VideoWidget::paintGL()
{
    mpv_opengl_cb_draw(this->mpv_gl, static_cast<int>(defaultFramebufferObject()), width(), -height());
}

void VideoWidget::swapped()
{
    mpv_opengl_cb_report_flip(this->mpv_gl, 0);
}

void VideoWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (this->mpv) {
        mpv_event *event = mpv_wait_event(this->mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        this->handle_mpv_event(event);
    }
}

void VideoWidget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = reinterpret_cast<mpv_event_property *>(event->data);
        if (strcmp(prop->name, "pause") == 0) {
            if (prop->format == MPV_FORMAT_FLAG) {
                int paused = *reinterpret_cast<int *>(prop->data);
                emit playbackChanged(paused == 0);
            }
        } else if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double currentTime = *reinterpret_cast<double *>(prop->data);
                double totalTime = mpv::qt::get_property_variant(this->mpv, "duration").toDouble();
                if (totalTime > 0) {
                    emit this->positionChanged(static_cast<float>(currentTime /  totalTime));
                } else {
                    emit this->positionChanged(0);
                }
            }
        }
        break;
    }
    case MPV_EVENT_VIDEO_RECONFIG: {
        int64_t w, h;
        if (mpv_get_property(this->mpv, "dwidth", MPV_FORMAT_INT64, &w) >= 0 &&
            mpv_get_property(this->mpv, "dheight", MPV_FORMAT_INT64, &h) >= 0 &&
            w > 0 && h > 0)
        {
            int width = static_cast<int>(w);
            int height = static_cast<int>(h);
            QSize size = QSize(width, height);
            emit videoSizeChanged(size);
        }
        break;
    }
    default: ;
        // Ignore uninteresting or unknown events.
    }
}

// Make Qt invoke mpv_opengl_cb_draw() to draw a new/updated video frame.
void VideoWidget::maybeUpdate()
{
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's opengl-cb API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        swapped();
        doneCurrent();
    } else {
        update();
    }
}

void VideoWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod(reinterpret_cast<VideoWidget *>(ctx), "maybeUpdate");
}
