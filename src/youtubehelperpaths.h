// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QVariantMap>

namespace YouTubeHelpers {
inline bool usable(const QString &path) {
    const QFileInfo file(path);
    return file.isAbsolute() && file.isFile() && file.isReadable() && file.isExecutable();
}
inline QString bundled(const QString &program,const QString &applicationDir=QCoreApplication::applicationDirPath()) {
    const QString filename=program
#ifdef Q_OS_WIN
        + ".exe"
#endif
        ;
    const auto path=QDir(applicationDir).absoluteFilePath("helpers/youtube/"+filename);
    return usable(path)?path:QString();
}
inline QString resolve(const QString &key, const QString &program, const QVariantMap &options,
                       const QString &applicationDir = QCoreApplication::applicationDirPath()) {
    const QString saved=options.value(key).toString().trimmed();
    if (usable(saved)) return saved;
    const QString filename=program
#ifdef Q_OS_WIN
        + ".exe"
#endif
        ;
    const auto included=bundled(program,applicationDir);
    if (!included.isEmpty()) return included;
    // Preserve the existing adjacent-tool and PATH fallbacks for portable/custom installs.
    const QFileInfo downloader(options.value("downloader").toString());
    if (downloader.isAbsolute()) {
        const auto adjacent=downloader.absoluteDir().filePath(filename);
        if (usable(adjacent)) return adjacent;
    }
    const auto found=QStandardPaths::findExecutable(program);
    if (usable(found)) return found;
    return saved; // Keep an invalid custom path visible so the host can correct it.
}
}
