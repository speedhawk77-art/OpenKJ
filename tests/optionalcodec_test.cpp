#include "optionalcodec.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QTemporaryDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>

int main(int argc,char **argv) {
    QApplication app(argc,argv);
    QTemporaryDir settings;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settings.path());
    QJsonArray checks;bool all=true;
    auto check=[&](const char *name,bool ok){checks.append(QJsonObject{{"name",name},{"passed",ok}});all&=ok;};
    check("disabled_by_default",!OptionalCodec::enabled());
    check("full_binary_license_embedded",OptionalCodec::licenseText().contains("AVC/H.264 Patent Portfolio License Conditions")
          && OptionalCodec::licenseText().contains("Any third-party software that makes use"));
    QString error;
    check("reject_empty_download",!OptionalCodec::installDownloaded(QByteArray(),&error));
    check("reject_oversized_download",!OptionalCodec::installDownloaded(QByteArray(2097153,'x'),&error));
    check("reject_wrong_download_hash",!OptionalCodec::installDownloaded(QByteArray("not-a-codec"),&error));
    error.clear();auto compressed=OptionalCodec::download(&error);
    check("direct_cisco_https_download",error.isEmpty()&&!compressed.isEmpty());
    error.clear();check("verified_download_decompress_and_atomic_install",OptionalCodec::installDownloaded(compressed,&error));
    check("installed_exact_pinned_binary",OptionalCodec::verified());
    OptionalCodec::setEnabled(true);check("enable_persists",OptionalCodec::enabled());
    OptionalCodec::configureStartup();
    check("enabled_optional_plugin_path",qgetenv("GST_PLUGIN_PATH_1_0").contains("optional-codecs"));
    OptionalCodec::setEnabled(false);check("disable_persists",!OptionalCodec::enabled());
    OptionalCodec::configureStartup();check("disabled_plugin_path_empty",qgetenv("GST_PLUGIN_PATH_1_0").isEmpty());
    OptionalCodec::setEnabled(true);check("reenable_downloaded_codec",OptionalCodec::enabled()&&OptionalCodec::verified());
    QFile corrupted(OptionalCodec::codecPath());
    if(corrupted.open(QIODevice::WriteOnly|QIODevice::Truncate)){corrupted.write("tampered");corrupted.close();}
    check("reject_tampered_cached_codec",!OptionalCodec::verified());
    OptionalCodec::configureStartup();check("tampered_codec_cannot_enable_plugin_path",qgetenv("GST_PLUGIN_PATH_1_0").isEmpty());
    OptionalCodec::setEnabled(false);corrupted.remove();
    QJsonObject result{{"checks",checks},{"passed",all},{"binary_distribution_approved",false}};
    auto output=QJsonDocument(result).toJson();
    QFile report("optional-codec-test-evidence.json");report.open(QIODevice::WriteOnly);report.write(output);
    std::fwrite(output.constData(),1,output.size(),stdout);
    return all?0:1;
}
