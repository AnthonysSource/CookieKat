#include "Tests/ECS_Test.h"
#include "Tests/ECST_Test.h"
#include "Tests/Entt_Test.h"

using namespace CKE;
using namespace CKE::ObjectModelTests;

int main()
{
	ECS_Test ecsTest{};
	ECST_Test ecstTest{};
	Entt_Test enttTest{};

	TestParameters testParams{};

	ecsTest.Setup(testParams);
	ecsTest.Run();

	ecstTest.Setup(testParams);
	ecstTest.Run();

	enttTest.Setup(testParams);
	enttTest.Run();

	return 0;
}
