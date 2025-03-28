/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QObject>
#include <QtQml/QQmlComponent>

class ToolStripAction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool             enabled             READ enabled                WRITE setEnabled                NOTIFY enabledChanged)
    Q_PROPERTY(bool             visible             READ visible                WRITE setVisible                NOTIFY visibleChanged)
    Q_PROPERTY(bool             checkable           READ checkable              WRITE setCheckable              NOTIFY checkableChanged)
    Q_PROPERTY(bool             checked             READ checked                WRITE setChecked                NOTIFY checkedChanged)
    Q_PROPERTY(bool             showAlternateIcon   READ showAlternateIcon      WRITE setShowAlternateIcon      NOTIFY showAlternateIconChanged)
    Q_PROPERTY(bool             biColorIcon         READ biColorIcon            WRITE setbiColorIcon            NOTIFY biColorIconChanged)
    Q_PROPERTY(bool             fullColorIcon       READ fullColorIcon          WRITE setfullColorIcon          NOTIFY fullColorIconChanged)
    Q_PROPERTY(bool             nonExclusive        READ nonExclusive           WRITE setNonExclusive           NOTIFY nonExclusiveChanged)
    Q_PROPERTY(int              toolStripIndex      READ toolStripIndex         WRITE setToolStripIndex         NOTIFY toolStripIndexChanged)
    Q_PROPERTY(QString          text                READ text                   WRITE setText                   NOTIFY textChanged)
    Q_PROPERTY(QString          iconSource          READ iconSource             WRITE setIconSource             NOTIFY iconSourceChanged)
    Q_PROPERTY(QString          alternateIconSource READ alternateIconSource    WRITE setAlternateIconSource    NOTIFY alternateIconSourceChanged)
    Q_PROPERTY(QQmlComponent    *dropPanelComponent READ dropPanelComponent     WRITE setDropPanelComponent     NOTIFY dropPanelComponentChanged)

public:
    ToolStripAction(QObject *parent = nullptr);

    bool enabled() const { return _enabled; }
    bool visible() const { return _visible; }
    bool checkable() const { return _checkable; }
    bool checked() const { return _checked; }
    bool showAlternateIcon() const { return _showAlternateIcon; }
    bool biColorIcon() const { return _biColorIcon; }
    bool fullColorIcon() const { return _fullColorIcon; }
    bool nonExclusive() const { return _nonExclusive; }
    int toolStripIndex() const { return _toolStripIndex; }
    QString text() const { return _text; }
    QString iconSource() const { return _iconSource; }
    QString alternateIconSource() const { return _alternateIconSource; }
    QQmlComponent *dropPanelComponent() const { return _dropPanelComponent; }

    void setEnabled(bool enabled);
    void setVisible(bool visible);
    void setCheckable(bool checkable);
    void setChecked(bool checked);
    void setShowAlternateIcon(bool showAlternateIcon);
    void setbiColorIcon(bool biColorIcon);
    void setfullColorIcon(bool fullColorIcon);
    void setNonExclusive(bool nonExclusive);
    void setToolStripIndex(int  toolStripIndex);
    void setText(const QString &text);
    void setIconSource(const QString &iconSource);
    void setAlternateIconSource(const QString &alternateIconSource);
    void setDropPanelComponent(QQmlComponent *dropPanelComponent);

signals:
    void enabledChanged(bool enabled);
    void visibleChanged(bool visible);
    void checkableChanged(bool checkable);
    void checkedChanged(bool checked);
    void showAlternateIconChanged(bool showAlternateIcon);
    void biColorIconChanged(bool biColorIcon);
    void fullColorIconChanged(bool fullColorIcon);
    void nonExclusiveChanged(bool nonExclusive);
    void toolStripIndexChanged(int toolStripIndex);
    void textChanged(const QString &text);
    void iconSourceChanged(const QString &iconSource);
    void alternateIconSourceChanged(const QString &alternateIconSource);
    void triggered(QObject *source);
    void dropPanelComponentChanged();

protected:
    bool _enabled = true;
    bool _visible = true;
    bool _checkable = false;
    bool _checked = false;
    bool _showAlternateIcon = false;
    bool _biColorIcon = false;
    bool _fullColorIcon = false;
    bool _nonExclusive = false;
    int _toolStripIndex = -1;
    QString _text;
    QString _iconSource;
    QString _alternateIconSource;
    QQmlComponent *_dropPanelComponent = nullptr;
};
