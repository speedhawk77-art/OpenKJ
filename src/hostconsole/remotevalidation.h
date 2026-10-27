// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <cmath>
namespace HostRemote {
inline bool integer(const QJsonValue &v, int low, int high, int &out) {
    if (!v.isDouble()) return false;
    const double n=v.toDouble();
    if (!std::isfinite(n) || std::floor(n)!=n || n<low || n>high) return false;
    out=static_cast<int>(n); return true;
}
inline bool text(const QString &v, int max) {
    if (v.trimmed().isEmpty() || v.size()>max) return false;
    for (auto ch:v) if (ch.isNull() || ch.category()==QChar::Other_Control) return false;
    return true;
}
inline QString revision(const QJsonValue &v) {
    QJsonArray array; array.append(v);
    return QString::fromLatin1(QCryptographicHash::hash(QJsonDocument(array).toJson(QJsonDocument::Compact),QCryptographicHash::Sha256).toHex());
}
inline QJsonObject error(const QString &message) { return {{"ok",false},{"error",message}}; }
inline QJsonObject done(const QString &message="Done") { return {{"ok",true},{"message",message}}; }
}
