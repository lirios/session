// SPDX-FileCopyrightText: 2021 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LIRI_QML_SESSION_SESSIONMANAGER_H
#define LIRI_QML_SESSION_SESSIONMANAGER_H

#include <QObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcSession)

class QmlSessionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool idle READ isIdle WRITE setIdle NOTIFY idleChanged)
public:
    QmlSessionManager(QObject *parent = nullptr);

    bool isIdle() const;
    void setIdle(bool value);

    Q_INVOKABLE void lock();
    Q_INVOKABLE void unlock();
    Q_INVOKABLE void setEnvironment(const QString &key, const QString &value);

Q_SIGNALS:
    void idleChanged(bool value);
    void idleInhibitRequested();
    void idleUninhibitRequested();
    void sessionLocked();
    void sessionUnlocked();

private:
    bool m_idle = false;
};

#endif // LIRI_QML_SESSION_SESSIONMANAGER_H
