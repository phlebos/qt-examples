#include <QCoreApplication>
#include <QDebug>
#include <QSslSocket>
#include <QSslCipher>
#include <QDateTime>

#include "connector.h"

struct ConnectorPrivate
{
    QString host;
    int port;
    QSslSocket *sock;
};

Connector::Connector( const QString &host, int port )
{
    d = new ConnectorPrivate;
    d->host = host;
    d->port = port;
    d->sock = 0;
}

Connector::~Connector()
{
    delete d;
}

void Connector::connectToHost()
{
    qDebug() << "Connecting...";

    d->sock = new QSslSocket(this);
    connect( d->sock, SIGNAL(encrypted()), this, SLOT(ready()) );
    connect( d->sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)) );
    connect( d->sock, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(sslError(const QList<QSslError> &)) );

    d->sock->connectToHostEncrypted( d->host, d->port );
}

void Connector::dumpCertificate( const QSslCertificate &cert )
{
    qDebug() << cert.toPem();

    qDebug() << "== Subject Info ==\b";
    qDebug() << "CommonName:\t\t" << cert.subjectInfo( QSslCertificate::CommonName );
    qDebug() << "Organization:\t\t" << cert.subjectInfo( QSslCertificate::Organization );
    qDebug() << "LocalityName:\t\t" << cert.subjectInfo( QSslCertificate::LocalityName );
    qDebug() << "OrganizationalUnitName:\t" << cert.subjectInfo( QSslCertificate::OrganizationalUnitName );
    qDebug() << "StateOrProvinceName:\t" << cert.subjectInfo( QSslCertificate::StateOrProvinceName );

    QMultiMap<QSsl::AlternateNameEntryType, QString> altNames = cert.alternateSubjectNames();
    if ( !altNames.isEmpty() ) {
        qDebug() << "Alternate Subject Names (DNS):";
        foreach (const QString &altName, altNames.values(QSsl::DnsEntry)) {
            qDebug() << altName;
        }

        qDebug() << "Alternate Subject Names (Email):";
        foreach (const QString &altName, altNames.values(QSsl::EmailEntry)) {
            qDebug() << altName;
        }
    }

    qDebug() << "\n== Issuer Info ==";
    qDebug() << "CommonName:\t\t" << cert.issuerInfo( QSslCertificate::CommonName );
    qDebug() << "Organization:\t\t" << cert.issuerInfo( QSslCertificate::Organization );
    qDebug() << "LocalityName:\t\t" << cert.issuerInfo( QSslCertificate::LocalityName );
    qDebug() << "OrganizationalUnitName:\t" << cert.issuerInfo( QSslCertificate::OrganizationalUnitName );
    qDebug() << "StateOrProvinceName:\t" << cert.issuerInfo( QSslCertificate::StateOrProvinceName );

    qDebug() << "\n== Certificate ==";
    //qDebug() << "Serial Number:\t\t" << cert.serialNumber(); // This seems buggy
    qDebug() << "Effective Date:\t\t" << cert.effectiveDate().toString();
    qDebug() << "Expiry Date:\t\t" << cert.expiryDate().toString();
    qDebug() << "Valid:\t\t\t" << (cert.isValid() ? "Yes" : "No");
}

void Connector::dumpCipher( const QSslCipher &cipher )
{
    qDebug() << "\n== Cipher ==";

    qDebug() << "Authentication:\t\t" << cipher.authenticationMethod();
    qDebug() << "Encryption:\t\t" << cipher.encryptionMethod();
    qDebug() << "Key Exchange:\t\t" << cipher.keyExchangeMethod();
    qDebug() << "Cipher Name:\t\t" << cipher.name();
    qDebug() << "Protocol:\t\t" <<  cipher.protocolString();
    qDebug() << "Supported Bits:\t\t" << cipher.supportedBits();
    qDebug() << "Used Bits:\t\t" << cipher.usedBits();
}

void Connector::ready()
{
    qDebug() << " === Peer Certificate ===";

    QSslCertificate cert = d->sock->peerCertificate();
    dumpCertificate( cert );

    QSslCipher cipher = d->sock->sessionCipher();
    dumpCipher( cipher );

    qDebug() << "Done";
    qApp->quit();
}

void Connector::socketError( QAbstractSocket::SocketError error )
{
    qDebug() << "Socket Error: " << d->sock->errorString() << " (" << int(error) << ")";
}

void Connector::sslError( const QList<QSslError> &errors )
{
    foreach( const QSslError &error, errors ) {
        qDebug() << "SSL Error: " << error.errorString();
    }

    // This is only used because we are interested in dumping all the info
    // do NOT use if you want real security (it introduces man-in-the-middle
    // attacks).
    d->sock->ignoreSslErrors();
}
