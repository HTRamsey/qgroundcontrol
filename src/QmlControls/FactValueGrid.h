/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QSettings>
#include <QtQuick/QQuickItem>

#include "QGCMAVLink.h"
#include "QmlObjectListModel.h"
#include "Vehicle.h"

class InstrumentValueData;

class FactValueGrid : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QmlObjectListModel   *columns                MEMBER _columns                                     NOTIFY columnsChanged)
    Q_PROPERTY(int                  rowCount                MEMBER _rowCount                                    NOTIFY rowCountChanged)
    Q_PROPERTY(QString              userSettingsGroup       MEMBER _userSettingsGroup                           NOTIFY userSettingsGroupChanged)
    Q_PROPERTY(QString              defaultSettingsGroup    MEMBER _defaultSettingsGroup                        NOTIFY defaultSettingsGroupChanged)
    Q_PROPERTY(QStringList          iconNames               READ iconNames                                      CONSTANT)
    Q_PROPERTY(FontSize             fontSize                READ fontSize                   WRITE setFontSize   NOTIFY fontSizeChanged)
    Q_PROPERTY(QStringList          fontSizeNames           MEMBER _fontSizeNames                               CONSTANT)
    Q_PROPERTY(Vehicle              *vehicle                READ vehicle                    WRITE setVehicle    NOTIFY vehicleChanged               REQUIRED)

public:
    explicit FactValueGrid(QQuickItem *parent = nullptr);
    FactValueGrid(const QString &defaultSettingsGroup);
    virtual ~FactValueGrid();

    enum FontSize {
        DefaultFontSize=0,
        SmallFontSize,
        MediumFontSize,
        LargeFontSize,
    };
    Q_ENUMS(FontSize)

    Q_INVOKABLE void resetToDefaults();
    Q_INVOKABLE QmlObjectListModel *appendColumn();
    Q_INVOKABLE void deleteLastColumn();
    Q_INVOKABLE void appendRow();
    Q_INVOKABLE void deleteLastRow();

    QmlObjectListModel *columns() const { return _columns; }
    QString userSettingsGroup() const { return _userSettingsGroup; }
    FontSize fontSize() const { return _fontSize; }
    QStringList iconNames() const { return _iconNames; }
    QGCMAVLink::VehicleClass_t vehicleClass() const { return _vehicleClass; }
    Vehicle *vehicle() const { return _vehicle; }

    void setFontSize(FontSize fontSize);
    void setVehicle(Vehicle *vehicle);

    // This is only exposed for usage of FactValueGrid to be able to just read the settings and display no ui. For this case
    // create a FactValueGrid object with a null parent. Set the userSettingsGroup/defaultSettingsGroup appropriately and then
    // call _loadSettings. Then after that you can read the settings from the object. You should not change any of the values.
    // Destroy the FactValueGrid object when done.
    void _loadSettings();

    void saveSettingsForced();

    // Override from QQmlParserStatus
    void componentComplete() final;

signals:
    void userSettingsGroupChanged(const QString &userSettingsGroup);
    void defaultSettingsGroupChanged(const QString &defaultSettingsGroup);
    void fontSizeChanged(FontSize fontSize);
    void vehicleChanged();
    void columnsChanged(QmlObjectListModel *model);
    void rowCountChanged(int rowCount);

protected:
    QGCMAVLink::VehicleClass_t _vehicleClass = QGCMAVLink::VehicleClassGeneric;
    // The combination of the two valuePage*SettingsGroup values allows each FactValueGrid to have it's own persistence space.
    // This is the setting group name for default settings which are used when the user has not modified anything from the default setup. These settings will be overwritten
    // prior to each use by the call to QGCCorePlugin::FactValueGridCreateDefaultSettings.
    QString _defaultSettingsGroup; // Settings group to read from if the user has not modified from the default settings
    // This is the settings group name for user modified settings. Settings will be saved to here whenever the user modified anything. Also at that point in time the
    // defaults settings group will be removed.
    QString _userSettingsGroup;    // Settings group to read from for user modified settings
    FontSize _fontSize = DefaultFontSize;
    bool _preventSaveSettings = false;
    QmlObjectListModel *_columns = nullptr;
    int _rowCount = 0;
    Vehicle *_vehicle = nullptr;

private slots:
    void _offlineVehicleTypeChanged();

private:
    InstrumentValueData *_createNewInstrumentValueWorker(QObject *parent);
    void _saveSettings();
    void _init();
    void _connectSaveSignals(InstrumentValueData *value);
    QString _pascalCase(const QString &text);
    void _saveValueData(QSettings &settings, InstrumentValueData *value);
    void _loadValueData(QSettings &settings, InstrumentValueData *value);
    QString _settingsKey();

    // These are user facing string for the various enums.
    static QStringList _iconNames;
    // Important: The indices of these strings must match the FactValueGrid::FontSize enum
    const QStringList _fontSizeNames = {
        QT_TRANSLATE_NOOP("FactValueGrid", "Default"),
        QT_TRANSLATE_NOOP("FactValueGrid", "Small"),
        QT_TRANSLATE_NOOP("FactValueGrid", "Medium"),
        QT_TRANSLATE_NOOP("FactValueGrid", "Large"),
    };

    static constexpr const char *_columnsKey = "columns";
    static constexpr const char *_rowsKey = "rows";
    static constexpr const char *_rowCountKey = "rowCount";
    static constexpr const char *_fontSizeKey = "fontSize";
    static constexpr const char *_versionKey = "version";
    static constexpr const char *_factGroupNameKey = "factGroupName";
    static constexpr const char *_factNameKey = "factName";
    static constexpr const char *_textKey = "text";
    static constexpr const char *_showUnitsKey = "showUnits";
    static constexpr const char *_iconKey = "icon";
    static constexpr const char *_rangeTypeKey = "rangeType";
    static constexpr const char *_rangeValuesKey = "rangeValues";
    static constexpr const char *_rangeColorsKey = "rangeColors";
    static constexpr const char *_rangeIconsKey = "rangeIcons";
    static constexpr const char *_rangeOpacitiesKey = "rangeOpacities";

    static constexpr const char *_deprecatedGroupKey = "ValuesWidget";

    // Static list of all instances. Used to notify others when settings have changed.
    static QList<FactValueGrid*> &instances() {
        static QList<FactValueGrid*> instanceList;
        return instanceList;
    }

    Q_DISABLE_COPY(FactValueGrid)
};
QML_DECLARE_TYPE(FactValueGrid)
Q_DECLARE_METATYPE(FactValueGrid::FontSize)
