#include "catch2/catch.hpp"

#include "eventhandler.hpp"

// Note: This is just a test to see the capabilities of Catch

TEST_CASE("EventHandler", "[utility]")
{
	SECTION("with no parameter")
	{
		EventHandler<void()> handler;
		int called = 0;
		handler.add([&called]()
		{
			++called;
		});

		SECTION("can handle one calls")
		{
			handler.call();
			REQUIRE(called == 1);
		}
		SECTION("can handle 10 calls")
		{
			for (int i = 0; i < 10; ++i)
				handler.call();
			REQUIRE(called == 10);
		}
		SECTION("can handle multiple events")
		{
			handler.add([&called]()
			{
				called += 2;
			});
			handler.call();
			REQUIRE(called == 3);
		}
	}

	SECTION("with one parameter")
	{
		EventHandler<void(int)> handler;

		int ret = 0;
		handler.add([&ret](int data)
		{
			ret = data;
		});

		SECTION("retrieves the parameter") {
			handler.call(1);
			REQUIRE(ret == 1);
		}
	}
}
