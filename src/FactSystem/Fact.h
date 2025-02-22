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
#include <QtCore/QString>
#include <QtCore/QVariant>

#include "FactMetaData.h"

class FactValueSliderListModel;

Q_DECLARE_LOGGING_CATEGORY(FactLog)

/// Used to hold a single value within the system.
class Fact : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int          componentId             READ componentId                                        CONSTANT)
    Q_PROPERTY(QStringList  bitmaskStrings          READ bitmaskStrings                                     NOTIFY bitmaskStringsChanged)
    Q_PROPERTY(QVariantList bitmaskValues           READ bitmaskValues                                      NOTIFY bitmaskValuesChanged)
    Q_PROPERTY(QStringList  selectedBitmaskStrings  READ selectedBitmaskStrings                             NOTIFY valueChanged)
    Q_PROPERTY(int          decimalPlaces           READ decimalPlaces                                      CONSTANT)
    Q_PROPERTY(QVariant     defaultValue            READ cookedDefaultValue                                 CONSTANT)
    Q_PROPERTY(QString      defaultValueString      READ cookedDefaultValueString                           CONSTANT)
    Q_PROPERTY(bool         defaultValueAvailable   READ defaultValueAvailable                              CONSTANT)
    Q_PROPERTY(int          enumIndex               READ enumIndex              WRITE setEnumIndex          NOTIFY valueChanged)
    Q_PROPERTY(QStringList  enumStrings             READ enumStrings                                        NOTIFY enumsChanged)
    Q_PROPERTY(QString      enumStringValue         READ enumStringValue        WRITE setEnumStringValue    NOTIFY valueChanged)
    Q_PROPERTY(QVariantList enumValues              READ enumValues                                         NOTIFY enumsChanged)
    Q_PROPERTY(QString      category                READ category                                           CONSTANT)
    Q_PROPERTY(QString      group                   READ group                                              CONSTANT)
    Q_PROPERTY(QString      longDescription         READ longDescription                                    CONSTANT)
    Q_PROPERTY(QVariant     max                     READ cookedMax                                          CONSTANT)
    Q_PROPERTY(QString      maxString               READ cookedMaxString                                    CONSTANT)
    Q_PROPERTY(bool         maxIsDefaultForType     READ maxIsDefaultForType                                CONSTANT)
    Q_PROPERTY(QVariant     min                     READ cookedMin                                          CONSTANT)
    Q_PROPERTY(QString      minString               READ cookedMinString                                    CONSTANT)
    Q_PROPERTY(bool         minIsDefaultForType     READ minIsDefaultForType                                CONSTANT)
    Q_PROPERTY(QString      name                    READ name                                               CONSTANT)
    Q_PROPERTY(bool         vehicleRebootRequired   READ vehicleRebootRequired                              CONSTANT)
    Q_PROPERTY(bool         qgcRebootRequired       READ qgcRebootRequired                                  CONSTANT)
    Q_PROPERTY(QString      shortDescription        READ shortDescription                                   CONSTANT)
    Q_PROPERTY(QString      units                   READ cookedUnits                                        CONSTANT)
    Q_PROPERTY(QVariant     value                   READ cookedValue            WRITE setCookedValue        NOTIFY valueChanged)
    Q_PROPERTY(QVariant     rawValue                READ rawValue               WRITE setRawValue           NOTIFY rawValueChanged)
    Q_PROPERTY(bool         valueEqualsDefault      READ valueEqualsDefault                                 NOTIFY valueChanged)
    Q_PROPERTY(QString      valueString             READ cookedValueString                                  NOTIFY valueChanged)
    Q_PROPERTY(QString      enumOrValueString       READ enumOrValueString                                  NOTIFY valueChanged)
    Q_PROPERTY(double       increment               READ cookedIncrement                                    CONSTANT)
    Q_PROPERTY(bool         typeIsString            READ typeIsString                                       CONSTANT)
    Q_PROPERTY(bool         typeIsBool              READ typeIsBool                                         CONSTANT)
    Q_PROPERTY(bool         hasControl              READ hasControl                                         CONSTANT)
    Q_PROPERTY(bool         readOnly                READ readOnly                                           CONSTANT)
    Q_PROPERTY(bool         writeOnly               READ writeOnly                                          CONSTANT)
    Q_PROPERTY(bool         volatileValue           READ volatileValue                                      CONSTANT)

