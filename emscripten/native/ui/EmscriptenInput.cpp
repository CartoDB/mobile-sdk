#include "EmscriptenInput.h"
#include "ui/MapView.h"
#include <unordered_map>

namespace carto {
    namespace EmscriptenInput {
        static const int NATIVE_ACTION_POINTER_1_DOWN = 0;
        static const int NATIVE_ACTION_POINTER_2_DOWN = 1;
        static const int NATIVE_ACTION_MOVE = 2;
        static const int NATIVE_ACTION_CANCEL = 3;
        static const int NATIVE_ACTION_POINTER_1_UP = 4;
        static const int NATIVE_ACTION_POINTER_2_UP = 5;
        static const int NATIVE_NO_COORDINATE = -1;

        struct Coord {
            long x;
            long y;
        };

        std::unordered_map<long, Coord> touchList;

        long _pointer1 = -1;
        long _pointer2 = -1;
        bool isPressed = false;

        long lastX = 0;
        long lastY = 0;

        long lastPressedX = 0;
        long lastPressedY = 0;


        EM_BOOL _emscripten_mouse_start(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
            MapView* mapView = (MapView*)userData;
            if (mouseEvent->buttons == 1) {
                mapView->onInputEvent(NATIVE_ACTION_POINTER_1_DOWN, mouseEvent->targetX * mapView->getScale(), mouseEvent->targetY * mapView->getScale(), NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
            }
            isPressed = true;
            return true;
        }

        EM_BOOL _emscripten_mouse_move(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
            MapView* mapView = (MapView*)userData;
            lastX = mouseEvent->targetX * mapView->getScale();
            lastY = mouseEvent->targetY * mapView->getScale();
            if (isPressed) {
                if (mouseEvent->buttons == 2 || (mouseEvent->buttons == 1 && (mouseEvent->ctrlKey || mouseEvent->shiftKey || mouseEvent->altKey || mouseEvent->metaKey))) {
                    if (lastPressedX != 0 && lastPressedY != 0) {
                        long diffX = lastX - lastPressedX;
                        long diffY = lastY - lastPressedY;

                        int x2 = mapView->getCanvasWidth() / 2;
                        int y2 = mapView->getCanvasHeight() / 2;

                        if (lastY > y2) diffX *= -1;

                        mapView->rotate(diffX / 1.5, 0);
                        mapView->tilt(diffY, 0);
                    } 
                    
                    lastPressedX = lastX;
                    lastPressedY = lastY;
                } else if (mouseEvent->buttons == 1) {
                    mapView->onInputEvent(NATIVE_ACTION_MOVE, mouseEvent->targetX * mapView->getScale(), mouseEvent->targetY * mapView->getScale(), NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
                }
            }
            
            return true;
        }

        EM_BOOL _emscripten_mouse_end(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
            MapView* mapView = (MapView*)userData;

            isPressed = false;
            if (mouseEvent->buttons == 0) {
                mapView->onInputEvent(NATIVE_ACTION_POINTER_1_UP, mouseEvent->targetX * mapView->getScale(), mouseEvent->targetY * mapView->getScale(), NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
            }

            lastPressedX = 0;
            lastPressedY = 0;

            return true;
        }


        EM_BOOL _emscripten_mouse_wheel(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData) {
            MapView* mapView = (MapView*)userData;
            if (wheelEvent->deltaY < -1.0) {
                mapView->onWheelEvent(1, lastX, lastY);
            } else if (wheelEvent->deltaY > 1.0) {
                mapView->onWheelEvent(-1, lastX, lastY);
            }

            return true;
        }


        bool isContainEmscriptenEvent(const EmscriptenTouchEvent *touchEvent, long identifier) {
            bool output = false;
            for (int i = 0; i < touchEvent->numTouches; ++i) {
                if (touchEvent->touches[i].identifier == identifier) {
                    output = true;
                    break;
                }
            }
            return output;
        }

        EM_BOOL _emscripten_touch_start(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData) {
            MapView* mapView = (MapView*)userData;
            // if (_pointer1 != -1) touchList.erase (_pointer1);
            // if (_pointer1 != -2) touchList.erase (_pointer2);
            // _pointer1 = -1;
            // _pointer2 = -1;
            for (int i = 0; i < touchEvent->numTouches; ++i) {
                long pointer = touchEvent->touches[i].identifier;
                if (pointer == _pointer1 || pointer == _pointer2) continue;
                Coord c;
                c.x = touchEvent->touches[i].targetX * mapView->getScale();
                c.y = touchEvent->touches[i].targetY * mapView->getScale();

                if (_pointer1 == -1) {
                    _pointer1 = pointer;
                    touchList[pointer] = c;

                    mapView->onInputEvent(NATIVE_ACTION_POINTER_1_DOWN, touchList[_pointer1].x, touchList[_pointer1].y, NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
                    continue;
                }
                
                if (_pointer2 == -1) {
                    _pointer2 = pointer;
                    touchList[pointer] = c;

                    mapView->onInputEvent(NATIVE_ACTION_POINTER_2_DOWN, touchList[_pointer1].x, touchList[_pointer1].y, touchList[_pointer2].x, touchList[_pointer2].y);
                    break;
                }
            }
            return true;
        }

        EM_BOOL _emscripten_touch_move(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData) {
            MapView* mapView = (MapView*)userData;
            for (int i = 0; i < touchEvent->numTouches; ++i) {
                auto &coord = touchList[touchEvent->touches[i].identifier];
                coord.x = touchEvent->touches[i].targetX * mapView->getScale();
                coord.y = touchEvent->touches[i].targetY * mapView->getScale();
            }
            if (_pointer1 != -1) {
                if (_pointer2 != -1) {
                    mapView->onInputEvent(NATIVE_ACTION_MOVE, touchList[_pointer1].x, touchList[_pointer1].y, touchList[_pointer2].x, touchList[_pointer2].y);
                } else {
                    mapView->onInputEvent(NATIVE_ACTION_MOVE, touchList[_pointer1].x, touchList[_pointer1].y, NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
                }
            }
            return true;
        }

        EM_BOOL _emscripten_touch_end(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData) {
            MapView* mapView = (MapView*)userData;
            if (_pointer2 != -1 && isContainEmscriptenEvent(touchEvent, _pointer2)) {
                // Dual pointer, second pointer goes up first
                mapView->onInputEvent(NATIVE_ACTION_POINTER_2_UP, touchList[_pointer1].x, touchList[_pointer1].y, touchList[_pointer2].x, touchList[_pointer2].y);
                touchList.erase (_pointer2);
                _pointer2 = -1;
            }
            
            if (_pointer1 != -1 && isContainEmscriptenEvent(touchEvent, _pointer1)) {
                // Single pointer, pointer goes up
                if (_pointer2 != -1) {
                    mapView->onInputEvent(NATIVE_ACTION_POINTER_1_UP, touchList[_pointer1].x, touchList[_pointer1].y, touchList[_pointer2].x, touchList[_pointer2].y);
                    touchList.erase (_pointer1);
                    _pointer1 = _pointer2;
                    _pointer2 = -1;
                } else {
                    mapView->onInputEvent(NATIVE_ACTION_POINTER_1_UP, touchList[_pointer1].x, touchList[_pointer1].y, NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
                    touchList.erase (_pointer1);
                    _pointer1 = -1;
                }
            }

            return true;
        }

        EM_BOOL _emscripten_touch_cancel(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData) {
            MapView* mapView = (MapView*)userData;
            mapView->onInputEvent(NATIVE_ACTION_CANCEL, NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
            touchList.erase (_pointer1);
            touchList.erase (_pointer2);
            _pointer1 = -1;
            _pointer2 = -1;

            return true;
        }
    }
}
