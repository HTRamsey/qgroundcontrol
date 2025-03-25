/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>

#include "FactPanelController.h"

class APMAirframeModel;
class APMAirframeType;
class QmlObjectListModel;

Q_DECLARE_LOGGING_CATEGORY(APMAirframeComponentControllerLog)

/// MVC Controller for APMAirframeComponent.qml.
class APMAirframeComponentController : public FactPanelController
{
    Q_OBJECT
    Q_MOC_INCLUDE("QmlObjectListModel.h")
    Q_PROPERTY(QmlObjectListModel *frameClassModel MEMBER _frameClassModel CONSTANT)

public:
    explicit APMAirframeComponentController(QObject *parent = nullptr);
    ~APMAirframeComponentController();


    Q_INVOKABLE void loadParameters(const QString &paramFile);

private slots:
    void _githubJsonDownloadComplete(const QString &remoteFile, const QString &localFile, const QString &errorMsg);
    void _paramFileDownloadComplete(const QString &remoteFile, const QString &localFile, const QString &errorMsg);

private:
    void _fillFrameClasses();
    void _loadParametersFromDownloadFile(const QString &downloadedParamFile);

    Fact *_frameClassFact = nullptr;
    Fact *_frameTypeFact = nullptr;
    QmlObjectListModel *_frameClassModel = nullptr;
};

/*===========================================================================*/

class APMFrameClass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString      name                    MEMBER _name                    CONSTANT)
    Q_PROPERTY(int          frameClass              MEMBER _frameClass              CONSTANT)
    Q_PROPERTY(int          frameType               READ   frameType                NOTIFY frameTypeChanged)
    Q_PROPERTY(QStringList  frameTypeEnumStrings    MEMBER _frameTypeEnumStrings    CONSTANT)
    Q_PROPERTY(QVariantList frameTypeEnumValues     MEMBER _frameTypeEnumValues     CONSTANT)
    Q_PROPERTY(int          defaultFrameType        MEMBER _defaultFrameType        CONSTANT)
    Q_PROPERTY(QString      imageResource           READ   imageResource            NOTIFY imageResourceChanged)
    Q_PROPERTY(QString      imageResourceDefault    MEMBER _imageResourceDefault    CONSTANT)
    Q_PROPERTY(bool         frameTypeSupported      MEMBER _frameTypeSupported      CONSTANT)

public:
    explicit APMFrameClass(const QString &name, bool copter, int frameClass, Fact *frameTypeFact, QObject *parent = nullptr);
    ~APMFrameClass();

    int frameType() const;
    QString imageResource() const;

    const QString _name;
    const bool _copter;
    QString _imageResource;
    QString _imageResourceDefault;
    int _frameClass;
    QStringList _frameTypeEnumStrings;
    QVariantList _frameTypeEnumValues;
    int _defaultFrameType = -1;
    bool _frameTypeSupported = false;

signals:
    void imageResourceChanged();
    void frameTypeChanged();

private:
    Fact *_frameTypeFact = nullptr;
};
