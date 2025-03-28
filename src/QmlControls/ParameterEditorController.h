/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "FactPanelController.h"
#include "FactMetaData.h"
#include "QmlObjectListModel.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtQmlIntegration/QtQmlIntegration>

Q_DECLARE_LOGGING_CATEGORY(ParameterEditorControllerLog)

class ParameterManager;

class ParameterEditorGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString              name    MEMBER  name        CONSTANT)
    Q_PROPERTY(QmlObjectListModel   *facts  READ    getFacts    CONSTANT)

public:
    explicit ParameterEditorGroup(QObject *parent = nullptr) : QObject(parent) { }

    QmlObjectListModel *getFacts(void) { return &facts; }

    int componentId = 0;
    QString name;
    QmlObjectListModel facts;
};

/*---------------------------------------------------------------------------*/

class ParameterEditorCategory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString              name    MEMBER  name        CONSTANT)
    Q_PROPERTY(QmlObjectListModel   *groups READ    getGroups   CONSTANT)
public:
    explicit ParameterEditorCategory(QObject *parent = nullptr) : QObject(parent) { }

    QmlObjectListModel *getGroups() { return &groups; }

    QString name;
    QmlObjectListModel groups;
    QMap<QString, ParameterEditorGroup*> mapGroupName2Group;
};

/*---------------------------------------------------------------------------*/

class ParameterEditorDiff : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int      componentId     MEMBER componentId      CONSTANT)
    Q_PROPERTY(QString  name            MEMBER name             CONSTANT)
    Q_PROPERTY(QString  fileValue       MEMBER fileValue        CONSTANT)
    Q_PROPERTY(QString  vehicleValue    MEMBER vehicleValue     CONSTANT)
    Q_PROPERTY(bool     noVehicleValue  MEMBER noVehicleValue   CONSTANT)
    Q_PROPERTY(QString  units           MEMBER units            CONSTANT)
    Q_PROPERTY(bool     load            MEMBER load             NOTIFY loadChanged)

public:
    explicit ParameterEditorDiff(QObject *parent = nullptr) : QObject(parent) { }

    int componentId = 0;
    QString name;
    FactMetaData::ValueType_t valueType;
    QString fileValue;
    QVariant fileValueVar;
    QString vehicleValue;
    bool noVehicleValue  = false;
    QString units;
    bool load = true;

signals:
    void loadChanged(bool load);
};

/*---------------------------------------------------------------------------*/

class ParameterEditorController : public FactPanelController
{
    Q_OBJECT
    // QML_ELEMENT
    Q_PROPERTY(QString              searchText              MEMBER _searchText                                          NOTIFY searchTextChanged)
    Q_PROPERTY(QmlObjectListModel   *categories             READ categories                                             CONSTANT)
    Q_PROPERTY(QObject              *currentCategory        READ currentCategory            WRITE setCurrentCategory    NOTIFY currentCategoryChanged)
    Q_PROPERTY(QObject              *currentGroup           READ currentGroup               WRITE setCurrentGroup       NOTIFY currentGroupChanged)
    Q_PROPERTY(QmlObjectListModel   *parameters             MEMBER _parameters                                          NOTIFY parametersChanged)
    Q_PROPERTY(bool                 showModifiedOnly        MEMBER _showModifiedOnly                                    NOTIFY showModifiedOnlyChanged)
    // These property are related to the diff associated with a load from file
    Q_PROPERTY(bool                 diffOtherVehicle        MEMBER _diffOtherVehicle                                    NOTIFY diffOtherVehicleChanged)
    Q_PROPERTY(bool                 diffMultipleComponents  MEMBER _diffMultipleComponents                              NOTIFY diffMultipleComponentsChanged)
    Q_PROPERTY(QmlObjectListModel   *diffList               READ diffList                                               CONSTANT)

public:
    explicit ParameterEditorController(QObject *parent = nullptr);
    ~ParameterEditorController();

    Q_INVOKABLE QStringList searchParameters(const QString &searchText, bool searchInName = true, bool searchInDescriptions = true);

    Q_INVOKABLE void saveToFile(const QString &filename);
    Q_INVOKABLE bool buildDiffFromFile(const QString &filename);
    Q_INVOKABLE void clearDiff();
    Q_INVOKABLE void sendDiff();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void resetAllToDefaults();
    Q_INVOKABLE void resetAllToVehicleConfiguration();

    QObject *currentCategory() { return _currentCategory; }
    QObject *currentGroup() { return _currentGroup; }
    QmlObjectListModel *categories() { return &_categories; }
    QmlObjectListModel *diffList() { return &_diffList; }
    void setCurrentCategory(QObject *currentCategory);
    void setCurrentGroup(QObject *currentGroup);

signals:
    void searchTextChanged(const QString &searchText);
    void currentCategoryChanged();
    void currentGroupChanged();
    void showModifiedOnlyChanged();
    void diffOtherVehicleChanged(bool diffOtherVehicle);
    void diffMultipleComponentsChanged(bool diffMultipleComponents);
    void parametersChanged();

private slots:
    void _currentCategoryChanged();
    void _currentGroupChanged();
    void _searchTextChanged();
    void _buildLists();
    void _buildListsForComponent(int compId);
    void _factAdded(int compId, Fact *fact);

private:
    bool _shouldShow(Fact *fact) const;
    void _performSearch();

    ParameterManager *_parameterMgr = nullptr;
    ParameterEditorCategory *_currentCategory = nullptr;
    ParameterEditorGroup *_currentGroup = nullptr;
    QString _searchText;
    QTimer _searchTimer;
    bool _showModifiedOnly = false;
    bool _diffOtherVehicle = false;
    bool _diffMultipleComponents = false;
    QmlObjectListModel _categories;
    QmlObjectListModel _diffList;
    QmlObjectListModel _searchParameters;
    QmlObjectListModel *_parameters = nullptr;
    QMap<QString, ParameterEditorCategory*> _mapCategoryName2Category;
};
