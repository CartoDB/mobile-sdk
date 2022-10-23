#ifndef _CARTO_EMSCRIPTENMAPREDRAWREQUESTLISTENER_H_
#define _CARTO_EMSCRIPTENMAPREDRAWREQUESTLISTENER_H_

#include "renderers/RedrawRequestListener.h"

namespace carto {
	class MapView;

	class MapRedrawRequestListener : public RedrawRequestListener {
	public:
		MapRedrawRequestListener(MapView* mapView);
		void onRedrawRequested() const;

	private:
		MapView* _mapView;
	};
}

#endif
