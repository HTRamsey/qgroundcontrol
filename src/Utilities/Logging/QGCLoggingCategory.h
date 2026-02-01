#pragma once

#include <QtCore/QHash>
#include <QtCore/QLoggingCategory>
#include <QtCore/QObject>
#include <QtCore/QString>

class QmlObjectListModel;
class QGCLoggingCategoryItem;

/// QGC-specific logging category macro. Categories are auto-registered with
/// QGCLoggingCategoryManager for runtime configuration via UI.
///
/// @param name     The logging category variable name
/// @param catStr   The category string (e.g., "Vehicle.Comms")
/// @param level    Default logging level (QtDebugMsg, QtInfoMsg, QtWarningMsg, etc.)
#define QGC_LOGGING_CATEGORY(name, catStr, ...) \
    Q_STATIC_LOGGING_CATEGORY(name, catStr __VA_OPT__(,) __VA_ARGS__)

class QGCLoggingCategoryManager : public QObject
{
    Q_OBJECT

public:
    static QGCLoggingCategoryManager* instance();

    /// Registers a logging category. Called automatically by QGC_LOGGING_CATEGORY macro.
    void registerCategory(const QString& category);

    /// Returns hierarchical tree model for QML display.
    QmlObjectListModel* treeCategoryModel() { return _treeCategoryModel.get(); }

    /// Returns flat list model for QML display.
    QmlObjectListModel* flatCategoryModel() { return _flatCategoryModel.get(); }

    /// Returns true if logging is enabled for the specified category.
    bool isCategoryEnabled(const QString& fullCategoryName) const;

    /// Enables/disables logging for a category. Persists to settings.
    void setCategoryEnabled(const QString& fullCategoryName, bool enabled);

    /// Installs the category filter. Call once at startup after command line parsing.
    /// @param commandLineLoggingOptions Comma-separated category names, or "full" for all
    void installFilter(const QString& commandLineLoggingOptions = QString());

    /// Disables all logging categories.
    void disableAllCategories();

    /// Finds a category item by full name. Returns nullptr if not found.
    QGCLoggingCategoryItem* findCategory(const QString& fullCategoryName) const;

signals:
    void categoryEnabledChanged(const QString& fullCategoryName, bool enabled);

private:
    QGCLoggingCategoryManager();

    void _loadEnabledCategories();
    void _saveEnabledCategory(const QString& fullCategoryName, bool enabled);
    void _refreshCategories();
    void _insertSorted(QmlObjectListModel* model, QGCLoggingCategoryItem* item);
    QGCLoggingCategoryItem* _findOrCreateParent(const QString& fullCategoryName);
    static QStringList _splitCategoryPath(const QString& fullCategoryName);
    static void _categoryFilter(QLoggingCategory* category);

    std::unique_ptr<QmlObjectListModel> _treeCategoryModel;
    std::unique_ptr<QmlObjectListModel> _flatCategoryModel;
    QHash<QString, QGCLoggingCategoryItem*> _categoryLookup;
    QSet<QString> _enabledCategories;
    QSet<QString> _commandLineCategories;
    bool _commandLineFullLogging = false;

    static QLoggingCategory::CategoryFilter s_previousFilter;
    static constexpr const char* kFilterRulesSettingsGroup = "LoggingFilters";
};

/*===========================================================================*/

/// Represents a single logging category for QML binding.
class QGCLoggingCategoryItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString shortName READ shortName CONSTANT)
    Q_PROPERTY(QString fullName READ fullName CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)
    Q_PROPERTY(QmlObjectListModel* children READ children CONSTANT)

public:
    explicit QGCLoggingCategoryItem(const QString& shortName,
                                    const QString& fullName,
                                    bool enabled,
                                    QObject* parent = nullptr);

    QString shortName() const { return _shortName; }
    QString fullName() const { return _fullName; }
    bool isEnabled() const { return _enabled; }
    bool isExpanded() const { return _expanded; }
    QmlObjectListModel* children() const { return _children.get(); }

    void setEnabled(bool enabled);
    void setExpanded(bool expanded);

    /// Creates child model on demand (for parent categories).
    void ensureChildModel();

    /// Returns true if this is a parent category (has children).
    bool isParent() const { return _children != nullptr; }

signals:
    void enabledChanged();
    void expandedChanged();

private:
    QString _shortName;
    QString _fullName;
    bool _enabled = false;
    bool _expanded = false;
    std::unique_ptr<QmlObjectListModel> _children;
};
