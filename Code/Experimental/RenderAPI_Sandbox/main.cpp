#include "FrameGraph_Tests.h"
#include "RenderSandboxEnv.h"
#include "TriangleTest.h"
#include "ParticleTest.h"

using namespace CKE;

int main() {
	FrameGraph_ComputeVertex triangleTest{};
	RenderSandboxEnv           renderEnv{};
	renderEnv.Run(&triangleTest);
	return 0;
}
