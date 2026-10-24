#pragma once
#include <QString>
#include <QByteArray>
class QWidget;

namespace OptionalCodec {
// Configure the optional Windows decoder before any GStreamer initialization.
void configureStartup();
void showDialog(QWidget *parent);
QString licenseText();
QString codecPath();
bool verified();
bool enabled();
void setEnabled(bool enabled);
// Pure verification/decompression boundary shared by the UI and CI harness.
QByteArray download(QString *error);
bool installDownloaded(const QByteArray &compressed, QString *error);
}
