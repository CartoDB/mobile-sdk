#ifndef _CARTO_EMSCRIPTENINPUT_H_
#define _CARTO_EMSCRIPTENINPUT_H_

#include <emscripten.h>
#include <emscripten/html5.h>

namespace carto {
    namespace EmscriptenInput {
        EM_BOOL _emscripten_mouse_doubleClick(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
        EM_BOOL _emscripten_mouse_start(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
        EM_BOOL _emscripten_mouse_move(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
        EM_BOOL _emscripten_mouse_end(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData);
        EM_BOOL _emscripten_mouse_wheel(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData);
        
        EM_BOOL _emscripten_touch_start(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
        EM_BOOL _emscripten_touch_move(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
        EM_BOOL _emscripten_touch_end(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
        EM_BOOL _emscripten_touch_cancel(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData);
    }
}

#endif
