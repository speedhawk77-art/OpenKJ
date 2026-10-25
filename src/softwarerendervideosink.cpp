#include "softwarerendervideosink.h"
#include <QObject>
#include <QPainter>
#include <QResizeEvent>
#include <limits>

SoftwareRenderVideoSink::SoftwareRenderVideoSink(QWidget *surface)
{
    m_surface = surface;

    m_appSink = (GstAppSink*)gst_element_factory_make("appsink", nullptr);
    gst_object_ref_sink(m_appSink);

    GstAppSinkCallbacks appsinkCallbacks;
    memset(&appsinkCallbacks, 0, sizeof(GstAppSinkCallbacks));
    appsinkCallbacks.new_sample = &SoftwareRenderVideoSink::NewSampleCallback;
    gst_app_sink_set_callbacks(m_appSink, &appsinkCallbacks, this, nullptr);

    // Formats we can handle:
    m_videoCaps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "RGB16", NULL);   // QImage::Format_RGB16
    gst_caps_append(m_videoCaps, gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGRx", NULL));  // QImage::Format_RGB32
    gst_caps_set_simple(m_videoCaps, "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, nullptr);
    g_object_set(m_appSink, "caps", m_videoCaps, NULL);

    // Only store one frame and drop old ones
    gst_app_sink_set_max_buffers (m_appSink, 1);
    gst_app_sink_set_drop(m_appSink, true);

    // Process eos even if there are unread samples.
    // As samples are only read when the widget is painted, samples are not read if the widget is hidden.
    // Without this, the sink will hang/never change state from playing->ready->null.
    gst_app_sink_set_wait_on_eos(m_appSink, false);

    // Allow the sink to send qos messages to pipeline to allow skipping frames
    gst_base_sink_set_qos_enabled(reinterpret_cast<GstBaseSink*>(m_appSink), true);

    m_surface->installEventFilter(this);

    connect(this, SIGNAL(newFrameAvailable()), m_surface, SLOT(update()), Qt::QueuedConnection);
}

SoftwareRenderVideoSink::~SoftwareRenderVideoSink()
{
    m_surface->removeEventFilter(this);
    g_object_unref(m_appSink);
    gst_caps_unref(m_videoCaps);
    m_appSink = nullptr;
}

bool SoftwareRenderVideoSink::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Paint)
    {
        m_pendingRepaint = false;
        if (m_active)
        {
            return pullSampleAndDrawImage();
        }
        else
        {
            return false;
        }
    }
    else if (event->type() == QEvent::Resize)
    {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
        onSurfaceResized(resizeEvent->size());
        return false;
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }
}

void SoftwareRenderVideoSink::onSurfaceResized(const QSize &size)
{
    // Tell what image dimension we can handle and let
    // the Videoscale element earlier in the pipeline do the actual scaline.
    gst_caps_set_simple(m_videoCaps, "width", G_TYPE_INT, size.width(), "height", G_TYPE_INT, size.height(), nullptr);
    gst_app_sink_set_caps(m_appSink, m_videoCaps);
    gst_element_send_event(GST_ELEMENT(m_appSink), gst_event_new_reconfigure());
}

GstFlowReturn SoftwareRenderVideoSink::NewSampleCallback([[maybe_unused]]GstAppSink *appsink, gpointer user_data)
{
    SoftwareRenderVideoSink *me = (SoftwareRenderVideoSink*) user_data;
    me->m_active = true;
    if (!me->m_pendingRepaint)
    {
        me->m_pendingRepaint = true;
        emit me->newFrameAvailable();
    }

    return GST_FLOW_OK;
}

void SoftwareRenderVideoSink::cleanupFunction(void* _info)
{
    SampleInfo *info = static_cast<SampleInfo*>(_info);

    gst_video_frame_unmap(&info->frame);
    gst_sample_unref(info->sample);
    delete info;
}

QImage SoftwareRenderVideoSink::imageFromSample(GstSample *sample)
{
    if (!sample)
        return {};
    GstVideoInfo videoInfo{};
    auto *caps = gst_sample_get_caps(sample);
    auto *buffer = gst_sample_get_buffer(sample);
    if (!caps || !buffer || !gst_video_info_from_caps(&videoInfo, caps)) {
        gst_sample_unref(sample);
        return {};
    }
    QImage::Format format = QImage::Format_Invalid;
    int bytesPerPixel = 0;
    switch (GST_VIDEO_INFO_FORMAT(&videoInfo)) {
        case GST_VIDEO_FORMAT_RGB16: format = QImage::Format_RGB16; bytesPerPixel = 2; break;
        case GST_VIDEO_FORMAT_BGRx: format = QImage::Format_RGB32; bytesPerPixel = 4; break;
        default: break;
    }
    const int width = GST_VIDEO_INFO_WIDTH(&videoInfo);
    const int height = GST_VIDEO_INFO_HEIGHT(&videoInfo);
    if (format == QImage::Format_Invalid || width <= 0 || height <= 0 ||
        width > (std::numeric_limits<int>::max)() / bytesPerPixel) {
        gst_sample_unref(sample);
        return {};
    }
    auto *info = new SampleInfo{};
    info->sample = sample;
    if (!gst_video_frame_map(&info->frame, &videoInfo, buffer, GST_MAP_READ)) {
        gst_sample_unref(sample);
        delete info;
        return {};
    }
    const int stride = GST_VIDEO_FRAME_PLANE_STRIDE(&info->frame, 0);
    if (stride < width * bytesPerPixel) {
        cleanupFunction(info);
        return {};
    }
    // Hold the mapped frame and its sample until the last QImage copy is released.
    return QImage(static_cast<const uchar *>(GST_VIDEO_FRAME_PLANE_DATA(&info->frame, 0)),
                  width, height, stride, format, cleanupFunction, info);
}

bool SoftwareRenderVideoSink::pullSampleAndDrawImage()
{
    // Pull sample and paint it. Must be called from gui thread!
    GstSample* sample = gst_app_sink_try_pull_sample(m_appSink, 0);

    if (sample)
    {
        const auto frame = imageFromSample(sample);
        if (frame.isNull())
            return false;
        m_buffer = frame;
    }
    else
    {
        // No sample found in queue - are we still playing?
        GstState state = GST_STATE_NULL;
        gst_element_get_state(reinterpret_cast<GstElement*>(m_appSink), &state, nullptr, 0);

        if (state == GST_STATE_NULL)
        {
            m_active = false;
            m_buffer = QImage();
        }
    }

    if (!m_buffer.isNull())
    {
        QPainter painter(m_surface);
        painter.drawImage(m_surface->contentsRect(), m_buffer, m_buffer.rect());
        return true;
    }

    return false;
}