public:
    Fact(QObject *parent = nullptr);
    Fact(int componentId, const QString &name, FactMetaData::ValueType_t type, QObject *parent = nullptr);
    Fact(const Fact &other, QObject *parent = nullptr);

    /// Creates a Fact using the name and type from metaData. Also calls QGCCorePlugin::adjustSettingsMetaData allowing
    /// custom builds to override the metadata.
    Fact(const QString &settingsGroup, FactMetaData *metaData, QObject *parent = nullptr);
    ~Fact();

    const Fact& operator=(const Fact& other);

    /// Convert and validate value
    ///     @param cookedValue: Value to convert and validate
    ///     @param convertOnly true: validate type conversion only, false: validate against meta data as well
    Q_INVOKABLE QString validate(const QString &cookedValue, bool convertOnly);
    /// Convert and clamp value
    Q_INVOKABLE QVariant clamp(const QString &cookedValue);

    QVariant cookedValue() const;   /// Value after translation
    QVariant rawValue() const { return _rawValue; }  /// value prior to translation, careful
    int componentId() const;
    int decimalPlaces() const;
    QVariant rawDefaultValue() const;
    QVariant cookedDefaultValue() const;
    bool defaultValueAvailable() const;
    QString cookedDefaultValueString() const;
    QStringList bitmaskStrings() const;
    QVariantList bitmaskValues() const;
    QStringList selectedBitmaskStrings() const;
    int enumIndex(); ///< This is not const, since an unknown value can modify the enum lists
    QStringList enumStrings() const;
    QString enumStringValue(); ///< This is not const, since an unknown value can modify the enum lists
    QVariantList enumValues() const;
    QString category() const;
    QString group() const;
    QString longDescription() const;
    QVariant rawMax() const;
    QVariant cookedMax() const;
    QString cookedMaxString() const;
    bool maxIsDefaultForType() const;
    QVariant rawMin() const;
    QVariant cookedMin() const;
    QString cookedMinString() const;
    bool minIsDefaultForType() const;
    QString name() const;
    QString shortDescription() const;
    FactMetaData::ValueType_ttype() const;
    QString cookedUnits() const;
    QString rawUnits() const;
    QString rawValueString() const;
    QString cookedValueString() const;
    bool valueEqualsDefault() const;
    bool vehicleRebootRequired() const;
    bool qgcRebootRequired() const;
    QString enumOrValueString(); ///< This is not const, since an unknown value can modify the enum lists
    double rawIncrement() const;
    double cookedIncrement() const;
    bool typeIsString() const { return type() == FactMetaData::valueTypeString; }
    bool typeIsBool() const { return type() == FactMetaData::valueTypeBool; }
    bool hasControl() const;
    bool readOnly() const;
    bool writeOnly() const;
    bool volatileValue() const;

    /// Internal hack to allow changes to fact which do not signal reboot. Currently used by font point size
    /// code in ScreenTools.qml to set initial sizing at first boot.
    Q_INVOKABLE void _setIgnoreQGCRebootRequired(bool ignore);

    Q_INVOKABLE FactValueSliderListModel *valueSliderModel();

    /// Returns the values as a string with full 18 digit precision if float/double.
    QString rawValueStringFullPrecision() const;

    void setRawValue(const QVariant &value);
    void setCookedValue(const QVariant &value);
    void setEnumIndex(int index);
    void setEnumStringValue(const QString &value);
    int valueIndex(const QString &value);

    /// The following methods allow you to defer sending of the valueChanged signals in order to implement
    /// rate limited signalling for ui performance. Used by FactGroup for example.
    void setSendValueChangedSignals(bool sendValueChangedSignals);
    bool sendValueChangedSignals() const { return _sendValueChangedSignals; }
    bool deferredValueChangeSignal() const { return _deferredValueChangeSignal; }
    void clearDeferredValueChangeSignal() { _deferredValueChangeSignal = false; }
    void sendDeferredValueChangedSignal();

    /// Sets and sends new value to vehicle even if value is the same
    void forceSetRawValue(const QVariant &value);

    /// Sets the meta data associated with the Fact.
    ///     @param metaData FactMetaData for Fact
    ///     @param setDefaultFromMetaData true: set the fact value to the default specified in the meta data
    void setMetaData(FactMetaData* metaData, bool setDefaultFromMetaData = false);

    FactMetaData* metaData() { return _metaData; }

    /// Value coming from Vehicle. This does NOT send a _containerRawValueChanged signal.
    void _containerSetRawValue(const QVariant &value);

    /// Generally you should not change the name of a fact. But if you know what you are doing, you can.
    void _setName(const QString &name) { _name = name; }

    /// Generally this is done during parsing. But if you know what you are doing, you can.
    void setEnumInfo(const QStringList &strings, const QVariantList &values);

signals:
    void bitmaskStringsChanged();
    void bitmaskValuesChanged();
    void enumsChanged();
    void sendValueChangedSignalsChanged(bool sendValueChangedSignals);

    /// This signal is only meant for use by the QT property system. It should not be connected to by client code.
    void valueChanged(QVariant value);
    void rawValueChanged(QVariant value);

    /// Signalled when the param write ack comes back from the vehicle
    void vehicleUpdated(QVariant value);

    /// Signalled when property has been changed by a call to the property write accessor
    /// This signal is meant for use by Fact container implementations. Used to send changed values to vehicle.
    void _containerRawValueChanged(const QVariant &value);

protected:
    QString _variantToString(const QVariant& variant, int decimalPlaces) const;
    void _sendValueChangedSignal(QVariant value);

    QString _name;
    int _componentId;
    QVariant _rawValue;
    FactMetaData::ValueType_t _type;
    FactMetaData *_metaData;
    bool _sendValueChangedSignals;
    bool _deferredValueChangeSignal;
    FactValueSliderListModel *_valueSliderModel;
    bool _ignoreQGCRebootRequired;

    static constexpr const char *kMissingMetadata = "Meta data pointer missing";

private slots:
    void _checkForRebootMessaging();

private:
    void _init();
};
