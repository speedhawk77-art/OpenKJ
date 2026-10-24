// Optional end-user Cisco binary integration; GPL-3.0-or-later (OpenKJ).
#include "optionalcodec.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QLibrary>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>
#ifdef Q_OS_WIN
#include <windows.h>
#include <winhttp.h>
#endif

namespace OptionalCodec {
static const char *dllHash="2076cb5675ec6c1a4c70e7a2a322552f547b6eeed649d6dfcd9e02a543b24691";
static QString root() {
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
        + "/OpenKJ/optional-codecs/openh264-2.6.0";
}
static QByteArray hash(const QByteArray &b) {return QCryptographicHash::hash(b,QCryptographicHash::Sha256).toHex();}
QString codecPath() { return root()+"/libopenh264-7.dll"; }
QString licenseText() {
    QFile f(":/optional-codec/BINARY_LICENSE.txt");
    return f.open(QIODevice::ReadOnly)?QString::fromUtf8(f.readAll()):QString();
}
bool verified() {
    QFile f(codecPath());
    return f.size()==978520 && f.open(QIODevice::ReadOnly) && hash(f.readAll())==dllHash;
}
bool enabled() {return QSettings("OpenKJ","OptionalCodecs").value("ciscoOpenH264Enabled",false).toBool();}
void setEnabled(bool on) {
    QSettings s("OpenKJ","OptionalCodecs");s.setValue("ciscoOpenH264Enabled",on);s.sync();
}
QByteArray download(QString *error) {
#ifdef Q_OS_WIN
    struct Handle { HINTERNET h; ~Handle(){if(h)WinHttpCloseHandle(h);} };
    Handle session{WinHttpOpen(L"OpenKJ-Optional-Codec/1.0",WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0)};
    auto fail=[&](){if(error)*error="The direct Cisco HTTPS download failed. Check your connection and try again.";return QByteArray();};
    if(!session.h)return fail();
    WinHttpSetTimeouts(session.h,10000,10000,30000,30000);
    Handle connection{WinHttpConnect(session.h,L"ciscobinary.openh264.org",INTERNET_DEFAULT_HTTPS_PORT,0)};
    if(!connection.h)return fail();
    Handle request{WinHttpOpenRequest(connection.h,L"GET",L"/openh264-2.6.0-win64.dll.bz2",
        nullptr,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE)};
    if(!request.h)return fail();
    DWORD policy=WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
    if(!WinHttpSetOption(request.h,WINHTTP_OPTION_REDIRECT_POLICY,&policy,sizeof(policy))
        || !WinHttpSendRequest(request.h,WINHTTP_NO_ADDITIONAL_HEADERS,0,WINHTTP_NO_REQUEST_DATA,0,0,0)
        || !WinHttpReceiveResponse(request.h,nullptr))return fail();
    DWORD status=0,size=sizeof(status);
    if(!WinHttpQueryHeaders(request.h,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,&status,&size,WINHTTP_NO_HEADER_INDEX) || status!=200)return fail();
    QByteArray data;char buffer[16384];DWORD read=0;
    for(;;){
        if(!WinHttpReadData(request.h,buffer,sizeof(buffer),&read))return fail();
        if(!read)break;
        if(data.size()+static_cast<int>(read)>2*1024*1024)return fail();
        data.append(buffer,static_cast<int>(read));
    }
    return data;
#else
    if(error)*error="This optional codec is for Windows x64.";
    return QByteArray();
#endif
}
bool installDownloaded(const QByteArray &compressed,QString *error) {
    auto fail=[&](const QString &text){if(error)*error=text;return false;};
    if(compressed.size()>2*1024*1024 || hash(compressed)!="dab5f2a872777f9a58b69bfa9fbcf20d9f82f2d6ec91383fd70bff49bd34ac9f")
        return fail("The Cisco download checksum did not match. Nothing was installed.");
#ifdef Q_OS_WIN
    QLibrary bz(QCoreApplication::applicationDirPath()+"/libbz2.dll");
    typedef int (*Decompress)(char*,unsigned int*,char*,unsigned int,int,int);
    auto decompress=reinterpret_cast<Decompress>(bz.resolve("BZ2_bzBuffToBuffDecompress"));
    if(!decompress)return fail("The packaged bzip2 decoder is unavailable.");
    QByteArray output(978520,Qt::Uninitialized);unsigned int length=static_cast<unsigned int>(output.size());
    QByteArray input=compressed;
    if(decompress(output.data(),&length,input.data(),static_cast<unsigned int>(input.size()),0,0)!=0
            || length!=978520 || hash(output)!=dllHash)
        return fail("The decoded Cisco binary did not match. Nothing was installed.");
    if(!QDir().mkpath(root()))return fail("Cannot create your optional codec directory.");
    QSaveFile file(codecPath());
    if(!file.open(QIODevice::WriteOnly) || file.write(output)!=output.size() || !file.commit())
        return fail("Cannot save the verified optional codec.");
    return true;
#else
    return fail("This optional download is for the Windows x64 package.");
#endif
}
void configureStartup() {
#ifdef Q_OS_WIN
    const bool active=enabled()&&verified();
    // Distinct registries prevent a disabled codec surviving via cached features.
    QDir().mkpath(root());
    qputenv("GST_REGISTRY_1_0",QDir::toNativeSeparators(root()+(active?"/registry-on.bin":"/registry-off.bin")).toUtf8());
    qputenv("GST_PLUGIN_PATH_1_0",QByteArray());
    qputenv("OPENKJ_OPTIONAL_CODEC_PATH",QByteArray());
    if(!active)return;
    const QString pluginDir=QCoreApplication::applicationDirPath()+"/optional-codecs/openh264";
    if(!QFile::exists(pluginDir+"/libgstopenh264.dll"))return;
    // Only the exact pinned bytes may be loaded; no writable-directory DLL search.
    static HMODULE codec=LoadLibraryExW(reinterpret_cast<LPCWSTR>(codecPath().utf16()),nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
    if(!codec)return;
    qputenv("OPENKJ_OPTIONAL_CODEC_PATH",codecPath().toUtf8());
    // The scanner is a separate process and also needs the end-user cache path.
    qputenv("PATH",QDir::toNativeSeparators(root()).toLocal8Bit()+";"+qgetenv("PATH"));
    qputenv("GST_PLUGIN_PATH_1_0",QDir::toNativeSeparators(pluginDir).toUtf8());
#endif
}
void showDialog(QWidget *parent) {
    QDialog dialog(parent);dialog.setWindowTitle("Optional H.264 video support");dialog.resize(710,600);
    QVBoxLayout layout(&dialog);
    QLabel description("OpenH264 Video Codec provided by Cisco Systems, Inc.\n\n"
        "This optional decoder is off by default. Download it directly from Cisco once, then use it offline. "
        "Other H.264 decoders may depend on your graphics hardware. Read the terms below before enabling. "
        "Your media and library are not uploaded. Changes take effect after restarting OpenKJ.",&dialog);
    description.setWordWrap(true);layout.addWidget(&description);
    QLabel status(&dialog);layout.addWidget(&status);
    QPlainTextEdit terms(&dialog);terms.setReadOnly(true);terms.setPlainText(licenseText());layout.addWidget(&terms);
    QDialogButtonBox buttons(&dialog);
    auto enableButton=buttons.addButton("Download and enable",QDialogButtonBox::ActionRole);
    auto disableButton=buttons.addButton("Disable",QDialogButtonBox::ActionRole);
    buttons.addButton(QDialogButtonBox::Close);layout.addWidget(&buttons);
    auto refresh=[&](){status.setText(QString("Saved setting: %1. Verified local download: %2. Restart to apply changes.")
            .arg(enabled()?"enabled":"disabled",verified()?"present":"absent"));
        enableButton->setText(verified()?"Enable downloaded codec":"Download and enable");
        enableButton->setEnabled(!terms.toPlainText().isEmpty());};
    refresh();
    struct DownloadResult { QByteArray bytes;QString error; };
    QFutureWatcher<DownloadResult> watcher;bool cancelled=false;
    QObject::connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    QObject::connect(disableButton,&QPushButton::clicked,&dialog,[&](){
        cancelled=true;setEnabled(false);refresh();
        if(watcher.isRunning())enableButton->setEnabled(false);
    });
    QObject::connect(enableButton,&QPushButton::clicked,&dialog,[&](){
        if(verified()){setEnabled(true);refresh();return;}
        enableButton->setEnabled(false);status.setText("Downloading the pinned binary directly from Cisco…");
        cancelled=false;
        watcher.setFuture(QtConcurrent::run([](){DownloadResult result;result.bytes=download(&result.error);return result;}));
    });
    QObject::connect(&watcher,&QFutureWatcher<DownloadResult>::finished,&dialog,[&](){
        if(cancelled){refresh();return;}
        auto result=watcher.result();
        if(result.error.isEmpty() && installDownloaded(result.bytes,&result.error))setEnabled(true);
        refresh();if(!result.error.isEmpty())status.setText(result.error);
    });
    dialog.exec();
    // Closing cancels installation; the bounded worker only downloads into memory.
    watcher.disconnect(&dialog);
}
}
