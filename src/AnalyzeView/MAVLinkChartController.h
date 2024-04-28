/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

/// @file
/// @brief MAVLink message inspector and charting controller
/// @author Gus Grubba <gus@auterion.com>

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QVariantList>
#include <QtCore/QLoggingCategory>
#include <QtCharts/QAbstractSeries>

Q_DECLARE_LOGGING_CATEGORY(MAVLinkInspectorLog)

class MAVLinkChartController;
class MAVLinkInspectorController;
class QGCMAVLinkMessage;

Q_DECLARE_METATYPE(QAbstractSeries*)

//-----------------------------------------------------------------------------
/// MAVLink message field
class QGCMAVLinkMessageField : public QObject {
    Q_OBJECT
public:
    Q_PROPERTY(QString          name        READ name       CONSTANT)
    Q_PROPERTY(QString          label       READ label      CONSTANT)
    Q_PROPERTY(QString          type        READ type       CONSTANT)
    Q_PROPERTY(QString          value       READ value      NOTIFY valueChanged)
    Q_PROPERTY(bool             selectable  READ selectable NOTIFY selectableChanged)
    Q_PROPERTY(int              chartIndex  READ chartIndex CONSTANT)
    Q_PROPERTY(QAbstractSeries* series      READ series     NOTIFY seriesChanged)

    QGCMAVLinkMessageField(QGCMAVLinkMessage* parent, QString name, QString type);

    QString         name            () { return _name;  }
    QString         label           ();
    QString         type            () { return _type;  }
    QString         value           () { return _value; }
    bool            selectable      () const{ return _selectable; }
    bool            selected        () { return _pSeries != nullptr; }
    QAbstractSeries*series          () { return _pSeries; }
    QList<QPointF>* values          () { return &_values;}
    qreal           rangeMin        () const{ return _rangeMin; }
    qreal           rangeMax        () const{ return _rangeMax; }
    int             chartIndex      ();

    void            setSelectable   (bool sel);
    void            updateValue     (QString newValue, qreal v);

    void            addSeries       (MAVLinkChartController* chart, QAbstractSeries* series);
    void            delSeries       ();
    void            updateSeries    ();

signals:
    void            seriesChanged       ();
    void            selectableChanged   ();
    void            valueChanged        ();

private:
    QString     _type;
    QString     _name;
    QString     _value;
    bool        _selectable = true;
    int         _dataIndex  = 0;
    qreal       _rangeMin   = 0;
    qreal       _rangeMax   = 0;

    QAbstractSeries*    _pSeries = nullptr;
    QGCMAVLinkMessage*  _msg     = nullptr;
    MAVLinkChartController*      _chart   = nullptr;
    QList<QPointF>      _values;
};

//-----------------------------------------------------------------------------
/// MAVLink message charting controller
class MAVLinkChartController : public QObject {
    Q_OBJECT
public:
    MAVLinkChartController(MAVLinkInspectorController* parent, int index);

    Q_PROPERTY(QVariantList chartFields         READ chartFields            NOTIFY chartFieldsChanged)
    Q_PROPERTY(QDateTime    rangeXMin           READ rangeXMin              NOTIFY rangeXMinChanged)
    Q_PROPERTY(QDateTime    rangeXMax           READ rangeXMax              NOTIFY rangeXMaxChanged)
    Q_PROPERTY(qreal        rangeYMin           READ rangeYMin              NOTIFY rangeYMinChanged)
    Q_PROPERTY(qreal        rangeYMax           READ rangeYMax              NOTIFY rangeYMaxChanged)
    Q_PROPERTY(int          chartIndex          READ chartIndex             CONSTANT)

    Q_PROPERTY(quint32      rangeYIndex         READ rangeYIndex            WRITE setRangeYIndex    NOTIFY rangeYIndexChanged)
    Q_PROPERTY(quint32      rangeXIndex         READ rangeXIndex            WRITE setRangeXIndex    NOTIFY rangeXIndexChanged)

    Q_INVOKABLE void        addSeries           (QGCMAVLinkMessageField* field, QAbstractSeries* series);
    Q_INVOKABLE void        delSeries           (QGCMAVLinkMessageField* field);

    Q_INVOKABLE MAVLinkInspectorController* controller() { return _controller; }

    QVariantList            chartFields         () { return _chartFields; }
    QDateTime               rangeXMin           () { return _rangeXMin;   }
    QDateTime               rangeXMax           () { return _rangeXMax;   }
    qreal                   rangeYMin           () const{ return _rangeYMin;   }
    qreal                   rangeYMax           () const{ return _rangeYMax;   }
    quint32                 rangeXIndex         () const{ return _rangeXIndex; }
    quint32                 rangeYIndex         () const{ return _rangeYIndex; }
    int                     chartIndex          () const{ return _index; }

    void                    setRangeXIndex      (quint32 t);
    void                    setRangeYIndex      (quint32 r);
    void                    updateXRange        ();
    void                    updateYRange        ();

signals:
    void chartFieldsChanged ();
    void rangeXMinChanged   ();
    void rangeXMaxChanged   ();
    void rangeYMinChanged   ();
    void rangeYMaxChanged   ();
    void rangeYIndexChanged ();
    void rangeXIndexChanged ();

private slots:
    void _refreshSeries     ();

private:
    QTimer              _updateSeriesTimer;
    QDateTime           _rangeXMin;
    QDateTime           _rangeXMax;
    int                 _index               = 0;
    qreal               _rangeYMin           = 0;
    qreal               _rangeYMax           = 1;
    quint32             _rangeXIndex         = 0;                    ///< 5 Seconds
    quint32             _rangeYIndex         = 0;                    ///< Auto Range
    QVariantList        _chartFields;
    MAVLinkInspectorController* _controller  = nullptr;
};
