// The MIT License (MIT)
//
// Copyright (c) Itay Grudev 2015 - 2016
// Copyright (c) Ildar Gilmanov 2020
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//
//  W A R N I N G !!!
//  -----------------
//
// This file is not part of the SingleApplication API. It is used purely as an
// implementation detail. This header file may change from version to
// version without notice, or may even be removed.
//

#ifndef SINGLEAPPLICATION_P_H
#define SINGLEAPPLICATION_P_H

#include <QSharedMemory>
#include <QLocalServer>
#include <QLocalSocket>

#include "SingleApplication.h"

//TODO: future: refactor SingleApplicationPrivate class

struct InstancesInfo
{
    explicit InstancesInfo() = default;

    static constexpr int primaryUserSize = 128;

    bool m_primary = false;
    quint32 m_secondary = 0;
    qint64 m_primaryPid = -1;
    quint16 m_checksum = 0;
    char m_primaryUser[primaryUserSize] = {};
};

class SingleApplicationPrivate : public QObject
{
    Q_OBJECT

public:
    enum class ConnectionType : quint8
    {
        InvalidConnection = 0,
        NewInstance = 1,
        SecondaryInstance = 2,
        Reconnect = 3
    };

    enum class ConnectionStage : quint8
    {
        Header = 0,
        Body = 1,
        Connected = 2,
    };

    struct ConnectionInfo
    {
        explicit ConnectionInfo() = default;

        qint64 m_messageLength = 0;
        quint32 m_instanceId = 0;
        ConnectionStage m_stage = ConnectionStage::Header;
    };

    Q_DECLARE_PUBLIC(SingleApplication)

    explicit SingleApplicationPrivate(SingleApplication *parent);
    ~SingleApplicationPrivate() override;

    QByteArray getUsername();
    void generateBlockServerName();
    void initializeMemoryBlock();
    void startPrimary();
    void startSecondary();
    void connectToPrimary(std::chrono::milliseconds timeout, ConnectionType connectionType);
    quint16 blockChecksum();
    qint64 primaryPid();
    QString primaryUser();
    void readInitMessageHeader(QLocalSocket *socket);
    void readInitMessageBody(QLocalSocket *socket);

    SingleApplication * const q_ptr;
    QSharedMemory *m_memory = nullptr;
    QLocalSocket *m_socket = nullptr;
    QLocalServer *m_server = nullptr;
    quint32 m_instanceNumber = -1;
    QString m_blockServerName;
    SingleApplication::Options m_options = {};
    QMap<QLocalSocket*, ConnectionInfo> m_connectionMap;

    void onDataAvailable(QLocalSocket *socket, quint32 instanceId);
    void onClientConnectionClosed(QLocalSocket *socket, quint32 instanceId);

public slots:
    void onConnectionEstablished();
};

#endif // SINGLEAPPLICATION_P_H
