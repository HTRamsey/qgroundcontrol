/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCLoggingCategory.h"

#include <QtCore/QGlobalStatic>
#include <QtCore/QSettings>

QGC_LOGGING_CATEGORY(QGCLoggingCategoryRegisterLog, "Utilities.QGCLoggingCategoryManager")

Q_GLOBAL_STATIC(QGCLoggingCategoryManager, _QGCLoggingCategoryManagerInstance);

QGCLoggingCategoryManager *QGCLoggingCategoryManager::instance()
{
    return _QGCLoggingCategoryManagerInstance();
}

void QGCLoggingCategoryManager::_insertSorted(QmlObjectListModel* model, QGCLoggingCategoryItem* item)
{
    for (int i=0; i<model->count(); i++) {
        auto existingItem = qobject_cast<QGCLoggingCategoryItem*>(model->get(i));
        if (item->fullCategory < existingItem->fullCategory) {
            model->insert(i, item);
            return;
        }
    }
    model->append(item);
}

void QGCLoggingCategoryManager::registerCategory(const QString &fullCategory)
{
    // Avoid duplicate registration
    if (_categoryLookup.contains(fullCategory)) {
        return;
    }

    QString parentCategory;
    QString childCategory(fullCategory);
    auto currentParentModel = &_treeCategoryModel;

    const int hierarchyIndex = fullCategory.indexOf(".");
    if (hierarchyIndex != -1) {
        parentCategory = fullCategory.left(hierarchyIndex);
        childCategory = fullCategory.mid(hierarchyIndex + 1);
        const QString fullParentCategory = parentCategory + ".";

        // Check if parent already exists in lookup
        QGCLoggingCategoryItem* parentItem = _categoryLookup.value(fullParentCategory, nullptr);
        if (parentItem && parentItem->children) {
            currentParentModel = parentItem->children;
        } else {
            // Create new parent
            auto newParentItem = new QGCLoggingCategoryItem(parentCategory, fullParentCategory, false /* enabled */, currentParentModel);
            newParentItem->children = new QmlObjectListModel(newParentItem);
            _insertSorted(&_flatCategoryModel, newParentItem);
            _insertSorted(currentParentModel, newParentItem);
            _categoryLookup.insert(fullParentCategory, newParentItem);
            currentParentModel = newParentItem->children;
        }
    }

    auto categoryItem = new QGCLoggingCategoryItem(childCategory, fullCategory, false /* enabled */, currentParentModel);
    _insertSorted(&_flatCategoryModel, categoryItem);
    _insertSorted(currentParentModel, categoryItem);
    _categoryLookup.insert(fullCategory, categoryItem);
}

void QGCLoggingCategoryManager::setCategoryLoggingOn(const QString &fullCategoryName, bool enable)
{
    qCDebug(QGCLoggingCategoryRegisterLog) << "Set category logging" << fullCategoryName << enable;
    
    QSettings settings;
    settings.beginGroup(kFilterRulesSettingsGroup);
    if (enable) {
        settings.setValue(fullCategoryName, enable);
    } else {
        settings.remove(fullCategoryName);
    }

    setFilterRulesFromSettings(QString());
}

bool QGCLoggingCategoryManager::categoryLoggingOn(const QString &fullCategoryName)
{
    QSettings settings;

    settings.beginGroup(kFilterRulesSettingsGroup);
    return settings.value(fullCategoryName, false).toBool();
}

void QGCLoggingCategoryManager::setFilterRulesFromSettings(const QString &commandLineLoggingOptions)
{
    QString filterRules;
    QString filterRuleFormat("%1.debug=true\n");

    QSettings settings;
    settings.beginGroup(kFilterRulesSettingsGroup);
    for (const QString &fullCategoryName : settings.childKeys()) {
        QString parentCategory;
        QString childCategory;
        _splitFullCategoryName(fullCategoryName, parentCategory, childCategory);

        qCDebug(QGCLoggingCategoryRegisterLog) << "Setting filter rule for saved settings" << fullCategoryName << parentCategory << childCategory << settings.value(fullCategoryName).toBool();

        auto categoryItem = _findLoggingCategory(fullCategoryName);
        if (categoryItem) {
            categoryItem->setEnabled(settings.value(fullCategoryName).toBool());
            if (categoryItem->enabled()) {
                if (childCategory.isEmpty()) {
                    // Wildcard parent category
                    filterRules += filterRuleFormat.arg(fullCategoryName + "*");
                } else {
                    filterRules += filterRuleFormat.arg(fullCategoryName);
                }
            }
        } else {
            qCWarning(QGCLoggingCategoryRegisterLog) << "Category not found for saved settings" << fullCategoryName;
        }
    }

    // Command line rules take precedence, so they go last in the list
    if (!commandLineLoggingOptions.isEmpty()) {
        const QStringList categoryList = commandLineLoggingOptions.split(",");

        if (categoryList[0] == QStringLiteral("full")) {
            filterRules += filterRuleFormat.arg("*");
        } else {
            for (const QString &category: categoryList) {
                filterRules += filterRuleFormat.arg(category);
            }
        }
    }

    filterRules += QStringLiteral("qt.qml.connections=false");

    qCDebug(QGCLoggingCategoryRegisterLog) << "Filter rules" << filterRules;
    QLoggingCategory::setFilterRules(filterRules);
}

void QGCLoggingCategoryManager::_splitFullCategoryName(const QString &fullCategoryName, QString &parentCategory, QString &childCategory)
{
    const int hierarchyIndex = fullCategoryName.indexOf(".");
    if (hierarchyIndex == -1) {
        parentCategory = QString();
        childCategory = fullCategoryName;
    } else {
        parentCategory = fullCategoryName.left(hierarchyIndex);
        childCategory = fullCategoryName.mid(hierarchyIndex + 1);
    }
}

void QGCLoggingCategoryManager::disableAllCategories()
{
    QSettings settings;
    settings.beginGroup(kFilterRulesSettingsGroup);
    settings.remove("");

    for (int i=0; i<_flatCategoryModel.count(); i++) {
        auto item = qobject_cast<QGCLoggingCategoryItem*>(_flatCategoryModel.get(i));
        item->setEnabled(false);
    }

    setFilterRulesFromSettings(QString());
}

QGCLoggingCategoryItem *QGCLoggingCategoryManager::_findLoggingCategory(const QString &fullCategoryName)
{
    return _categoryLookup.value(fullCategoryName, nullptr);
}

QGCLoggingCategoryItem::QGCLoggingCategoryItem(const QString& _shortCategory, const QString& _fullCategory, bool _enabled, QObject* parent)
    : QObject(parent)
    , shortCategory(_shortCategory)
    , fullCategory(_fullCategory)
    , _enabled(_enabled)
{
    connect(this, &QGCLoggingCategoryItem::enabledChanged, this, [this]() {
        QGCLoggingCategoryManager::instance()->setCategoryLoggingOn(this->fullCategory, this->_enabled);
    });
}

void QGCLoggingCategoryItem::setEnabled(bool enabled)
{
    if (enabled != _enabled) {
        _enabled = enabled;
        emit enabledChanged();
    }
}
