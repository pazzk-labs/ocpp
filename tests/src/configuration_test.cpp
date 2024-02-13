#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "ocpp/core/configuration.h"
#include "ocpp/overrides.h"
#include <errno.h>

int ocpp_configuration_lock(void) {
	return 0;
}
int ocpp_configuration_unlock(void) {
	return 0;
}

TEST_GROUP(Configuration) {
	void setup(void) {
		ocpp_reset_configuration();
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(Configuration, reset_ShouldSetTheDefaultValues) {
	int connTimeout;
	int authRemoteTxReq;
	int blink;

	ocpp_get_configuration("ConnectionTimeOut", &connTimeout, sizeof(connTimeout), NULL);
	ocpp_get_configuration("AuthorizeRemoteTxRequests", &authRemoteTxReq, sizeof(authRemoteTxReq), NULL);
	ocpp_get_configuration("BlinkRepeat", &blink, sizeof(blink), NULL);

	LONGS_EQUAL(180, connTimeout);
	LONGS_EQUAL(true, authRemoteTxReq);
	LONGS_EQUAL(0, blink);
}

TEST(Configuration, has_configuration_ShouldReturnTrue_WhenValidOneGiven) {
	LONGS_EQUAL(true, ocpp_has_configuration("AuthorizeRemoteTxRequests"));
	LONGS_EQUAL(true, ocpp_has_configuration("WebSocketPingInterval"));
}

TEST(Configuration, has_configuration_ShouldReturnFalse_WhenInvalidOneGiven) {
	LONGS_EQUAL(false, ocpp_has_configuration("AuthorizeRemoteTxRequest"));
	LONGS_EQUAL(false, ocpp_has_configuration("UnkwonKey"));
}

TEST(Configuration, count_ShouldReturnTheNumberOfConfigurations) {
	LONGS_EQUAL(54, ocpp_count_configurations());
}

TEST(Configuration, get_len_ShouldReturnWholeConfigurationSize) {
	LONGS_EQUAL(270, ocpp_compute_configuration_size());
}

TEST(Configuration, set_ShouldSetTheConfiguration) {
	int expected = 3;
	int actual = 0;
	ocpp_set_configuration("ResetRetries", &expected, sizeof(expected));
	ocpp_get_configuration("ResetRetries", &actual, sizeof(actual), NULL);
	LONGS_EQUAL(3, actual);
	expected = OCPP_MEASURAND_SOC | OCPP_MEASURAND_TEMPERATURE;
	ocpp_set_configuration("MeterValuesSampledData", &expected, sizeof(expected));
	ocpp_get_configuration("MeterValuesSampledData", &actual, sizeof(actual), NULL);
	LONGS_EQUAL(0x180000, actual);

	char buf[16];
	ocpp_set_configuration("AuthorizationKey", "My Auth Key!", 12);
	ocpp_get_configuration("AuthorizationKey", buf, sizeof(buf), NULL);
	STRCMP_EQUAL("My Auth Key!", buf);
}

TEST(Configuration, is_writable_ShouldReturnItsAccessibility) {
	LONGS_EQUAL(false, ocpp_is_configuration_writable("NumberOfConnectors"));
	LONGS_EQUAL(true, ocpp_is_configuration_writable("StopTransactionOnInvalidId"));
	LONGS_EQUAL(false, ocpp_is_configuration_writable("UnknownKey"));
}

TEST(Configuration, get_ShouldReturnEINVAL_WhenUnknownKeyGiven) {
	int tmp;
	LONGS_EQUAL(-EINVAL, ocpp_get_configuration("AnyKey", &tmp, sizeof(tmp), NULL));
}

TEST(Configuration, set_ShouldReturnEINVAL_WhenUnknownKeyGiven) {
	int tmp;
	LONGS_EQUAL(-EINVAL, ocpp_set_configuration("AnyKey", &tmp, sizeof(tmp)));
}

TEST(Configuration, set_ShouldReturnEPERM_WhenTryWriteOnReadOnly) {
	int tmp;
	LONGS_EQUAL(-EPERM, ocpp_set_configuration("NumberOfConnectors", &tmp, sizeof(tmp)));
}
