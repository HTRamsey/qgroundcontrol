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
#include <QtGui/QAction>
#include <QtQml/QQmlComponent>

Q_DECLARE_LOGGING_CATEGORY(ToolStripActionLog)

class ToolStripAction : public QAction
{
    Q_OBJECT
    Q_PROPERTY(bool             showAlternateIcon   READ showAlternateIcon      WRITE setShowAlternateIcon      NOTIFY showAlternateIconChanged)
    Q_PROPERTY(bool             biColorIcon         READ biColorIcon            WRITE setbiColorIcon            NOTIFY biColorIconChanged)
    Q_PROPERTY(bool             fullColorIcon       READ fullColorIcon          WRITE setfullColorIcon          NOTIFY fullColorIconChanged)
    Q_PROPERTY(bool             nonExclusive        READ nonExclusive           WRITE setNonExclusive           NOTIFY nonExclusiveChanged)
    Q_PROPERTY(int              toolStripIndex      READ toolStripIndex         WRITE setToolStripIndex         NOTIFY toolStripIndexChanged)
    Q_PROPERTY(QString          iconSource          READ iconSource             WRITE setIconSource             NOTIFY iconSourceChanged)
    Q_PROPERTY(QString          alternateIconSource READ alternateIconSource    WRITE setAlternateIconSource    NOTIFY alternateIconSourceChanged)
    Q_PROPERTY(QQmlComponent    *dropPanelComponent READ dropPanelComponent     WRITE setDropPanelComponent     NOTIFY dropPanelComponentChanged)

public:
    explicit ToolStripAction(QObject *parent = nullptr);
    ~ToolStripAction();

    bool showAlternateIcon() const { return _showAlternateIcon; }
    bool biColorIcon() const { return _biColorIcon; }
    bool fullColorIcon() const { return _fullColorIcon; }
    bool nonExclusive() const { return _nonExclusive; }
    int toolStripIndex() const { return _toolStripIndex; }
    QString iconSource() const { return _iconSource; }
    QString alternateIconSource() const { return _alternateIconSource; }
    QQmlComponent *dropPanelComponent() const { return _dropPanelComponent; }

    void setShowAlternateIcon(bool showAlternateIcon);
    void setbiColorIcon(bool biColorIcon);
    void setfullColorIcon(bool fullColorIcon);
    void setNonExclusive(bool nonExclusive);
    void setToolStripIndex(int toolStripIndex);
    void setIconSource(const QString &iconSource);
    void setAlternateIconSource(const QString &alternateIconSource);
    void setDropPanelComponent(QQmlComponent *dropPanelComponent);

signals:
    void showAlternateIconChanged(bool showAlternateIcon);
    void biColorIconChanged(bool biColorIcon);
    void fullColorIconChanged(bool fullColorIcon);
    void nonExclusiveChanged(bool nonExclusive);
    void toolStripIndexChanged(int toolStripIndex);
    void iconSourceChanged(const QString &iconSource);
    void alternateIconSourceChanged(const QString &alternateIconSource);
    void dropPanelComponentChanged();

private:
    bool _showAlternateIcon = false;
    bool _biColorIcon = false;
    bool _fullColorIcon = false;
    bool _nonExclusive = false;
    int _toolStripIndex = -1;
    QString _iconSource;
    QString _alternateIconSource;
    QQmlComponent *_dropPanelComponent = nullptr;
};
