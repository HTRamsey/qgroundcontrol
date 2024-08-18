#include "CustomOptions.h"

CustomFlyViewOptions::CustomFlyViewOptions(CustomOptions *options, QObject *parent)
    : QGCFlyViewOptions(options, parent)
{

}

CustomOptions::CustomOptions(QObject *parent)
    : QGCOptions(parent)
{

}

QGCFlyViewOptions *CustomOptions::flyViewOptions()
{
    if (!_flyViewOptions) {
        _flyViewOptions = new CustomFlyViewOptions(this, this);
    }

    return _flyViewOptions;
}
