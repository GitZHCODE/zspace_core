#include <zspace/display.h>

int main()
{
	zSpace::zObjectMesh mesh;
	zSpace::zObjectGraph graph;
	zSpace::zDisplayScene scene;

	zSpace::zDisplayMesh& meshDisplay = scene.mesh(mesh);
	meshDisplay.setElements(true, true, true);

	zSpace::zDisplayGraph& graphDisplay = scene.graph(graph);
	graphDisplay.setElements(true, true);

	return meshDisplay.verticesVisible() && graphDisplay.verticesVisible() ? 0 : 1;
}
