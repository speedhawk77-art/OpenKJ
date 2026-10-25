#ifndef SOFTWARERENDERVIDEOSINK_H
#define SOFTWARERENDERVIDEOSINK_H

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/video/video.h>

#include <QWidget>


class SoftwareRenderVideoSink : public QObject
{
    Q_OBJECT
    friend struct MaintenanceTestAccess;

private:

    struct SampleInfo
    {
        GstSample *sample;
        GstVideoFrame frame;
    };

    std::atomic<bool> m_active {false};
    std::atomic<bool> m_pendingRepaint {false};

    QWidget *m_surface;
    QImage m_buffer;

    void onSurfaceResized(const QSize &size);

    GstAppSink *m_appSink;
    GstCaps *m_videoCaps;

    static GstFlowReturn NewSampleCallback(GstAppSink *appsink, gpointer user_data);
    bool pullSampleAndDrawImage();
    static void cleanupFunction(void *info);
    static QImage imageFromSample(GstSample *sample);

signals:
    void newFrameAvailable();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

public:
    SoftwareRenderVideoSink(QWidget *surface);
    ~SoftwareRenderVideoSink();
    GstAppSink* getSink() { return m_appSink; }


};

#endif // SOFTWARERENDERVIDEOSINK_H
