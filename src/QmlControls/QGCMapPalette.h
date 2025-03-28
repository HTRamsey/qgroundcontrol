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
#include <QtGui/QColor>
#include <QtQmlIntegration/QtQmlIntegration>

class QGCMapPalette : public QObject
{
    Q_OBJECT
    // QML_ELEMENT
    Q_PROPERTY(bool     lightColors READ lightColors    WRITE setLightColors    NOTIFY paletteChanged)
    Q_PROPERTY(QColor   text        READ text                                   NOTIFY paletteChanged)
    Q_PROPERTY(QColor   textOutline READ textOutline                            NOTIFY paletteChanged)

public:
    explicit QGCMapPalette(QObject *parent = nullptr);

    QColor text() const { return _text[_lightColors ? 0 : 1]; }
    QColor textOutline() const { return _textOutline[_lightColors ? 0 : 1]; }

    bool lightColors() const { return _lightColors; }
    void setLightColors(bool lightColors);

signals:
    void paletteChanged(void);
    void lightColorsChanged(bool lightColors);

private:
    bool _lightColors = false;

    static constexpr int _cColorGroups = 2;

    static QColor _text[_cColorGroups];
    static QColor _textOutline[_cColorGroups];
};
