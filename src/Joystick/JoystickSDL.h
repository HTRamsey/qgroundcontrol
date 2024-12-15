/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "Joystick.h"

struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;

struct _SDL_GameController;
typedef struct _SDL_GameController SDL_GameController;

class JoystickSDL : public Joystick
{
public:
    JoystickSDL(const QString &name, int axisCount, int buttonCount, int hatCount, int index, bool isGameController);
    ~JoystickSDL();

    int index() const { return _index; }
    void setIndex(int index) { _index = index; }

    // bool requiresCalibration() final { return !_isGameController; }

    static QMap<QString, Joystick*> discover();
    static bool init();

private:
    bool _open() final;
    void _close() final;
    bool _update() final;

    bool _getButton(int i) final;
    int  _getAxis(int i) final;
    bool _getHat(int hat, int i) final;

    static void _loadGameControllerMappings();

    bool _isGameController = false;
    int _index = -1;

    SDL_Joystick *_sdlJoystick = nullptr;
    SDL_GameController *_sdlController = nullptr;
};
